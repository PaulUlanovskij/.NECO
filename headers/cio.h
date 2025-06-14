#pragma once

#include "str.h"

enum LogLevel {
  LogLevel_Nolog,
  LogLevel_Info,
  LogLevel_Warning,
  LogLevel_Error
};

void log_write(enum LogLevel log_level, cstr file, int line, cstr msg);

#define log_info(msg) log_write(LogLevel_Info, __FILE__, __LINE__, msg)
#define log_warning(msg) log_write(LogLevel_Warning, __FILE__, __LINE__, msg)
#define log_error(msg) log_write(LogLevel_Error, __FILE__, __LINE__, msg)

#define panic(msg)                                                             \
  fprintf(stderr, "[PANIC]     %s:%d: %s\n", __FILE__, __LINE__, msg), abort();
#define todo(msg)                                                              \
  fprintf(stdout, "[TODO]      %s:%d: %s\n", __FILE__, __LINE__, msg), abort();

void clear_stdin();

cstr_o cread_line(const cstr prompt, bool ensure_nonempty);

int cread_int(const cstr prompt);
unsigned int cread_uint(const cstr prompt);
int cread_hex(const cstr prompt);
long cread_long(const cstr prompt);
long cread_longhex(const cstr prompt);
float cread_float(const cstr prompt);

unsigned int cread_option(const cstr prompt, const cstr const *options,
                          unsigned int options_length);
