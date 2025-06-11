#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/da.h"
#include "headers/fio.h"
#include "headers/helpers.h"
#include "headers/mem.h"
#include "headers/str.h"

cstr cstr_offset(const cstr str, int offset) { return str + offset; }
vstr vstr_offset(vstr vs, int offset) {
  return (vstr){.items = vs.items + offset, .length = vs.length - offset};
}
// dstr_offset is absent deliberately as it would go against what dstr promises
// to user

vstr cstr_slice(const cstr str, int begin, int end) {
  return (vstr){.items = str + begin,
                .length = (end == -1 ? strlen(str) : end) - begin};
}
vstr vstr_slice(vstr vs, int begin, int end) {
  return (vstr){.items = vs.items + begin,
                .length = (end == -1 ? vs.length : end) - begin};
}
vstr dstr_slice(dstr ds, int begin, int end) {
  return (vstr){.items = ds.items + begin,
                .length = (end == -1 ? ds.length : end) - begin};
}

cstr_o cstr_copy(const cstr str) { return malloc_copy(strlen(str) + 1, str); }
vstr_o vstr_copy(vstr vs) {
  return (vstr_o){.items = malloc_copy(vs.length, vs.items),
                  .length = vs.length};
}
dstr_o dstr_copy(dstr ds) {
  return (dstr_o){.items = malloc_copy(ds.capacity, ds.items),
                  .length = ds.length,
                  .capacity = ds.capacity};
}

vstr_o cstr_to_vstr(const cstr str) {
  return (vstr){.items = malloc_copy(strlen(str), str), .length = strlen(str)};
}
dstr_o cstr_to_dstr(const cstr str) {
  dstr_o ds = {};
  int len = strlen(str);
  da_reserve(&ds, len + 1);
  memcpy(ds.items, str, len + 1);
  ds.length = len + 1;
  return ds;
}
dstr_o vstr_to_dstr(vstr vs) {
  dstr_o ds = {};
  da_reserve(&ds, vs.length);
  memcpy(ds.items, vs.items, vs.length);
  ds.length = vs.length;
  return ds;
}
cstr_o vstr_to_cstr(vstr vs) {
  cstr_o str = (cstr_o)malloc(vs.length + 1);
  memcpy(str, vs.items, vs.length);
  str[vs.length] = '\0';
  return str;
}
cstr_o dstr_to_cstr(dstr ds) {
  return (cstr_o)malloc_copy(ds.length, ds.items);
}
vstr_o dstr_to_vstr(dstr ds) {
  return (vstr_o){.items = malloc_copy(ds.length, ds.items),
                  .length = ds.length};
}

bool cstr_ends_with(const cstr str, const cstr pattern) {
  int str_length = strlen(str);
  int pattern_length = strlen(pattern);
  if (str_length < pattern_length) {
    return false;
  }
  return strncmp(str + str_length - pattern_length, pattern, pattern_length) ==
         0;
}
bool cstr_starts_with(const cstr str, const cstr pattern) {
  int str_length = strlen(str);
  int pattern_length = strlen(pattern);
  if (str_length < pattern_length) {
    return false;
  }
  return strncmp(str, pattern, pattern_length) == 0;
}

bool vstr_ends_with(vstr vs, const cstr pattern) {
  int pattern_length = strlen(pattern);
  if (vs.length < pattern_length) {
    return false;
  }
  return strncmp(vs.items + vs.length - pattern_length, pattern,
                 pattern_length) == 0;
}
bool vstr_starts_with(vstr vs, const cstr pattern) {
  int pattern_length = strlen(pattern);
  if (vs.length < pattern_length) {
    return false;
  }
  return strncmp(vs.items, pattern, pattern_length) == 0;
}

bool dstr_ends_with(dstr ds, const cstr pattern) {
  int pattern_length = strlen(pattern);
  if (ds.length - 1 < pattern_length) {
    return false;
  }
  return strncmp(ds.items + ds.length - pattern_length - 1, pattern,
                 pattern_length) == 0;
}
bool dstr_starts_with(dstr ds, const cstr pattern) {
  int pattern_length = strlen(pattern);
  if (ds.length - 1 < pattern_length) {
    return false;
  }
  return strncmp(ds.items, pattern, pattern_length) == 0;
}

