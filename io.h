#include "da.h"
#include "helpers.h"
#include <stdlib.h>

typedef struct {
  char *items;
  int length;
} StringView;

typedef struct {
  char *items;
  int length;
  int capacity;
} StringBuilder;

StringView sb_to_sv(StringBuilder sb);
StringBuilder sb_from_sv(StringView sv);

void sb_append(StringBuilder *sb, char c);

void sb_append_cstr(StringBuilder *sb, cstr str);
void sb_append_sv(StringBuilder *sb, StringView sv);
void sb_append_sb(StringBuilder *sb, StringBuilder sb2);

bool sb_append_file(StringBuilder *sb, char *path);
bool sv_from_file(StringView *sv, char *path);

void sb_clear(StringBuilder *sb);

bool fexists(cstr path);
int flength(FILE *file);

void fwrite_string(char *str, FILE *file);
pubool fread_string(cstr *str, FILE *file);

#define define_fwrite(primitive)                                               \
  void fwrite_##primitive(primitive *value_ref, FILE *file) {    \
    fwrite(value_ref, sizeof(primitive), 1, file);                             \
  }

define_fwrite(char);
define_fwrite(short);
define_fwrite(int);
define_fwrite(long);
define_fwrite(float);
define_fwrite(double);

#define define_fread(primitive)                                                \
  pubool fread_##primitive(primitive *dest, FILE *file) {        \
    return fread(dest, sizeof(primitive), 1, file) == 1;                       \
  }

define_fread(char);
define_fread(short);
define_fread(int);
define_fread(long);
define_fread(float);
define_fread(double);

StringView sb_to_sv(StringBuilder sb) {
  StringView sv = {};
  sv.items = (char *)malloc(sb.length);
  memcpy(sv.items, sb.items, sb.length);
  sv.length = sb.length;
  return sv;
}
void sb_clear(StringBuilder *sb) {
  free(sb->items);
  sb->capacity = 0;
  sb->length = 0;
}
void sb_append(StringBuilder *sb, char c) {
  if (sb->length == sb->capacity) {
    if (sb->capacity == 0) {
      sb->capacity = 256;
      sb->length = 1;
    } else {
      sb->capacity *= 2;
    }
    sb->items = (char *)realloc(sb->items, sb->capacity);
    sb->items[sb->length - 1] = c;
    sb->items[sb->length] = '\0';
    sb->length++;
  }
}

StringBuilder sb_from_sv(StringView sv) {
  return (StringBuilder){sv.items, sv.length, sv.length};
}

void sb_append_cstr(StringBuilder *sb, cstr str) {
  for (char *c = str; *c != '\0'; c++) {
    sb_append(sb, *c);
  }
}
void sb_append_sv(StringBuilder *sb, StringView sv) {
  for (int i = 0; i < sv.length; i++) {
    sb_append(sb, sv.items[i]);
  }
}
void sb_append_sb(StringBuilder *sb, StringBuilder sb2) {
  for (int i = 0; i < sb2.length; i++) {
    sb_append(sb, sb2.items[i]);
  }
}
bool sv_from_file(StringView *sv, char *path) {
  bool result = true;
  FILE *file = fopen(path, "rb");
  int m = flength(file);
  if (m < 0) {
    defer_res(false);
  }
  sv->items = (char *)malloc(m);
  fread(sv->items, m, 1, file);
  if (ferror(file)) {
    defer_res(false);
  }
  sv->length = m;
defer:
  fclose(file);
  return result;
}

bool sb_append_file(StringBuilder *sb, char *path) {
  bool result = true;
  FILE *file = fopen(path, "rb");
  int m = flength(file);
  if (m < 0) {
    defer_res(false);
  }
  size_t new_length = sb->length + m;
  da_reserve(sb, new_length);
  fread(sb->items + sb->length, m, 1, file);
  if (ferror(file)) {
    defer_res(false);
  }
  sb->length = new_length;
defer:
  fclose(file);
  return result;
}

bool fexists(cstr path) {
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

void fwrite_string(char *str, FILE *file) {
  fwrite(str, sizeof(char), strlen(str) + 1, file);
}

pubool fread_string(cstr *str, FILE *file) {
  StringBuilder line = {};
  StringBuilder *temp = &line;
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
