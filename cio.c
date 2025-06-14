#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/da.h"
#include "headers/str.h"
#include "headers/cio.h"


void log_write(enum LogLevel log_level, cstr file, int line, cstr msg) {
  switch (log_level) {
  case LogLevel_Info:
    fprintf(stdout, "[INFO]      %s:%d: %s\n", file, line, msg);
    return;
  case LogLevel_Warning:
    fprintf(stderr, "[WARNING]   %s:%d: %s\n", file, line, msg);
    return;
  case LogLevel_Error:
    fprintf(stderr, "[ERROR]     %s:%d: %s\n", file, line, msg);
    return;
  case LogLevel_Nolog:
    return;
  }
  panic("Unknown log level.");
}

void clear_stdin() {
  int c = 0;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

cstr_o cread_line(const cstr prompt, bool ensure_nonempty) {
  if (prompt != NULL) {
    printf("%s : ", prompt);
  }

  int c = 0;
  dstr line = {};
  cstr_o res = NULL;

START_OVER:
  while ((c = getchar()) != '\n' && c != EOF) {
    dstr_append(&line, c);
  }

  if (c == EOF || line.length == 0) {
    if (ensure_nonempty) {
      puts("Input Error : String must be non-empty.");
      goto START_OVER;
    }
    return (char *)"";
  }

  if (ensure_nonempty) {
    da_foreach(&line, i) {
      if (i != NULL && isspace(*i) == false) {
        goto COPY_AND_RETURN;
      }
    }
    puts("Input Error : String must be non-empty.");
    goto START_OVER;
  } else {
    goto COPY_AND_RETURN;
  }

COPY_AND_RETURN:
  res = dstr_to_cstr(line);
  da_free(&line);

  return res;
}

#define define_cread(name, type, fmt, err_m)                                   \
  type cread_##name(const cstr prompt) {                                       \
    if (prompt != NULL) {                                                      \
      printf("%s : ", prompt);                                                 \
    }                                                                          \
    type input = 0;                                                            \
    while (true) {                                                             \
      if (scanf(fmt, &input) == 1 && getchar() == '\n') {                      \
        break;                                                                 \
      } else {                                                                 \
        puts("Input Error : " err_m);                                          \
        clear_stdin();                                                         \
      }                                                                        \
    }                                                                          \
                                                                               \
    return input;                                                              \
  }

define_cread(int, int, "%d", "Input must be an integer.");
define_cread(uint, unsigned int, "%u", "Input must be a non-negative integer.");
define_cread(long, long, "%ld", "Input must be an integer.");
define_cread(longhex, long, "%lx", "Input must be a hexadecimal number.");
define_cread(hex, int, "%x", "Input must be a hexadecimal number.");
define_cread(float, float, "%f", "Input must be a rational number.");

unsigned int cread_option(const cstr prompt, const cstr const *options,
                          unsigned int options_length) {
  assert(options != NULL);
  for (int i = 0; i < options_length; i++) {
    assert(options[i] != NULL);
  }

  if (prompt == NULL) {
    puts("----Choose an option----");
  } else {
    printf("%s\n", prompt);
  }

  for (int i = 0; i < options_length; i++) {
    printf("%d. %s\n", i + 1, options[i]);
  }

  unsigned int input = cread_uint(NULL);
  while (input < 1 || input > options_length) {
    printf("Invalid option, enter an integer in range from 1 to %u : ",
           options_length);
    input = cread_uint(NULL);
  }

  return input;
}