void dstr_append(dstr *ds, char c) {
  dstr_reserve_append(ds, 1);
  ds->items[ds->length - 1] = c;
  ds->items[ds->length++] = ('\0');
}
void dstr_append_buf(dstr *ds, const char *buf, int buf_length) {
  dstr_reserve_append(ds, buf_length);
  memcpy(ds->items + ds->length - 1, buf, buf_length);
  ds->length += buf_length;
  ds->items[ds->length - 1] = '\0';
}
void dstr_append_cstr(dstr *ds, const cstr str) {
  dstr_append_buf(ds, str, strlen(str));
}
void dstr_append_vstr(dstr *ds, vstr vs) {
  dstr_append_buf(ds, vs.items, vs.length);
}
void dstr_append_dstr(dstr *ds, dstr ds2) {
  dstr_append_buf(ds, ds2.items, ds2.length - 1);
}
bool dstr_append_file(dstr *ds, const cstr path) {
  bool result = true;
  FILE *file = fopen(path, "rb");
  int m = flength(file);
  if (m < 0) {
    defer_res(false);
  }
  dstr_reserve_append(ds, m);
  fread(ds->items + ds->length - 1, m, 1, file);
  if (ferror(file)) {
    defer_res(false);
  }
  ds->length += m;
  ds->items[ds->length - 1] = '\0';

defer:
  fclose(file);
  return result;
}

bool cstr_from_file(cstr_o *str, const cstr path) {
  dstr_o ds = {};
  if (dstr_from_file(&ds, path) == false) {
    return false;
  }
  *str = dstr_to_cstr(ds);
  da_free(&ds);
  return true;
}
bool vstr_from_file(vstr_o *vs, const cstr path) {
  dstr_o ds = {};
  if (dstr_from_file(&ds, path) == false) {
    return false;
  }
  *vs = dstr_to_vstr(ds);
  da_free(&ds);
  return true;
}
bool dstr_from_file(dstr_o *ds, const cstr path) {
  return dstr_append_file(ds, path);
}

bool cstr_cmp_vstr(cstr str, vstr vs) {
  if (vs.length != strlen(str)) {
    return false;
  }
  return strncmp(str, vs.items, vs.length) == 0;
}
bool cstr_cmp_dstr(cstr str, dstr ds) {
  if (ds.length != strlen(str)) {
    return false;
  }
  return strncmp(str, ds.items, ds.length) == 0;
}
bool vstr_cmp_cstr(vstr vs, cstr str) { return cstr_cmp_vstr(str, vs); }
bool vstr_cmp_dstr(vstr vs, dstr ds) {
  if (ds.length != vs.length) {
    return false;
  }
  return memcmp(vs.items, ds.items, ds.length) == 0;
}
bool dstr_cmp_cstr(dstr ds, cstr str) { return cstr_cmp_dstr(str, ds); }
bool dstr_cmp_vstr(dstr ds, vstr vs) { return vstr_cmp_dstr(vs, ds); }

int cstr_char_index(cstr str, char c) {
  char *ch = strchr(str, c);
  return ch == NULL ? -1 : ch - str;
}
int cstr_first_char_index(cstr str, const char *chars) {
  char *ch = strpbrk(str, chars);
  return ch == NULL ? -1 : ch - str;
}
int cstr_word_index(cstr str, const cstr word) {
  char *ch = strstr(str, word);
  return ch == NULL ? -1 : ch - str;
}
int cstr_first_word_index(cstr str, const cstr const *words, int count) {
  int result = -1;
  cstr_o heads = malloc(count + 1);
  int_da lens = {};
  int len = strlen(str);

  for (int i = 0; i < count; i++) {
    heads[i] = words[i][0];
    da_append(&lens, strlen(words[i]));
  }

  int offset = 0;
  int index = cstr_first_char_index(str + offset, heads);
  while (index != -1) {
    for (int i = 0; i < count; i++) {
      if (offset + index < len - lens.items[i]) {
        if (strncmp(str + index + offset, words[i], lens.items[i]) == 0) {
          defer_res(offset + index);
        }
      }
    }
    offset += index + 1;
    index = cstr_first_char_index(str + offset, heads);
  }

defer:
  free(heads);
  da_free(&lens);
  return result;
}

