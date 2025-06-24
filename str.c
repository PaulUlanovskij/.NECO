#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
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

bool cstr_eql_vstr(cstr str, vstr vs) {
  if (vs.length != strlen(str)) {
    return false;
  }
  return strncmp(str, vs.items, vs.length) == 0;
}
bool cstr_eql_dstr(cstr str, dstr ds) {
  if (ds.length != strlen(str)) {
    return false;
  }
  return strncmp(str, ds.items, ds.length) == 0;
}
bool vstr_eql_cstr(vstr vs, cstr str) { return cstr_eql_vstr(str, vs); }
bool vstr_eql_dstr(vstr vs, dstr ds) {
  if (ds.length != vs.length) {
    return false;
  }
  return memcmp(vs.items, ds.items, ds.length) == 0;
}
bool dstr_eql_cstr(dstr ds, cstr str) { return cstr_eql_dstr(str, ds); }
bool dstr_eql_vstr(dstr ds, vstr vs) { return vstr_eql_dstr(vs, ds); }

int cstr_char_index(cstr str, char c) {
  char *ch = strchr(str, c);
  return ch == NULL ? -1 : ch - str;
}
int cstr_first_char_index(cstr str, int count, const char chars[count]) {
  char *ch = strpbrk(str, chars);
  return ch == NULL ? -1 : ch - str;
}
int cstr_word_index(cstr str, const cstr word) {
  char *ch = strstr(str, word);
  return ch == NULL ? -1 : ch - str;
}
int cstr_first_word_index(cstr str, int count, const cstr const words[count]) {
  int result = -1;
  cstr_o heads = malloc(count + 1);
  int_da lens = {};
  int len = strlen(str);

  for (int i = 0; i < count; i++) {
    heads[i] = words[i][0];
    da_append(&lens, strlen(words[i]));
  }

  int offset = 0;
  int index = first_char_index(str + offset, count, heads);
  while (index != -1) {
    for (int i = 0; i < count; i++) {
      if (offset + index < len - lens.items[i]) {
        if (strncmp(str + index + offset, words[i], lens.items[i]) == 0) {
          defer_res(offset + index);
        }
      }
    }
    offset += index + 1;
    index = first_char_index(str + offset, count, heads);
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

int vstr_first_char_index(vstr vs, int count, const char chars[count]) {
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
  int index = 0;
  while ((index = char_index(temp, word[0])) != -1) {
    if (offset + index < vs.length - word_len) {
      if (strncmp(vs.items + index + offset, word, word_len) == 0) {
        return offset + index;
      }
      offset += index + 1;
      temp = slice(temp, index + 1, -1);
    } else {
      break;
    }
  }
  return -1;
}
int vstr_first_word_index(vstr vs, int count, const cstr const words[count]) {
  int result = -1;
  cstr_o heads = malloc(count + 1);
  int_da lens = {};

  for (int i = 0; i < count; i++) {
    heads[i] = words[i][0];
    da_append(&lens, strlen(words[i]));
  }

  int offset = 0;
  int index = 0;
  while ((index = first_char_index(slice(vs, offset, -1), count, heads)) !=
         -1) {
    for (int i = 0; i < count; i++) {
      if (offset + index < vs.length - lens.items[i]) {
        if (strncmp(vs.items + index + offset, words[i], lens.items[i]) == 0) {
          defer_res(offset + index);
        }
      }
    }
    offset += index + 1;
  }

defer:
  free(heads);
  da_free(&lens);
  return result;
}

int dstr_char_index(dstr ds, char c) {
  return ds.length == 0 ? -1 : char_index(ds.items, c);
}
int dstr_first_char_index(dstr ds, int count, const char chars[count]) {
  return ds.length == 0 ? -1 : first_char_index(ds.items, count, chars);
}
int dstr_word_index(dstr ds, const cstr word) {
  return ds.length == 0 ? -1 : word_index(ds.items, word);
}
int dstr_first_word_index(dstr ds, int count, const cstr const words[count]) {
  return ds.length == 0 ? -1 : first_word_index(ds.items, count, words);
}

#define _index_by_char_word(type, name, pattern, index_to_add,           \
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

#define _index_by_chars_words(type, name, count, words, index_to_add, temp_reassign, index_func) \
  int_da indices = {};                                                         \
  type temp = name;                                                            \
  int index = index_func(name, count, words);                                  \
  while (index != -1) {                                                        \
    da_append(&indices, index_to_add);                                         \
    temp = temp_reassign;                                                      \
    index = index_func(temp, count, words);                                    \
  }                                                                            \
  return indices;

int_da cstr_index_char(const cstr str, char c) {
  _index_by_char_word(cstr, str, c, temp + index - str, temp + index + 1,
                            cstr_char_index);
}
int_da cstr_index_chars(const cstr str, int count, const char chars[count]) {
  _index_by_chars_words(cstr, str, count, chars, temp + index - str,
                            temp + index + 1, cstr_first_char_index);
}
int_da vstr_index_char(vstr vs, char c) {
  _index_by_char_word(vstr, vs, c, temp.items + index - vs.items,
                            slice(temp, index + 1, -1), vstr_char_index);
}
int_da vstr_index_chars(vstr vs, int count, const char chars[count]) {
  _index_by_chars_words(vstr, vs, count, chars, temp.items + index - vs.items,
                            slice(temp, index + 1, -1),
                            vstr_first_char_index);
}
int_da dstr_index_char(dstr ds, char c) {
  return ds.length == 0 ? (int_da){} : index_char(ds.items, c);
}
int_da dstr_index_chars(dstr ds, int count, const char chars[count]) {
  return ds.length == 0 ? (int_da){} : index_chars(ds.items, count, chars);
}
int_da cstr_index_word(const cstr str, const cstr word) {
  _index_by_char_word(cstr, str, word, temp + index - str,
                            temp + index + 1, cstr_word_index);
}
int_da cstr_index_words(const cstr str, int count, const cstr const words[count]) {
  _index_by_chars_words(cstr, str, count, words, temp + index - str, temp + index + 1,
                  cstr_first_word_index);
}
int_da vstr_index_word(vstr vs, const cstr word) {
  _index_by_char_word(vstr, vs, word, temp.items + index - vs.items,
                            slice(temp, index + 1, -1), vstr_word_index);
}
int_da vstr_index_words(vstr vs, int count, const cstr const words[count]) {
  _index_by_chars_words(vstr, vs, count, words, temp.items + index - vs.items,
                  slice(temp, index + 1, -1), vstr_first_word_index);
}
int_da dstr_index_word(dstr ds, const cstr word) {
  return ds.length == 0 ? (int_da){} : index_word(ds.items, word);
}
int_da dstr_index_words(dstr ds, int count, const cstr const words[count]) {
  return ds.length == 0 ? (int_da){} : index_words(ds.items, count, words);
}

#define _split_by_char_word(name, pattern, index_func, sso)              \
  vstr_da splits = {};                                                         \
  int_da indices = index_func(name, pattern);                                  \
  if (indices.length == 0) {                                                   \
    da_append(&splits, slice(name, 0, -1));                                    \
    return splits;                                                             \
  }                                                                            \
  vstr split = slice(name, 0, indices.items[0]);                               \
  if(sso & SSO_TRIM_ENTRIES)                                                   \
    split = trim(split);                                                       \
  if ((split.length == 0 && (sso & SSO_REMOVE_EMPTY)) == false) {              \
    da_append(&splits, split);                                                 \
  }                                                                            \
  for (int i = 0; i < indices.length - 1; i++) {                               \
    split = slice(name, indices.items[i] + i + 1, indices.items[i + 1]);       \
    if(sso & SSO_TRIM_ENTRIES)                                                 \
      split = trim(split);                                                     \
    if ((split.length == 0 && (sso & SSO_REMOVE_EMPTY)) == false) {            \
      da_append(&splits, split);                                               \
    }                                                                          \
  }                                                                            \
  split = slice(name, indices.items[indices.length - 1] + indices.length, -1); \
  if(sso & SSO_TRIM_ENTRIES)                                                   \
    split = trim(split);                                                       \
  if ((split.length == 0 && (sso & SSO_REMOVE_EMPTY)) == false) {              \
    da_append(&splits, split);                                                 \
  }                                                                            \
  da_free(&indices);                                                           \
  return splits;

#define _split_by_chars_words(name, words, count, index_func, sso)             \
  vstr_da splits = {};                                                         \
  int_da indices = index_func(name, words, count);                             \
  if (indices.length == 0) {                                                   \
    da_append(&splits, slice(name, 0, -1));                                    \
    return splits;                                                             \
  }                                                                            \
  vstr split = slice(name, 0, indices.items[0]);                               \
  if(sso & SSO_TRIM_ENTRIES)                                                   \
    split = trim(split);                                                       \
  if ((split.length == 0 && (sso & SSO_REMOVE_EMPTY)) == false) {              \
    da_append(&splits, split);                                                 \
  }                                                                            \
  for (int i = 0; i < indices.length - 1; i++) {                               \
    split = slice(name, indices.items[i] + i + 1, indices.items[i + 1]);       \
    if(sso & SSO_TRIM_ENTRIES)                                                 \
      split = trim(split);                                                     \
    if ((split.length == 0 && (sso & SSO_REMOVE_EMPTY)) == false) {            \
      da_append(&splits, split);                                               \
    }                                                                          \
  }                                                                            \
  split = slice(name, indices.items[indices.length - 1] + indices.length, -1); \
  if(sso & SSO_TRIM_ENTRIES)                                                   \
    split = trim(split);                                                       \
  if ((split.length == 0 && (sso & SSO_REMOVE_EMPTY)) == false) {              \
    da_append(&splits, split);                                                 \
  }                                                                            \
  da_free(&indices);                                                           \
  return splits;

vstr_da cstr_split_by_char(const cstr str, char c, StrSplitOptions sso) {
  _split_by_char_word(str, c, index_char, sso);
}
vstr_da cstr_split_by_chars(const cstr str, int count, const char chars[count], StrSplitOptions sso) {
  _split_by_chars_words(str, count, chars, index_chars, sso);
}
vstr_da vstr_split_by_char(vstr vs, char c, StrSplitOptions sso) {
  _split_by_char_word(vs, c, index_char, sso);
}
vstr_da vstr_split_by_chars(vstr vs, int count, const char chars[count], StrSplitOptions sso) {
  _split_by_chars_words(vs, count, chars, index_chars, sso);
}
vstr_da dstr_split_by_char(dstr ds, char c, StrSplitOptions sso) {
  return split_by_char(ds.items, c, sso);
}
vstr_da dstr_split_by_chars(dstr ds, int count, const char chars[count], StrSplitOptions sso) {
  return split_by_chars(ds.items, count, chars, sso);
}
vstr_da cstr_split_by_word(const cstr str, const cstr word, StrSplitOptions sso) {
  _split_by_char_word(str, word, index_word, sso);
}
vstr_da cstr_split_by_words(const cstr str, int count, const cstr const words[count], StrSplitOptions sso) {
  _split_by_chars_words(str, count, words, index_words, sso);
}
vstr_da vstr_split_by_word(vstr vs, const cstr word, StrSplitOptions sso) {
  _split_by_char_word(vs, word, index_word, sso);
}
vstr_da vstr_split_by_words(vstr vs, int count, const cstr const words[count], StrSplitOptions sso) {
  _split_by_chars_words(vs, count, words, index_words, sso);
}
vstr_da dstr_split_by_word(dstr ds, const cstr word, StrSplitOptions sso) {
  return split_by_word(ds.items, word, sso);
}
vstr_da dstr_split_by_words(dstr ds, int count, const cstr const words[count], StrSplitOptions sso) {
  return split_by_words(ds.items, count, words, sso);
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
void dstr_appendf(dstr *ds, cstr format, ...) {
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
  while ((index = first_char_index(str, len("\\\""), "\\\"")) != -1) {
    dstr_append_vstr(&ds, slice(str, 0, index));
    dstr_append(&ds, '\\');
    dstr_append(&ds, str[index]);

    str += index + 1;
  }
  dstr_append(&ds, '\"');

  cstr_o res = dstr_to_cstr(ds);
  da_free(&ds);

  return res;
}

cstr_o cstr_sprintf(cstr format, ...) {
  va_list vl;
  va_start(vl, format);
  cstr_o res = cstr_vsprintf(format, vl);
  va_end(vl);
  return res;
}

cstr_o cstr_vsprintf(cstr format, va_list va) {
  dstr ds = {};
  dstr_reserve_append(&ds, 1024);
  while (vsnprintf(ds.items, ds.length, format, va) == ds.length - 1) {
    dstr_reserve_append(&ds, 1024);
  }

  cstr_o res = dstr_to_cstr(ds);
  da_free(&ds);
  return res;
}

cstr_o cstr_concat_many(...) {
  dstr ds = {};
  va_list vl;
  va_start(vl);
  cstr arg = NULL;
  while ((arg = va_arg(vl, cstr)) != NULL) {
    dstr_append_cstr(&ds, arg);
  }
  va_end(vl);
  cstr_o str = dstr_to_cstr(ds);
  da_free(&ds);
  return str;
}

vstr cstr_capture_by_char(cstr str, int offset, char inc, char dec) {
  cstr data = offset(str, offset);
  int index = char_index(data, inc);
  if (index == -1) {
    return (vstr){};
  }
  int start_index = index;
  data = offset(data, index + 1);
  int balance = 1;
  while (balance > 0) {
    index = first_char_index(data, 2, ((char[2]){inc, dec}));
    if (index == -1) {
      return (vstr){};
    }
    data += index;
    if (data[0] == dec) {
      balance--;
    } else {
      balance++;
    }
    data++;
  }
  return slice(offset(str, offset + start_index), 0,
                    data - (str + offset + start_index));
}
vstr cstr_capture_by_word(cstr str, int offset, const cstr inc,
                                const cstr dec) {
  cstr data = offset(str, offset);
  int index = word_index(data, inc);
  if (index == -1) {
    return (vstr){};
  }
  int start_index = index;
  data = offset(data, index + strlen(inc));
  int balance = 1;
  while (balance > 0) {
    index = first_word_index(data, 2, ((cstr[2]){inc, dec}));
    if (index == -1) {
      return (vstr){};
    }
    data = offset(data, index);
    if (starts_with(data, dec)) {
      balance--;
      data = offset(data, strlen(dec));
    } else {
      balance++;
      data = offset(data, strlen(inc));
    }
  }
  return slice(offset(str, offset + start_index), 0,
                    data - (str + offset + start_index));
}
vstr vstr_capture_by_char(vstr vs, int offset, char inc, char dec) {
  vstr data = offset(vs, offset);
  int index = char_index(data, inc);
  if (index == -1) {
    return (vstr){};
  }
  int start_index = index;
  data = offset(data, index + 1);
  int balance = 1;
  while (balance > 0) {
    index = first_char_index(data, 2, ((char[2]){inc, dec}));
    if (index == -1) {
      return (vstr){};
    }
    data = offset(data, index);
    if (data.items[0] == dec) {
      balance--;
    } else {
      balance++;
    }
    data = offset(data, 1);
  }
  return slice(offset(vs, offset + start_index), 0,
                    data.items - (vs.items + offset + start_index));
}
vstr vstr_capture_by_word(vstr vs, int offset, const cstr inc,
                                const cstr dec) {
  vstr data = offset(vs, offset);
  int index = word_index(data, inc);
  if (index == -1) {
    return (vstr){};
  }
  int start_index = index;
  data = offset(data, index + strlen(inc));
  int balance = 1;
  while (balance > 0) {
    index = first_word_index(data, 2, ((cstr[2]){inc, dec}));
    if (index == -1) {
      return (vstr){};
    }
    data = offset(data, index);
    if (starts_with(data, dec)) {
      balance--;
      data = offset(data, strlen(dec));
    } else {
      balance++;
      data = offset(data, strlen(inc));
    }
  }
  return slice(offset(vs, offset + start_index), 0,
                    data.items - (vs.items + offset + start_index));
}
vstr dstr_capture_by_char(dstr ds, int offset, char inc, char dec) {
  return ds.items == 0 ? (vstr){}
                       : capture_by_char(ds.items, offset, inc, dec);
}
vstr dstr_capture_by_word(dstr ds, int offset, const cstr inc,
                                const cstr dec) {
  return ds.items == 0 ? (vstr){}
                       : capture_by_word(ds.items, offset, inc, dec);
}

#define _count_char_word(name, delim_type, delim)                        \
  int count = 0;                                                               \
  for (int index = 0;                                                          \
       (index = delim_type##_index(name, delim)) != -1; count++) {    \
    name = offset(name, index + 1);                                     \
  }                                                                            \
  return count;

int cstr_count_char(cstr str, char c) { _count_char_word(str, char, c); }
int cstr_count_word(cstr str, const cstr word) {
  _count_char_word(str, word, word);
}
int vstr_count_char(vstr vs, char c) { _count_char_word(vs, char, c); }
int vstr_count_word(vstr vs, const cstr word) {
  _count_char_word(vs, word, word);
}
int dstr_count_char(dstr ds, char c) {
  return ds.length == 0 ? 0 : count_char(ds.items, c);
}
int dstr_count_word(dstr ds, const cstr word) {
  return ds.length == 0 ? 0 : count_word(ds.items, word);
}

vstr cstr_trim(const cstr str) {
  int len = strlen(str);
  int start_index = 0;
  for (; start_index < len && isspace(str[start_index]); start_index++) {
  }
  if (start_index == len - 1) {
    return (vstr){};
  }
  int end_index = len - 1;
  for (; isspace(str[end_index]); end_index--) {
  }
  return slice(str, start_index, end_index + 1);
}
vstr vstr_trim(vstr vs) {
  int start_index = 0;
  for (; start_index < vs.length && isspace(vs.items[start_index]);
       start_index++) {
  }
  if (start_index == vs.length - 1) {
    return (vstr){};
  }
  int end_index = vs.length - 1;
  for (; isspace(vs.items[end_index]); end_index--) {
  }
  return slice(vs, start_index, end_index + 1);
}
vstr dstr_trim(dstr ds) {
  return ds.length == 0 ? (vstr){} : cstr_trim(ds.items);
}
