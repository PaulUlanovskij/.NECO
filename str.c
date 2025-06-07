#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/pulib.h"

bool cmp_cstr_sv(cstr str, SV sv) {
  if (sv.length != strlen(str)) {
    return false;
  }
  return strncmp(str, sv.items, sv.length) == 0;
}

#define _sv_index_by(sv, needle, index_func)                                   \
  int_da indices = {};                                                         \
  SV temp = sv;                                                                \
  int index = index_func(sv, needle);                                          \
  while (index != -1) {                                                        \
    da_append(&indices, temp.items + index - sv.items);                        \
    temp = sv_from(temp, index + 1);                                           \
    index = index_func(temp, needle);                                          \
  }                                                                            \
  return indices;

int_da sv_index_by_char(SV sv, char c) {
  _sv_index_by(sv, c, sv_get_char_index);
}

int_da sv_index_by_chars(SV sv, char *c) {
  _sv_index_by(sv, c, sv_get_first_char_index);
}

#define cstr_index_by(str, needle, index_func)                                 \
  int_da indices = {};                                                         \
  cstr temp = str;                                                             \
  char *index = index_func(str, needle);                                       \
  while (index != NULL) {                                                      \
    da_append(&indices, index - str);                                          \
    temp = index + 1;                                                          \
    index = index_func(str, needle);                                           \
  }                                                                            \
  return indices;

int_da cstr_index_by_char(const cstr str, char c) {
  cstr_index_by(str, c, strchr);
}
int_da cstr_index_by_chars(const cstr str, char *chars) {
  cstr_index_by(str, chars, strpbrk);
}

#define sv_split_by(sv, needle, index_func)                                    \
  SV_da splits = {};                                                           \
  int_da indices = index_func(sv, needle);                                     \
  for (int i = 0; i < indices.length; i++) {                                   \
    SV split = {};                                                             \
    if (i == 0) {                                                              \
      split = (SV){.items = sv.items, .length = indices.items[i]};             \
      da_append(&splits, split);                                               \
    }                                                                          \
    if (i == indices.length - 1) {                                             \
      split = (SV){.items = sv.items + indices.items[i] + i + 1,               \
                   .length = sv.length - indices.items[i] - 1};                \
    } else {                                                                   \
      split = (SV){.items = sv.items + indices.items[i] + i + 1,               \
                   .length = indices.items[i + 1] - indices.items[i] - 1};     \
    }                                                                          \
    da_append(&splits, split);                                                 \
  }                                                                            \
  return splits;

SV_da sv_split_by_char(SV sv, char delim) {
  sv_split_by(sv, delim, sv_index_by_char);
}
SV_da sv_split_by_chars(SV sv, char *delim) {
  sv_split_by(sv, delim, sv_index_by_chars);
}

#define cstr_split_by(str, needle, index_func)                                 \
  SV_da splits = {};                                                           \
  int_da indices = index_func(str, needle);                                    \
  int str_length = strlen(str);                                                \
  for (int i = 0; i < indices.length; i++) {                                   \
    SV split = {};                                                             \
    if (i == 0) {                                                              \
      split = (SV){.items = str, .length = indices.items[i]};                  \
      da_append(&splits, split);                                               \
    }                                                                          \
    if (i == indices.length - 1) {                                             \
      split = (SV){.items = str + indices.items[i] + i + 1,                    \
                   .length = str_length - indices.items[i] - 1};               \
    } else {                                                                   \
      split = (SV){.items = str + indices.items[i] + i + 1,                    \
                   .length = indices.items[i + 1] - indices.items[i] - 1};     \
    }                                                                          \
    da_append(&splits, split);                                                 \
  }                                                                            \
  return splits;

SV_da cstr_split_by_char(const cstr str, char delim) {
  cstr_split_by(str, delim, cstr_index_by_char);
}
SV_da cstr_split_by_chars(const cstr str, char *delim) {
  cstr_split_by(str, delim, cstr_index_by_chars);
}

cstr_o cstr_from_sb(SB *sb) {
  cstr str = NULL;
  da_copy_items(&str, sb);
  return str;
}

cstr_o cstr_copy(const cstr str) {
  return (cstr_o)malloc_copy(strlen(str) + 1, str);
}

cstr_o cstr_concat_many(void *nil, ...) {
  SB sb = {};
  va_list vl;
  va_start(vl, nil);
  cstr arg = NULL;
  while ((arg = va_arg(vl, cstr)) != NULL) {
    sb_append_cstr(&sb, arg);
  }
  va_end(vl);
  cstr str = cstr_from_sb(&sb);
  da_free(&sb);
  return str;
}

bool cstr_ends_with(const cstr haystack, const cstr needle) {
  int haystack_length = strlen(haystack);
  int needle_length = strlen(needle);
  if (haystack_length < needle_length) {
    return false;
  }
  return strncmp(haystack + haystack_length - needle_length, needle,
                 needle_length) == 0;
}
bool cstr_starts_with(const cstr haystack, const cstr needle) {
  int haystack_length = strlen(haystack);
  int needle_length = strlen(needle);
  if (haystack_length < needle_length) {
    return false;
  }
  return strncmp(haystack, needle, needle_length) == 0;
}

