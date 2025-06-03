#include "headers/str.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <dirent.h>
#else
// TODO: add windows support
#endif

#include "headers/pulib.h"

#define path_join(...) path_join_many(NULL, __VA_ARGS__, NULL)
cstr_o path_join_many(void *nil, ...) {
  cstr_o joined_path = NULL;
  StringBuilder sb = {};

  va_list vl;
  va_start(vl, nil);
  cstr arg = NULL;
  while ((arg = va_arg(vl, cstr)) != NULL) {
    if (sb.length == 0) {
      sb_append_cstr(&sb, arg);
    } else {
      if (cstr_starts_with(arg, "/") && cstr_ends_with(sb.items, "/")){
        sb_append_cstr(&sb, arg + 1);
      }else{
        sb_append_cstr(&sb, arg);
      }
    }
  }

  va_end(vl);

  return joined_path;
}

DirEntry_da dir_get_all_items(DIR *dir) {
  DirEntry_da entries = {};
  struct dirent *dp = NULL;
  while ((dp = readdir(dir)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue;
    }
    DirEntry entry = {.name = dp->d_name,
                      .ino = dp->d_ino,
                      .reclen = dp->d_reclen,
                      .type = dp->d_type};
    da_append(&entries, entry);
  }
  return entries;
}

bool fexists(const cstr path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    return false;
  }
  fclose(file);
  return true;
}

// For files opened in binary format only
int flength(FILE *file) {
  if (file == NULL)
    return -1;
  int pos = ftell(file);
  if (pos < 0 || fseek(file, 0, SEEK_END) < 0)
    return -1;
  long length = ftell(file);
  if (length < 0 || fseek(file, pos, SEEK_SET) < 0)
    return -1;
  return length;
}

void fwrite_sv(StringView sv, FILE *file) {
  fwrite(sv.items, sizeof(char), sv.length, file);
}

void fwrite_sb(StringBuilder sb, FILE *file) {
  fwrite(sb.items, sizeof(char), sb.length, file);
}

void fwrite_cstr(char *str, FILE *file) {
  fwrite(str, sizeof(char), strlen(str) + 1, file);
}

cstr_o fread_cstr(FILE *file) {
  StringBuilder line = {};
  char c = 0;
  do {
    if (fread_val(&c, file) == false)
      return NULL;
    da_append(&line, c);
  } while (c != '\0');
  cstr result = cstr_from_sb(&line);
  da_free(&line);
  return result;
}
