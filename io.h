#include "da.h"
#include "helpers.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
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

StringView sv_from_sb(StringBuilder sb);
StringView sv_from_sb_owned(StringBuilder sb);

StringView sv_from_sv_until_delim(StringView sv, char delim);
StringView sv_from_sb_until_delim(StringBuilder sb, char delim);
StringView sv_from_sb_until_delim_owned(StringBuilder sb, char delim);

StringView sv_from_sv_until_word(StringView sv, cstr delim);
StringView sv_from_sb_until_word(StringBuilder sb, cstr delim);
StringView sv_from_sb_until_word_owned(StringBuilder sb, cstr delim);

int sv_get_char_index(StringView sv, char c);
int sv_get_word_index(StringView sv, cstr delim);

bool fexists(cstr path);
int flength(FILE *file);

void fwrite_string(char *str, FILE *file);
pubool fread_string(cstr *str, FILE *file);

#define define_fwrite(primitive)                                               \
  void fwrite_##primitive(primitive *value_ref, FILE *file) {                  \
    fwrite(value_ref, sizeof(primitive), 1, file);                             \
  }

define_fwrite(char);
define_fwrite(short);
define_fwrite(int);
define_fwrite(long);
define_fwrite(float);
define_fwrite(double);

#define define_fread(primitive)                                                \
  pubool fread_##primitive(primitive *dest, FILE *file) {                      \
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

StringView sv_from_sb(StringBuilder sb) {
  return (StringView){.items = sb.items, .length = sb.length};
}
StringView sv_from_sb_owned(StringBuilder sb) {
  cstr copy = (cstr)malloc(sb.length);
  memcpy(copy, sb.items, sb.length);
  return (StringView){.items = copy, .length = sb.length};
}

StringView sv_from_sv_until_delim(StringView sv, char delim) {
  return (StringView){.items = sv.items,
                      .length = sv_get_char_index(sv, delim)};
}
StringView sv_from_sb_until_delim(StringBuilder sb, char delim) {
  return sv_from_sv_until_delim(sv_from_sb(sb), delim);
}

StringView sv_from_sb_until_delim_owned(StringBuilder sb, char delim) {
  return sv_from_sv_until_delim(sv_from_sb_owned(sb), delim);
}

StringView sv_from_sv_until_word(StringView sv, cstr delim) {
  return (StringView){.items = sv.items,
                      .length = sv_get_word_index(sv, delim)};
}

StringView sv_from_sb_until_word(StringBuilder sb, cstr delim) {
  return sv_from_sv_until_word(sv_from_sb(sb), delim);
}

StringView sv_from_sb_until_word_owned(StringBuilder sb, cstr delim) {
  return sv_from_sv_until_word(sv_from_sb_owned(sb), delim);
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

int sv_get_char_index(StringView sv, char c) {
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) ((x)-ONES & ~(x) & HIGHS)
  int i = 0;
  while ((uintptr_t)(sv.items+i) % sizeof(size_t)){
    if (i >= sv.length){
      return -1;
    }
    if (sv.items[i] == c){
      return i;
    }
    i++;
  }

  size_t pattern = ONES * (unsigned char)c;
  size_t *word_ptr = (size_t *)(sv.items + i);

  while (i + sizeof(size_t) < sv.length) {
    if(HASZERO(*word_ptr ^ pattern)){
      break;
    }
    i += sizeof(size_t);
    word_ptr++;
  }

  while (i < sv.length) {
    if (sv.items[i] == c){
      return i;
    }
    i++;
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int sv_get_word_index(StringView sv, cstr delim) {
  int delim_length = strlen(delim);
  int i = 0;
  StringView temp = sv;
  temp.length-=(delim_length-1);
  while(i < sv.length-delim_length){
    int j = sv_get_char_index(temp, delim[0]);
    if(j < 0)
      return -1;
    if(strncmp(temp.items + j, delim, delim_length) == 0){
      return i+j;
    }
    j++;
    i+=j;
    temp.length -=j;
    temp.items +=j;
  }
  return -1;
}
