#pragma once

#include "str.h"
#ifndef _WIN32
#include <sys/types.h>

#else
// TODO: Add windows support
#endif

typedef int fd;

typedef cstr_da Cmd;

typedef struct {
  int returncode;
  fd stdout;
  fd stderr;
} FinishedProcess;

typedef struct {
  pid_t pid;
  fd fd_in;
  fd fd_out;
  fd fd_err;
} ProcessPromise;

void free_process_promise(ProcessPromise promise);

bool process_finished(ProcessPromise proc_promise);
FinishedProcess process_getresult(ProcessPromise proc_promise);

FinishedProcess subprocess_run_x(Cmd *cmd, fd fd_in, fd fd_out, fd fd_err);
FinishedProcess subprocess_run(Cmd *cmd, cstr input, bool capture_output);

ProcessPromise subprocess_run_async_x(Cmd *cmd, fd fd_in, fd fd_out, fd fd_err);
ProcessPromise subprocess_run_async(Cmd *cmd, cstr input, bool capture_output);

void cmd_free(Cmd *cmd);
void cmd_render(Cmd *cmd);

void cmd_append_many(Cmd *cmd, ...);
#define cmd_append(cmd, ...) cmd_append_many(cmd, __VA_ARGS__, NULL)
