#include "da.h"
#include "helpers.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SBV_DECLARATION

typedef struct {
  char *items;
  int length;
} StringView;

typedef struct {
  char *items;
  int length;
  int capacity;
} StringBuilder;

StringView sv_from_sb(StringBuilder sb);
StringBuilder sb_from_sv(StringView sv);

StringView sv_copy(StringView sv);

StringView sv_from(StringView sv, int index);
StringView sv_upto(StringView sv, int index);

void sb_clear(StringBuilder *sb);

void sb_append(StringBuilder *sb, char c);
void sb_append_buf(StringBuilder *sb, char *buf, int buf_length);
void sb_append_cstr(StringBuilder *sb, cstr str);
void sb_append_sv(StringBuilder *sb, StringView sv);
void sb_append_sb(StringBuilder *sb, StringBuilder sb2);

bool sv_from_file(StringView *sv, char *path);
bool sb_append_file(StringBuilder *sb, char *path);

int sv_get_char_index(StringView sv, char c);
int sv_get_first_char_index(StringView sv, cstr str);
int sv_get_word_index(StringView sv, cstr delim);
int sv_get_first_word_index(StringView sv, cstr *delim, int count);

// END SBV_DECLARATION

// FIO_DECLARATION

bool fexists(cstr path);
int flength(FILE *file);

void fwrite_sv(StringView sv, FILE *file);
void fwrite_sb(StringBuilder sb, FILE *file);
void fwrite_string(char *str, FILE *file);

pubool fread_string(cstr *str, FILE *file);

#define fwrite_val(value, file) fwrite(value, sizeof(*value), 1, file)
#define fread_val(value, file) fread(value, sizeof(*value), 1, file)

// END FIO_DECLARATION

// SBV_IMPLEMENTATION

StringView sv_from_sb(StringBuilder sb) {
  return (StringView){
      .items = sb.items,
      .length = sb.length,
  };
}
StringBuilder sb_from_sv(StringView sv) {
  return (StringBuilder){
      .items = sv.items,
      .length = sv.length,
      .capacity = sv.length,
  };
}

StringView sv_copy(StringView sv) {
  return (StringView){
      .items = (char *)malloc_copy(sv.length, sv.items),
      .length = sv.length,
  };
}

StringView sv_from(StringView sv, int index) {
  return (StringView){
      .items = sv.items + index,
      .length = sv.length - index,
  };
}
StringView sv_upto(StringView sv, int index) {
  return (StringView){
      .items = sv.items,
      .length = index,
  };
}

void sb_clear(StringBuilder *sb) {
  free(sb->items);
  sb->capacity = 0;
  sb->length = 0;
}
void sb_append(StringBuilder *sb, char c) {
  sb->items[sb->length - 1] = c;
  da_append(sb, '\0');
}

void sb_append_buf(StringBuilder *sb, char *buf, int buf_length) {
  da_reserve(sb, sb->length + buf_length);
  memcpy(sb->items + sb->length - 1, buf, buf_length);
  sb->items[sb->length - 1] = '\0';
}
void sb_append_cstr(StringBuilder *sb, cstr str) {
  sb_append_buf(sb, str, strlen(str));
}
void sb_append_sv(StringBuilder *sb, StringView sv) {
  sb_append_buf(sb, sv.items, sv.length);
}
void sb_append_sb(StringBuilder *sb, StringBuilder sb2) {
  sb_append_buf(sb, sb2.items, sb2.length);
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

int sv_get_char_index(StringView sv, char c) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = 0;
  while ((uintptr_t)(sv.items + i) % sizeof(size_t)) {
    if (i >= sv.length) {
      return -1;
    }
    if (sv.items[i] == c) {
      return i;
    }
    i++;
  }

  size_t pattern = ONES * (unsigned char)c;
  size_t *word_ptr = (size_t *)(sv.items + i);

  while (i + sizeof(size_t) < sv.length) {
    if (HASZERO(*word_ptr ^ pattern)) {
      break;
    }
    i += sizeof(size_t);
    word_ptr++;
  }

  while (i < sv.length) {
    if (sv.items[i] == c) {
      return i;
    }
    i++;
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int sv_get_first_char_index(StringView sv, cstr str) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = 0;
  while ((uintptr_t)(sv.items + i) % sizeof(size_t)) {
    if (i >= sv.length) {
      return -1;
    }
    if (strchr(str, sv.items[i]) != NULL) {
      return i;
    }
    i++;
  }

  int pattern_count = strlen(str);
  size_t *patterns = (size_t *)malloc(pattern_count * sizeof(size_t));
  for (int j = 0; j < pattern_count; j++) {
    patterns[j] = ONES * (unsigned char)str[j];
  }
  size_t *word_ptr = (size_t *)(sv.items + i);

  while (i + sizeof(size_t) < sv.length) {
    for (int j = 0; j < pattern_count; j++) {
      if (HASZERO(*word_ptr ^ patterns[j])) {
        goto seek_single;
      }
    }
    i += sizeof(size_t);
    word_ptr++;
  }
seek_single:
  while (i < sv.length) {
    if (strchr(str, sv.items[i]) != NULL) {
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
  temp.length -= (delim_length - 1);
  while (i < sv.length - delim_length) {
    int j = sv_get_char_index(temp, delim[0]);
    if (j < 0) {
      return -1;
    }
    if (strncmp(temp.items + j, delim, delim_length) == 0) {
      return i + j;
    }
    j++;
    i += j;
    temp.length -= j;
    temp.items += j;
  }
  return -1;
}
int sv_get_first_word_index(StringView sv, cstr *delim, int count) {
  int *delim_lengths = (int *)malloc(sizeof(int) * count);
  char *heads = (char *)malloc(count);
  for (int i = 0; i < count; i++) {
    heads[i] = delim[i][0];
    delim_lengths[i] = strlen(delim[i]);
  }
  int result = 0;
  StringView temp = sv;
  bool matched = true;
  while (matched) {
    matched = false;
    int j = sv_get_first_char_index(temp, heads);
    if (j < 0) {
      defer_res(-1);
    }
    for (int k = 0; k < count; k++) {
      if (j > sv.length - (delim_lengths[k] - 1)) {
        continue;
      }
      if (result < sv.length - delim_lengths[k]) {
        continue;
      }
      matched = true;
      if (strncmp(temp.items + j, delim[k], delim_lengths[k]) == 0) {
        defer_res(result + j);
      }
    }
    j++;
    result += j;
    temp.length -= j;
    temp.items += j;
  }
defer:
  free(heads);
  free(delim_lengths);
  return result;
}

// END SBV_IMPLEMENTATION

// FIO_IMPLEMENTATION

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

void fwrite_sv(StringView sv, FILE *file) {
  fwrite(sv.items, sizeof(char), sv.length, file);
}

void fwrite_sb(StringBuilder sb, FILE *file) {
  fwrite(sb.items, sizeof(char), sb.length, file);
}

void fwrite_string(char *str, FILE *file) {
  fwrite(str, sizeof(char), strlen(str) + 1, file);
}

pubool fread_string(cstr *str, FILE *file) {
  StringBuilder line = {};
  char c = 0;
  do {
    if (fread_val(&c, file) == false)
      return false;
    da_append(&line, c);
  } while (c != '\0');
  da_copy_items(*str, &line);
  da_free(&line);
  return true;
}

// END FIO_IMPLEMENTATION
