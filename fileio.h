#pragma once
#include "helpers.h"
#include <stdio.h>
#include <string.h>

static inline void fwrite_string(char *str, FILE *file) {
  fwrite(str, sizeof(char), strlen(str) + 1, file);
}

#define define_fwrite(primitive)                                               \
  static inline void fwrite_##primitive(primitive *value_ref, FILE *file) {    \
    fwrite(value_ref, sizeof(primitive), 1, file);                             \
  }

define_fwrite(char);
define_fwrite(short);
define_fwrite(int);
define_fwrite(long);
define_fwrite(float);
define_fwrite(double);

#define define_fread(primitive)                                                \
  static inline pubool fread_##primitive(primitive *dest, FILE *file) {          \
    return fread(dest, sizeof(primitive), 1, file) == 1;       \
  }

define_fread(char);
define_fread(short);
define_fread(int);
define_fread(long);
define_fread(float);
define_fread(double);

static inline pubool fread_string(char **str, FILE *file) {
  string line = {};
  string *temp = &line;
  char c = 0;
  do {
    if (fread_char(&c, file) == false)
      return false;
    da_append(temp, c);
  } while (c != '\0');
  da_copy(*str, temp);
  da_free(temp);
  return true;
}