cstr_o cstr_quote_copy(cstr arg) {
  char *pos = NULL;
  SB sb = {};
  if (strpbrk(arg, " \t\n\v\r\"") == NULL) {
    return cstr_copy(arg);
  }

  sb_append(&sb, '\"');
  pos = strpbrk(arg, "\\\"");
  uintptr_t index = 0;
  while (pos != NULL) {
    index = pos - arg;

    sb_append_sv(&sb, sv_from_cstr(arg, 0, index));
    sb_append(&sb, '\\');
    sb_append(&sb, arg[index]);

    arg = arg + index + 1;
    pos = strpbrk(arg, "\\\"");
  }
  sb_append(&sb, '\"');

  cstr res = cstr_from_sb(&sb);
  da_free(&sb);

  return res;
}

SV sv_from_sb(SB sb) {
  return (SV){
      .items = sb.items,
      .length = sb.length,
  };
}
SB sb_from_sv(SV sv) {
  return (SB){
      .items = sv.items,
      .length = sv.length,
      .capacity = sv.length,
  };
}
SV sv_from_cstr(const cstr str, int start, int end) {
  return (SV){.items = str + start, end - start};
}

SV sv_copy(SV sv) {
  return (SV){
      .items = (char *)malloc_copy(sv.length, sv.items),
      .length = sv.length,
  };
}

cstr_o sv_copy_to_cstr(SV sv) {
  cstr_o str = (cstr_o)calloc(1, sv.length + 1);
  memcpy(str, sv.items, sv.length);
  return str;
}

SV sv_from(SV sv, int index) {
  return (SV){
      .items = sv.items + index,
      .length = sv.length - index,
  };
}
SV sv_upto(SV sv, int index) {
  return (SV){
      .items = sv.items,
      .length = index,
  };
}

void sb_clear(SB *sb) {
  free(sb->items);
  sb->capacity = 0;
  sb->length = 0;
}
void sb_append(SB *sb, char c) {
  if (sb->length == 0) {
    da_reserve(sb, 2);
    sb->length = 1;
  }
  sb->items[sb->length - 1] = c;
  do {
    do {
      while (((sb))->capacity < ((sb)->length + 1)) {
        if (((sb))->capacity == 0)
          ((sb))->capacity = 16;
        else
          ((sb))->capacity *= 2;
      }
      ((sb))->items = (typeof(((sb))->items))realloc(
          ((sb))->items, ((sb))->capacity * sizeof(*((sb))->items));
    } while (0);
    (sb)->items[(sb)->length++] = ('\0');
  } while (0);
}

void sb_append_buf(SB *sb, const char *buf, int buf_length) {
  if (sb->length == 0) {
    sb->length = 1;
  }
  do {
    while ((sb)->capacity < (sb->length + buf_length)) {
      if ((sb)->capacity == 0)
        (sb)->capacity = 16;
      else
        (sb)->capacity *= 2;
    }
    (sb)->items = (typeof((sb)->items))realloc(
        (sb)->items, (sb)->capacity * sizeof(*(sb)->items));
  } while (0);
  memcpy(sb->items + sb->length - 1, buf, buf_length);
  sb->length += buf_length;
  sb->items[sb->length - 1] = '\0';
}
void sb_append_cstr(SB *sb, const cstr str) {
  sb_append_buf(sb, str, strlen(str));
}
void sb_append_sv(SB *sb, SV sv) { sb_append_buf(sb, sv.items, sv.length); }
void sb_append_sb(SB *sb, SB sb2) { sb_append_buf(sb, sb2.items, sb2.length); }

bool sv_from_file(SV_o *sv, const cstr path) {
  char result = true;
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

bool sb_append_file(SB *sb, const cstr path) {
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

int sv_get_char_index(SV sv, char c) {
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

// TODO: consider using strpbrk
int sv_get_first_char_index(SV sv, const cstr str) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int result = -1;
  int i = 0;
  int pattern_count = strlen(str);
  size_t *patterns = (size_t *)malloc(pattern_count * sizeof(size_t));

  while ((uintptr_t)(sv.items + i) % sizeof(size_t)) {
    if (i >= sv.length) {
      defer_res(-1);
    }
    if (strchr(str, sv.items[i]) != NULL) {
      defer_res(i);
    }
    i++;
  }

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
      defer_res(i);
    }
    i++;
  }
defer:
  free(patterns);
  return result;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int sv_get_word_index(SV sv, const cstr delim) {
  int delim_length = strlen(delim);
  int i = 0;
  SV temp = sv;
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
int sv_get_first_word_index(SV sv, const cstr const *delim, int count) {
  int *delim_lengths = (int *)malloc(sizeof(int) * count);
  char *heads = (char *)malloc(count);
  for (int i = 0; i < count; i++) {
    heads[i] = delim[i][0];
    delim_lengths[i] = strlen(delim[i]);
  }
  int result = 0;
  SV temp = sv;
  char matched = true;
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