int vstr_char_index(vstr vs, char c) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = 0;
  for (; (uintptr_t)(vs.items + i) % sizeof(size_t); i++) {
    if (i >= vs.length) {
      return -1;
    }
    if (vs.items[i] == c) {
      return i;
    }
  }

  size_t pattern = ONES * (unsigned char)c;
  size_t *word_ptr = (size_t *)(vs.items + i);

  for (; i + sizeof(size_t) < vs.length; i += sizeof(size_t), word_ptr++) {
    if (HASZERO(*word_ptr ^ pattern)) {
      break;
    }
  }

  for (; i < vs.length; i++) {
    if (vs.items[i] == c) {
      return i;
    }
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int vstr_first_char_index(vstr vs, const char *chars) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int result = -1;
  int i = 0;
  int pattern_count = strlen(chars);
  size_t *patterns = (size_t *)malloc(pattern_count * sizeof(size_t));

  for (; (uintptr_t)(vs.items + i) % sizeof(size_t); i++) {
    if (i >= vs.length) {
      defer_res(-1);
    }
    if (strchr(chars, vs.items[i]) != NULL) {
      defer_res(i);
    }
  }

  for (int j = 0; j < pattern_count; j++) {
    patterns[j] = ONES * (unsigned char)chars[j];
  }
  size_t *word_ptr = (size_t *)(vs.items + i);

  for (; i + sizeof(size_t) < vs.length; i += sizeof(size_t), word_ptr++) {
    for (int j = 0; j < pattern_count; j++) {
      if (HASZERO(*word_ptr ^ patterns[j])) {
        goto seek_single;
      }
    }
  }
seek_single:
  for (; i < vs.length; i++) {
    if (strchr(chars, vs.items[i]) != NULL) {
      defer_res(i);
    }
  }
defer:
  free(patterns);
  return result;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int vstr_word_index(vstr vs, const cstr word) {
  int word_len = strlen(word);
  int offset = 0;
  vstr temp = vs;
  temp.length -= (word_len - 1);
  int index = vstr_char_index(temp, word[0]);
  while (index != -1) {
    if (offset + index < vs.length - word_len) {
      if (strncmp(vs.items + index + offset, word, word_len) == 0) {
        return offset + index;
      }
      offset += index + 1;
      temp = vstr_slice(temp, index + 1, -1);
    } else {
      break;
    }
  }
  return -1;
}
int vstr_first_word_index(vstr vs, const cstr const *words, int count) {
  int result = -1;
  cstr_o heads = malloc(count + 1);
  int_da lens = {};

  for (int i = 0; i < count; i++) {
    heads[i] = words[i][0];
    da_append(&lens, strlen(words[i]));
  }

  int offset = 0;
  int index = vstr_first_char_index(vstr_slice(vs, offset, -1), heads);
  while (index != -1) {
    for (int i = 0; i < count; i++) {
      if (offset + index < vs.length - lens.items[i]) {
        if (strncmp(vs.items + index + offset, words[i], lens.items[i]) == 0) {
          defer_res(offset + index);
        }
      }
    }
    offset += index + 1;
    index = vstr_first_char_index(vstr_slice(vs, offset, -1), heads);
  }

defer:
  free(heads);
  da_free(&lens);
  return result;
}

int dstr_char_index(dstr ds, char c) {
  return ds.length == 0 ? -1 : cstr_char_index(ds.items, c);
}
int dstr_first_char_index(dstr ds, const char *chars) {
  return ds.length == 0 ? -1 : cstr_first_char_index(ds.items, chars);
}
int dstr_word_index(dstr ds, const cstr word) {
  return ds.length == 0 ? -1 : cstr_word_index(ds.items, word);
}
int dstr_first_word_index(dstr ds, const cstr const *words, int count) {
  return ds.length == 0 ? -1 : cstr_first_word_index(ds.items, words, count);
}

#define _index_by_char_chars_word(type, name, pattern, index_to_add,           \
                                  temp_reassign, index_func)                   \
  int_da indices = {};                                                         \
  type temp = name;                                                            \
  int index = index_func(name, pattern);                                       \
  while (index != -1) {                                                        \
    da_append(&indices, index_to_add);                                         \
    temp = temp_reassign;                                                      \
    index = index_func(temp, pattern);                                         \
  }                                                                            \
  return indices;

