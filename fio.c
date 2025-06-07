#include "headers/str.h"
#include <assert.h>
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
  SB sb = {};

  va_list vl;
  va_start(vl, nil);
  cstr arg = NULL;
  while ((arg = va_arg(vl, cstr)) != NULL) {
    if (sb.length == 0) {
      sb_append_cstr(&sb, arg);
    } else {
      bool ends_with_slash = cstr_ends_with(sb.items, "/");
      bool starts_with_slash = cstr_starts_with(arg, "/");
      if (starts_with_slash && ends_with_slash) {
        sb_append_cstr(&sb, arg + 1);
      } else if (ends_with_slash == false && starts_with_slash == false) {
        sb_append(&sb, '/');
        sb_append_cstr(&sb, arg);
      } else {
        sb_append_cstr(&sb, arg);
      }
    }
  }

  va_end(vl);
  cstr_o joined_path = cstr_from_sb(&sb);
  da_free(&sb);

  return joined_path;
}

SV_da path_split(const cstr path) {
  // There might be more logic needed, not sure for now
  return cstr_split_by_char(path, '/');
}

cstr path_greatest_common_path(const cstr path1, const cstr path2) {
  if (path1 == NULL || path2 == NULL) {
    return "";
  }
  SV_da splits1 = path_split(path1);
  SV_da splits2 = path_split(path2);
  SV_da common = {};
  for (int i = 0; i < splits1.length && i < splits2.length; i++) {
    SV sv1 = splits1.items[i];
    SV sv2 = splits2.items[i];
    if (sv1.length != sv2.length) {
      break;
    }
    if(strncmp(sv1.items, sv2.items, sv1.length)!=0){
      break;
    }
    da_append(&common, sv1);
  }
  da_free(&splits1);
  da_free(&splits2);
  SB sb = {};
  for (int i = 0; i < common.length; i++) {
    if (i == 0) {
      sb_append_sv(&sb, common.items[i]);
    } else {
      sb_append(&sb, '/');
      sb_append_sv(&sb, common.items[i]);
    }
  }
  cstr common_path = cstr_from_sb(&sb);
  da_free(&common);
  da_free(&sb);
  return common_path;
}
int path_depth(const cstr path) {
  SV_da splits = path_split(path);
  int depth = splits.length;
  da_free(&splits);
  return depth;
}

bool dir_open(cstr path, Dir *dir) {
  if (path == NULL) {
    path = ".";
  }
  DIR *handle = opendir(path);
  if (handle == NULL) {
    return false;
  }
  dir->path = path;
  struct dirent *dp = NULL;
  while ((dp = readdir(handle)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue;
    }
    DirEntry entry = {.name = dp->d_name,
                      .ino = dp->d_ino,
                      .reclen = dp->d_reclen,
                      .type = dp->d_type};
    da_append(dir, entry);
  }
  closedir(handle);
  return true;
}

bool dir_exists(const cstr path) {
  DIR *handle = opendir(path);
  if (handle == NULL) {
    return false;
  }
  closedir(handle);
  return true;
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

void fwrite_sv(SV sv, FILE *file) {
  fwrite(sv.items, sizeof(char), sv.length, file);
}

void fwrite_sb(SB sb, FILE *file) {
  fwrite(sb.items, sizeof(char), sb.length, file);
}

void fwrite_cstr(char *str, FILE *file) {
  fwrite(str, sizeof(char), strlen(str) + 1, file);
}

cstr_o fread_cstr(FILE *file) {
  SB line = {};
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
