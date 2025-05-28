#pragma once
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define pubool char
#define pucstr char *

#ifndef cstr
#define cstr pucstr
#endif

#if !__bool_true_false_are_defined
#define true 1
#define false 0
#define bool pubool
#endif

#define defer_res(v)                                                           \
  do {                                                                         \
    result = v;                                                                \
    goto defer;                                                                \
  } while (0)
#define errdefer_res(v)                                                        \
  do {                                                                         \
    result = v;                                                                \
    goto errdefer;                                                             \
  } while (0)

enum LogLevel { LogLevel_Info, LogLevel_Error };
void log_write(enum LogLevel log_level, cstr file, int line, cstr message) {
  switch (log_level) {
  case LogLevel_Info:
    fprintf(stdout, "[INFO] %s:%d: %s\n", file, line, message);
    break;
  case LogLevel_Error:
    fprintf(stderr, "[ERROR] %s:%d: %s\n", file, line, message);
    break;
  }
}
#define log_error(msg)                                              \
  log_write(LogLevel_Error, __FILE__, __LINE__, msg)
#define log_info(msg)                                               \
  log_write(LogLevel_Info, __FILE__, __LINE__, msg)

#define unreachable(message)                                                   \
  do {                                                                         \
    log_error(message);                                                        \
    abort();                                                                   \
  } while (0)
#define todo(message)                                                          \
  do {                                                                         \
    log_info(message);                                                         \
    abort();                                                                   \
  } while (0)

#define len(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define ll_append(xs, x, nullness_check)                                       \
  do {                                                                         \
    if (nullness_check) {                                                      \
      (xs)->this = (x);                                                        \
    } else {                                                                   \
      while ((xs)->next != NULL) {                                             \
        (xs) = (xs)->next;                                                     \
      }                                                                        \
      (xs)->next = malloc(sizeof(*xs));                                        \
      (xs)->next->next = NULL;                                                 \
      (xs)->next->this = (x);                                                  \
    }                                                                          \
  } while (0)

#define ll_dealloc(xs, temp, prev)                                             \
  do {                                                                         \
    typeof((xs)) temp = xs;                                                    \
    typeof((xs)) prev = NULL;                                                  \
    while ((xs)->next != NULL) {                                               \
      for (temp = (xs); temp->next != NULL; temp = temp->next) {               \
        prev = temp;                                                           \
      }                                                                        \
      prev->next = NULL;                                                       \
      free(temp);                                                              \
    }                                                                          \
  } while (0)

#define ll_foreach(xs, name)                                                   \
  for (typeof(xs) name = (xs); name != NULL; name = name->next)