#define _index_by_words(type, name, words, count, index_to_add, temp_reassign, \
                        index_func)                                            \
  int_da indices = {};                                                         \
  type temp = name;                                                            \
  int index = index_func(name, words, count);                                  \
  while (index != -1) {                                                        \
    da_append(&indices, index_to_add);                                         \
    temp = temp_reassign;                                                      \
    index = index_func(temp, words, count);                                    \
  }                                                                            \
  return indices;

int_da cstr_index_char(const cstr str, char c) {
  _index_by_char_chars_word(cstr, str, c, temp + index - str, temp + index + 1,
                            cstr_char_index);
}
int_da cstr_index_chars(const cstr str, const char *chars) {
  _index_by_char_chars_word(cstr, str, chars, temp + index - str,
                            temp + index + 1, cstr_first_char_index);
}
int_da vstr_index_char(vstr vs, char c) {
  _index_by_char_chars_word(vstr, vs, c, temp.items + index - vs.items,
                            vstr_slice(temp, index + 1, -1), vstr_char_index);
}
int_da vstr_index_chars(vstr vs, const char *chars) {
  _index_by_char_chars_word(vstr, vs, chars, temp.items + index - vs.items,
                            vstr_slice(temp, index + 1, -1),
                            vstr_first_char_index);
}
int_da dstr_index_char(dstr ds, char c) {
  return ds.length == 0 ? (int_da){} : cstr_index_char(ds.items, c);
}
int_da dstr_index_chars(dstr ds, const char *chars) {
  return ds.length == 0 ? (int_da){} : cstr_index_chars(ds.items, chars);
}
int_da cstr_index_word(const cstr str, const cstr word) {
  _index_by_char_chars_word(cstr, str, word, temp + index - str,
                            temp + index + 1, cstr_word_index);
}
int_da cstr_index_words(const cstr str, const cstr const *words, int count) {
  _index_by_words(cstr, str, words, count, temp + index - str, temp + index + 1,
                  cstr_first_word_index);
}
int_da vstr_index_word(vstr vs, const cstr word) {
  _index_by_char_chars_word(vstr, vs, word, temp.items + index - vs.items,
                            vstr_slice(temp, index + 1, -1), vstr_word_index);
}
int_da vstr_index_words(vstr vs, const cstr const *words, int count) {
  _index_by_words(vstr, vs, words, count, temp.items + index - vs.items,
                  vstr_slice(temp, index + 1, -1), vstr_first_word_index);
}
int_da dstr_index_word(dstr ds, const cstr word) {
  return ds.length == 0 ? (int_da){} : cstr_index_word(ds.items, word);
}
int_da dstr_index_words(dstr ds, const cstr const *words, int count) {
  return ds.length == 0 ? (int_da){} : cstr_index_words(ds.items, words, count);
}

#define _split_by_char_chars_word(type, name, pattern, index_func)             \
  vstr_da splits = {};                                                         \
  int_da indices = index_func(name, pattern);                                  \
  if (indices.length == 0) {                                                   \
    return splits;                                                             \
  }                                                                            \
  da_append(&splits, type##_slice(name, 0, indices.items[0]));                 \
  for (int i = 0; i < indices.length - 1; i++) {                               \
    da_append(&splits, type##_slice(name, indices.items[i] + i + 1,            \
                                    indices.items[i + 1]));                    \
  }                                                                            \
  da_append(&splits,                                                           \
            type##_slice(name,                                                 \
                         indices.items[indices.length - 1] + indices.length,   \
                         -1));                                                 \
  da_free(&indices);                                                           \
  return splits;

#define _split_by_words(type, name, words, count, index_func)                  \
  vstr_da splits = {};                                                         \
  int_da indices = index_func(name, words, count);                             \
  if (indices.length == 0) {                                                   \
    return splits;                                                             \
  }                                                                            \
  da_append(&splits, type##_slice(name, 0, indices.items[0]));                 \
  for (int i = 0; i < indices.length - 1; i++) {                               \
    da_append(&splits, type##_slice(name, indices.items[i] + i + 1,            \
                                    indices.items[i + 1]));                    \
  }                                                                            \
  da_append(&splits,                                                           \
            type##_slice(name,                                                 \
                         indices.items[indices.length - 1] + indices.length,   \
                         -1));                                                 \
  da_free(&indices);                                                           \
  return splits;

