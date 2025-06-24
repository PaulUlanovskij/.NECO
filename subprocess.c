#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/cio.h"
#include "headers/da.h"
#include "headers/str.h"
#include "headers/subprocess.h"

#ifndef _WIN32
#include <sys/wait.h>
#include <unistd.h>
#else
// TODO: add windows support
#endif

void free_process_promise(ProcessPromise promise) {
  close(promise.fd_in);
  close(promise.fd_out);
  close(promise.fd_err);
}
bool process_finished(ProcessPromise proc_promise) {
  if (proc_promise.pid <= 0) {
    return false;
  }
  int status = 0;
  return waitpid(proc_promise.pid, &status, WNOHANG) > 0;
}
FinishedProcess process_getresult(ProcessPromise proc_promise) {
  if (proc_promise.pid <= 0) {
    return (FinishedProcess){.returncode = -1};
  }
  int status;
  waitpid(proc_promise.pid, &status, 0);

  if (WIFEXITED(status)) {
    close(proc_promise.fd_in);
    return (FinishedProcess){.returncode = WEXITSTATUS(status),
                             .stdout = proc_promise.fd_out,
                             .stderr = proc_promise.fd_err};
  }
  return (FinishedProcess){.returncode = -1};
}

FinishedProcess subprocess_run_x(Cmd *cmd, fd fd_in, fd fd_out, fd fd_err) {
  return process_getresult(subprocess_run_async_x(cmd, fd_in, fd_out, fd_err));
}
FinishedProcess subprocess_run(Cmd *cmd, cstr input, bool capture_output) {
  return process_getresult(subprocess_run_async(cmd, input, capture_output));
}

ProcessPromise subprocess_run_async_x(Cmd *cmd, fd fd_in, fd fd_out,
                                      fd fd_err) {
  if (cmd->length < 1) {
    return (ProcessPromise){};
  }

  pid_t cpid = fork();
  if (cpid < 0) {
    return (ProcessPromise){};
  }

  da_append(cmd, NULL);

  if (cpid == 0) {
    if (fd_in > 0) {
      if (dup2(fd_in, STDIN_FILENO) < 0) {
        exit(1);
      }
    }
    if (fd_out > 1) {
      if (dup2(fd_out, STDOUT_FILENO) < 0) {
        exit(1);
      }
    }
    if (fd_err > 2) {
      if (dup2(fd_err, STDERR_FILENO) < 0) {
        exit(1);
      }
    }

    if (execvp(cmd->items[0], cmd->items) < 0) {
      exit(1);
    }
    panic("execvp returned non-negative value. Something went horribly wrong.");
  }

  cmd->length = 0;
  cmd->capacity = 0;

  return (ProcessPromise){
      .pid = cpid, .fd_in = fd_in, .fd_out = fd_out, .fd_err = fd_err};
}
ProcessPromise subprocess_run_async(Cmd *cmd, cstr input, bool capture_output) {
  char template[] = "/tmp/fileXXXXXX";
  int fd_in = -1;
  int fd_out = -1;
  int fd_err = -1;

  if (input != NULL) {
    fd_in = mkstemp(template);
    if (fd_in < 0) {
      return (ProcessPromise){};
    }
    write(fd_in, input, strlen(input));
    lseek(fd_in, 0, SEEK_SET);
  }
  if (capture_output) {
    fd_out = mkstemp(template);
    if (fd_out < 0) {
      return (ProcessPromise){};
    }
    fd_err = mkstemp(template);
    if (fd_err < 0) {
      return (ProcessPromise){};
    }
  }
  return subprocess_run_async_x(cmd, fd_in, fd_out, fd_err);
}

void cmd_append_many(Cmd *cmd, ...) {
  va_list vl;
  va_start(vl, cmd);
  cstr arg = NULL;
  while ((arg = va_arg(vl, cstr)) != NULL) {
    cstr_o quoted = cstr_quote(arg);
    da_append(cmd, quoted);
  }
  va_end(vl);
}
void cmd_free(Cmd *cmd) {
  da_foreach(cmd, i) { free(i); }
  da_free(cmd);
}
void cmd_render(Cmd *cmd) {
  dstr sb = {};
  for (int i = 0; i < cmd->length; i++) {
    dstr_append_cstr(&sb, cmd->items[i]);
    dstr_append(&sb, ' ');
  }
  printf("%s\n", sb.items);
  da_free(&sb);
}