vstr_da cstr_split_by_char(const cstr str, char c) {
  _split_by_char_chars_word(cstr, str, c, cstr_index_char);
}
vstr_da cstr_split_by_chars(const cstr str, const char *chars) {
  _split_by_char_chars_word(cstr, str, chars, cstr_index_chars);
}
vstr_da vstr_split_by_char(vstr vs, char c) {
  _split_by_char_chars_word(vstr, vs, c, vstr_index_char);
}
vstr_da vstr_split_by_chars(vstr vs, const char *chars) {
  _split_by_char_chars_word(vstr, vs, chars, vstr_index_chars);
}
vstr_da dstr_split_by_char(dstr ds, char c) {
  _split_by_char_chars_word(dstr, ds, c, dstr_index_char);
}
vstr_da dstr_split_by_chars(dstr ds, const char *chars) {
  _split_by_char_chars_word(dstr, ds, chars, dstr_index_chars);
}
vstr_da cstr_split_by_word(const cstr str, const cstr word) {
  _split_by_char_chars_word(cstr, str, word, cstr_index_word);
}
vstr_da cstr_split_by_words(const cstr str, const cstr const *words,
                            int count) {
  _split_by_words(cstr, str, words, count, cstr_index_words);
}
vstr_da vstr_split_by_word(vstr vs, const cstr word) {
  _split_by_char_chars_word(vstr, vs, word, vstr_index_word);
}
vstr_da vstr_split_by_words(vstr vs, const cstr const *words, int count) {
  _split_by_words(vstr, vs, words, count, vstr_index_words);
}
vstr_da dstr_split_by_word(dstr ds, const cstr word) {
  return cstr_split_by_word(ds.items, word);
}
vstr_da dstr_split_by_words(dstr ds, const cstr const *words, int count) {
  return cstr_split_by_words(ds.items, words, count);
}

void dstr_reserve_append(dstr *ds, int capacity) {
  if (ds->length == 0) {
    da_reserve(ds, capacity + 1);
    ds->length = 1;
  } else {
    da_reserve(ds, ds->length + capacity);
  }
}
void dstr_rewind(dstr *ds) { ds->length = 0; }
void dstr_appendf(dstr *ds, cstr format, ...){
  va_list vl;
  va_start(vl, format);
  cstr_o str = cstr_vsprintf(format, vl);
  va_end(vl);
  dstr_append_cstr(ds, str);
  free(str);
}

cstr_o cstr_quote(cstr str) {
  dstr ds = {};
  if (strpbrk(str, " \t\n\v\r\"") == NULL) {
    return cstr_copy(str);
  }

  int index = 0;
  dstr_append(&ds, '\"');
  while ((index = cstr_first_char_index(str, "\\\"")) != -1) {
    dstr_append_vstr(&ds, cstr_slice(str, 0, index));
    dstr_append(&ds, '\\');
    dstr_append(&ds, str[index]);

    str += index + 1;
  }
  dstr_append(&ds, '\"');

  cstr_o res = dstr_to_cstr(ds);
  da_free(&ds);

  return res;
}

cstr_o cstr_sprintf(cstr format, ...){
  va_list vl;
  va_start(vl, format);
  cstr_o res = cstr_vsprintf(format, vl);
  va_end(vl);
  return res;
}

cstr_o cstr_vsprintf(cstr format, va_list va){
  dstr ds = {};
  dstr_reserve_append(&ds, 1024);
  while (vsnprintf(ds.items, ds.length, format, va) == ds.length - 1) {
    dstr_reserve_append(&ds, 1024);
  }

  cstr_o res = dstr_to_cstr(ds);
  da_free(&ds);
  return res;
}

cstr_o cstr_concat_many(void *nil, ...) {
  dstr ds = {};
  va_list vl;
  va_start(vl, nil);
  cstr arg = NULL;
  while ((arg = va_arg(vl, cstr)) != NULL) {
    dstr_append_cstr(&ds, arg);
  }
  va_end(vl);
  cstr_o str = dstr_to_cstr(ds);
  da_free(&ds);
  return str;
}
