#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "da.h"
#include "fio.h"
#include "helpers.h"
#include "mem.h"
#include "str.h"

int cstr_length(cstr str) {return strlen(str);}
int vstr_length(vstr str) {return str.length;}
int dstr_length(dstr str) {return str.length-1;}

char* cstr_items(cstr str) {return str;}
char* vstr_items(vstr str) {return str.items;}
char* dstr_items(dstr str) {return str.items;}

cstr cstr_offset(cstr str, int offset) { return str + offset; }
vstr vstr_offset(vstr vs, int offset) {
  int len = vs.length - offset;
  return (vstr){.items = vs.items + offset, .length = len < 0 ? 0 : len};
}
// dstr_offset is absent deliberately as it would go against what dstr promises
// to user

vstr cstr_slice(cstr str, int begin, int end) {
  int len = (end == -1 ? strlen(str) : end) - begin; 
  return (vstr){.items = str + begin,
                .length = len < 0 ? 0 : len};
}
vstr vstr_slice(vstr vs, int begin, int end) {
  int len = (end == -1 ? vs.length : end) - begin; 
  return (vstr){.items = vs.items + begin,
                .length = len < 0 ? 0 : len};
}
vstr dstr_slice(dstr ds, int begin, int end) {
  int len = (end == -1 ? ds.length : end) - begin; 
  return (vstr){.items = ds.items + begin,
                .length = len < 0 ? 0 : len};
}

cstr_o cstr_copy(cstr str) { return malloc_copy(strlen(str) + 1, str); }
vstr_o vstr_copy(vstr vs) {
  return (vstr_o){.items = malloc_copy(vs.length, vs.items),
                  .length = vs.length};
}
dstr_o dstr_copy(dstr ds) {
  return (dstr_o){.items = malloc_copy(ds.capacity, ds.items),
                  .length = ds.length,
                  .capacity = ds.capacity};
}

vstr_o cstr_to_vstr(cstr str) {
  return (vstr){.items = malloc_copy(strlen(str), str), .length = strlen(str)};
}
dstr_o cstr_to_dstr(cstr str) {
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

bool cstr_ends_with(cstr str, cstr pattern) {
  int str_length = strlen(str);
  int pattern_length = strlen(pattern);
  if (str_length < pattern_length) {
    return false;
  }
  return strncmp(str + str_length - pattern_length, pattern, pattern_length) ==
         0;
}
bool cstr_starts_with(cstr str, cstr pattern) {
  int str_length = strlen(str);
  int pattern_length = strlen(pattern);
  if (str_length < pattern_length) {
    return false;
  }
  return strncmp(str, pattern, pattern_length) == 0;
}

bool vstr_ends_with(vstr vs, cstr pattern) {
  int pattern_length = strlen(pattern);
  if (vs.length < pattern_length) {
    return false;
  }
  return strncmp(vs.items + vs.length - pattern_length, pattern,
                 pattern_length) == 0;
}
bool vstr_starts_with(vstr vs, cstr pattern) {
  int pattern_length = strlen(pattern);
  if (vs.length < pattern_length) {
    return false;
  }
  return strncmp(vs.items, pattern, pattern_length) == 0;
}

bool dstr_ends_with(dstr ds, cstr pattern) {
  int pattern_length = strlen(pattern);
  if (ds.length - 1 < pattern_length) {
    return false;
  }
  return strncmp(ds.items + ds.length - pattern_length - 1, pattern,
                 pattern_length) == 0;
}
bool dstr_starts_with(dstr ds, cstr pattern) {
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
void dstr_append_buf(dstr *ds, char *buf, int buf_length) {
  dstr_reserve_append(ds, buf_length);
  memcpy(ds->items + ds->length - 1, buf, buf_length);
  ds->length += buf_length;
  ds->items[ds->length - 1] = '\0';
}
void dstr_append_cstr(dstr *ds, cstr str) {
  dstr_append_buf(ds, str, strlen(str));
}
void dstr_append_vstr(dstr *ds, vstr vs) {
  dstr_append_buf(ds, vs.items, vs.length);
}
void dstr_append_dstr(dstr *ds, dstr ds2) {
  dstr_append_buf(ds, ds2.items, ds2.length - 1);
}
bool dstr_append_file(dstr *ds, cstr path) {
  FILE *file = fopen(path, "rb");
  defer(if(file) fclose(file));

  int m = flength(file);
  if (m < 0) {
    return false;
  }
  dstr_reserve_append(ds, m);
  fread(ds->items + ds->length - 1, m, 1, file);
  if (ferror(file)) {
    return false;
  }
  ds->length += m;
  ds->items[ds->length - 1] = '\0';
  return true;
}

bool cstr_from_file(cstr_o *str, cstr path) {
  dstr_o ds = {};
  if (dstr_from_file(&ds, path) == false) {
    return false;
  }
  *str = dstr_to_cstr(ds);
  da_free(&ds);
  return true;
}
bool vstr_from_file(vstr_o *vs, cstr path) {
  dstr_o ds = {};
  if (dstr_from_file(&ds, path) == false) {
    return false;
  }
  vs->items = malloc_copy(ds.length-1, ds.items);
  vs->length = ds.length-1;
  da_free(&ds);
  return true;
}
bool dstr_from_file(dstr_o *ds, cstr path) {
  return dstr_append_file(ds, path);
}

bool streql(int len1, char mem1[static len1], int len2, char mem2[static len2]){
  if(len1 != len2){
    return false;  
  }
  for(;mem1[len1-1] == mem2[len1-1] && len1 > 0; len1--){}
  return len1 == 0;
}
bool cstr_eql_cstr(cstr str1, cstr str2) {
  return streql(strlen(str1), str1, strlen(str2), str2);
}
bool vstr_eql_vstr(vstr vs1, vstr vs2) {
  return streql(vs1.length, vs1.items, vs2.length, vs2.items);
}
bool dstr_eql_dstr(dstr ds1, dstr ds2) {
  return streql(ds1.length-1, ds1.items, ds2.length-1, ds2.items);
}
bool cstr_eql_vstr(cstr str, vstr vs) {
  return streql(strlen(str), str, vs.length, vs.items);
}
bool cstr_eql_dstr(cstr str, dstr ds) {
  return streql(strlen(str), str, ds.length-1, ds.items);
}
bool vstr_eql_cstr(vstr vs, cstr str) { return cstr_eql_vstr(str, vs); }
bool vstr_eql_dstr(vstr vs, dstr ds) {
  return streql(vs.length, vs.items, ds.length-1, ds.items);
}
bool dstr_eql_cstr(dstr ds, cstr str) { return cstr_eql_dstr(str, ds); }
bool dstr_eql_vstr(dstr ds, vstr vs) { return vstr_eql_dstr(vs, ds); }

int cstr_char_index(cstr str, char c) {
  char *ch = strchr(str, c);
  return ch == NULL ? -1 : ch - str;
}
int cstr_char_index_rev(cstr str, char c) {
  char *ch = strrchr(str, c);
  return ch == NULL ? -1 : ch - str;
}
int cstr_first_char_index(cstr str, int count, char chars[count]) {
  char* min = str + len(str);
  for(int i = 0; i < count; i++){
    char *ch = strchr(str, chars[i]);
    if(ch && ch < min) min = ch; 
  }
  return min == str + len(str) ? -1 : min - str;
}
int cstr_first_char_index_rev(cstr str, int count, char chars[count]) {
  char* max = nullptr;
  for(int i = 0; i < count; i++){
    char *ch = strrchr(str, chars[i]);
    if(ch > max) max = ch; 
  }
  return max == NULL ? -1 : max - str;
}
int cstr_word_index(cstr str, cstr word) {
  char *ch = strstr(str, word);
  return ch == NULL ? -1 : ch - str;
}
int cstr_word_index_rev(cstr str, cstr word) {
  vstr temp = slice(str, 0, -1);
  int index = -1;
  while((index = vstr_char_index_rev(temp, word[0])) != -1){
    if(starts_with(offset(temp, index), word)) return index;
    temp = slice(temp, 0, index+len(word)-1);
  }
  return -1;
}

#define _first_word_index(str, count, words)                                   \
  vstr name = slice(str, 0, -1);                                               \
  cstr_o heads = calloc(count+1, 1);                                           \
  int_da lens = {};                                                            \
                                                                               \
  for (int i = 0; i < count; i++) {                                            \
    heads[i] = words[i][0];                                                    \
    da_append(&lens, strlen(words[i]));                                        \
  }                                                                            \
                                                                               \
  int index = -1;                                                              \
  while ((index = first_char_index(name, count, heads)) != -1) {               \
    name = offset(name, index);                                                \
    for (int i = 0; i < count; i++) {                                          \
      if (starts_with(name, words[i])) {                                       \
        free(heads);                                                           \
        da_free(&lens);                                                        \
        return name.items - items(str);                                        \
      }                                                                        \
    }                                                                          \
    name = offset(name, 1);                                                    \
  }                                                                            \
  free(heads);                                                                 \
  da_free(&lens);                                                              \
  return -1;

#define _first_word_index_rev(str, count, words)                               \
  cstr_o heads = calloc(count+1, 1);                                           \
  int_da lens = {};                                                            \
                                                                               \
  int max_len = 0;                                                             \
  for (int i = 0; i < count; i++) {                                            \
    heads[i] = words[i][0];                                                    \
    int len = strlen(words[i]);                                                \
    da_append(&lens, len);                                                     \
    if(len > max_len) max_len = len;                                           \
  }                                                                            \
                                                                               \
  vstr temp = slice(str, 0, -1);                                               \
  int index = -1;                                                              \
  while((index = vstr_first_char_index_rev(temp, count, heads)) != -1){        \
    for (int i = 0; i < count; i++) {                                          \
      if(starts_with(offset(temp, index), words[i])) return index;             \
    }                                                                          \
    temp = slice(temp, 0, index+max_len-1);                                    \
  }                                                                            \
  free(heads);                                                                 \
  da_free(&lens);                                                              \
  return -1;

int cstr_first_word_index(cstr str, int count, cstr words[count]) {
  _first_word_index(str, count, words);
}
int cstr_first_word_index_rev(cstr str, int count, cstr words[count]) {
  _first_word_index_rev(str, count, words);
}

int vstr_char_index(vstr vs, char c) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = 0;
  for (; (uintptr_t)(vs.items + i) % sizeof(size_t); i++) {
    if (i >= vs.length)      return -1;
    if (vs.items[i] == c)    return  i;
  }

  size_t pattern = ONES * (unsigned char)c;
  size_t *word_ptr = (size_t *)(vs.items + i);

  for (; i + sizeof(size_t) < vs.length; i += sizeof(size_t), word_ptr++) {
    if (HASZERO(*word_ptr ^ pattern))  break;
  }

  for (; i < vs.length; i++) {
    if (vs.items[i] == c)  return i;
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}
int vstr_char_index_rev(vstr vs, char c) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = vs.length-1;
  do{
    if (i < 0)             return -1;
    if (vs.items[i] == c)  return  i;
    i--;
  }while((uintptr_t)(vs.items + i) % sizeof(size_t) != sizeof(size_t)-1);
  i++;

  size_t pattern = ONES * (unsigned char)c;
  size_t *word_ptr = (size_t *)(vs.items + i)-1;

  for (; i >= sizeof(size_t); i -= sizeof(size_t), word_ptr--) {
    if (HASZERO(*word_ptr ^ pattern))  break;
  }

  for (; i >= 0; i--) {
    if (vs.items[i] == c)  return i;
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}


int vstr_first_char_index(vstr vs, int count, char chars[count]) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = 0;

  for (; (uintptr_t)(vs.items + i) % sizeof(size_t); i++) {
    if (i >= vs.length) return -1;
    if (vstr_char_index(slice(chars, 0, count), vs.items[i]) != -1) return i;
  }

  size_t *word_ptr = (size_t *)(vs.items + i);

  for (; i + sizeof(size_t) < vs.length; i += sizeof(size_t), word_ptr++) {
    for (int j = 0; j < count; j++) {
      if (HASZERO(*word_ptr ^ ONES * (unsigned char)chars[j])) {
        goto seek_single;
      }
    }
  }
seek_single:
  for (; i < vs.length; i++) {
    if (vstr_char_index(slice(chars, 0, count), vs.items[i]) != -1) return i;
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int vstr_first_char_index_rev(vstr vs, int count, char chars[count]) {
#define ONES ((size_t)-1 / UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX / 2 + 1))
#define HASZERO(x) ((x) - ONES & ~(x) & HIGHS)
  int i = vs.length-1;

  do{
    if (i < 0)             return -1;
    if (vstr_char_index(slice(chars, 0, count), vs.items[i]) != -1) return i;
    i--;
  }while((uintptr_t)(vs.items + i) % sizeof(size_t) != sizeof(size_t)-1);
  i++;

  size_t *word_ptr = (size_t *)(vs.items + i)-1;

  for (; i >= sizeof(size_t); i -= sizeof(size_t), word_ptr--) {
    for (int j = 0; j < count; j++) {
      if (HASZERO(*word_ptr ^ ONES * (unsigned char)chars[j])) {
        goto seek_single;
      }
    }
  }
seek_single:
  for (; i >= 0; i--) {
    if (vstr_char_index(slice(chars, 0, count), vs.items[i]) != -1) return i;
  }
  return -1;
#undef ONES
#undef HIGHS
#undef HASZERO
}

int vstr_word_index(vstr vs, cstr word) {
  int word_len = strlen(word);
  vstr temp = vs;
  int index = -1;
  while((index = char_index(temp, word[0])) != -1){
    temp = slice(temp, index, -1);
    if(starts_with(temp, word))
      return temp.items - vs.items;
    else 
      temp = slice(temp, word_len, -1);
  }
  return -1;
}
int vstr_word_index_rev(vstr vs, cstr word) {
  vstr temp = vs;
  int index = -1;
  while((index = char_index_rev(temp, word[0])) != -1){
    if(starts_with(slice(temp, index, -1), word))
      return temp.items - vs.items;
    temp = slice(temp, 0, index + len(word)-1);
  }
  return -1;
}

int vstr_first_word_index(vstr vs, int count, cstr words[count]) {
  _first_word_index(vs, count, words);  
}
int vstr_first_word_index_rev(vstr vs, int count, cstr words[count]) {
  _first_word_index_rev(vs, count, words);  
}

int dstr_char_index(dstr ds, char c) {
  return ds.length == 0 ? -1 : char_index(ds.items, c);
}
int dstr_char_index_rev(dstr ds, char c) {
  return ds.length == 0 ? -1 : char_index_rev(ds.items, c);
}

int dstr_first_char_index(dstr ds, int count, char chars[count]) {
  return ds.length == 0 ? -1 : first_char_index(ds.items, count, chars);
}
int dstr_first_char_index_rev(dstr ds, int count, char chars[count]) {
  return ds.length == 0 ? -1 : first_char_index_rev(ds.items, count, chars);
}

int dstr_word_index(dstr ds, cstr word) {
  return ds.length == 0 ? -1 : word_index(ds.items, word);
}
int dstr_word_index_rev(dstr ds, cstr word) {
  return ds.length == 0 ? -1 : word_index_rev(ds.items, word);
}

int dstr_first_word_index(dstr ds, int count, cstr words[count]) {
  return ds.length == 0 ? -1 : first_word_index(ds.items, count, words);
}
int dstr_first_word_index_rev(dstr ds, int count, cstr words[count]) {
  return ds.length == 0 ? -1 : first_word_index_rev(ds.items, count, words);
}

#define _index_by_char(str, c)                                                 \
  int_da indices = {};                                                         \
  vstr temp = slice(str, 0, -1);                                               \
  int index = -1;                                                              \
  while ((index = char_index(temp, c)) != -1) {                                \
    da_append(&indices, temp.items + index - items(str));                              \
    temp = slice(temp, index + 1, -1);                                         \
  }                                                                            \
  return indices;

#define _index_by_chars(str, count, chars)                                     \
  int_da indices = {};                                                         \
  vstr temp = slice(str, 0, -1);                                               \
  int index = -1;                                                              \
  while ((index = first_char_index(temp, count, chars)) != -1) {               \
    da_append(&indices, temp.items + index - items(str));                              \
    temp = slice(temp, index + 1, -1);                                         \
  }                                                                            \
  return indices;

#define _index_by_word(str, word)                                              \
  int_da indices = {};                                                         \
  vstr temp = slice(str, 0, -1);                                               \
  int index = -1;                                                              \
  while ((index = word_index(temp, word)) != -1) {                             \
    da_append(&indices, temp.items + index - items(str));                              \
    temp = slice(temp, index + 1, -1);                                         \
  }                                                                            \
  return indices;

#define _index_by_words(str, count, words)                                     \
  int_da indices = {};                                                         \
  vstr temp = slice(str, 0, -1);                                               \
  int index = -1;                                                              \
  while ((index = first_word_index(temp, count, words)) != -1) {               \
    da_append(&indices, temp.items + index - items(str));                              \
    temp = slice(temp, index + 1, -1);                                         \
  }                                                                            \
  return indices;

int_da cstr_index_char(cstr str, char c) {
  _index_by_char(str, c);
}
int_da vstr_index_char(vstr vs, char c) {
  _index_by_char(vs, c);
}
int_da dstr_index_char(dstr ds, char c) {
  _index_by_char(ds, c);
}

int_da cstr_index_chars(cstr str, int count, char chars[count]) {
  _index_by_chars(str, count, chars);
}
int_da vstr_index_chars(vstr vs, int count, char chars[count]) {
  _index_by_chars(vs, count, chars);
}
int_da dstr_index_chars(dstr ds, int count, char chars[count]) {
  _index_by_chars(ds, count, chars);
}

int_da cstr_index_word(cstr str, cstr word) {
  _index_by_word(str, word);
}
int_da vstr_index_word(vstr vs, cstr word) {
  _index_by_word(vs, word);
}
int_da dstr_index_word(dstr ds, cstr word) {
  _index_by_word(ds, word);
}

int_da cstr_index_words(cstr str, int count, cstr words[count]) {
  _index_by_words(str, count, words);
}
int_da vstr_index_words(vstr vs, int count, cstr words[count]) {
  _index_by_words(vs, count, words);
}
int_da dstr_index_words(dstr ds, int count, cstr words[count]) {
  _index_by_words(ds, count, words);
}

#define _da_append_split(splits, split, sso)                                   \
  if(sso & SSO_TRIM_ENTRIES)                                                   \
    split = trim(split);                                                       \
  if ((split.length < 1 && (sso & SSO_REMOVE_EMPTY)) == false) {               \
    da_append(splits, split);                                                  \
  }

#define _split_by_char(str, c, sso)                                            \
  vstr_da splits = {};                                                         \
  int index = -1;                                                              \
  vstr split = {};                                                             \
  vstr name = slice(str, 0, -1);                                               \
  do{                                                                          \
    index = char_index(name, c);                                               \
    split = slice(name, 0, index);                                             \
    _da_append_split(&splits, split, sso);                                     \
    name = offset(name, index + 1);                                            \
  }while(index != -1);                                                         \
  return splits;

#define _split_by_chars(str, count, chars, sso)                                \
  vstr_da splits = {};                                                         \
  int index = -1;                                                              \
  vstr split = {};                                                             \
  vstr name = slice(str, 0, -1);                                               \
  do{                                                                          \
    index = first_char_index(name, count, chars);                              \
    split = slice(name, 0, index);                                             \
    _da_append_split(&splits, split, sso);                                     \
    name = offset(name, index + 1);                                            \
  }while(index != -1);                                                         \
  return splits;

#define _split_by_word(str, word, sso)                                         \
  vstr_da splits = {};                                                         \
  int index = -1;                                                              \
  vstr split = {};                                                             \
  vstr name = slice(str, 0, -1);                                               \
  do{                                                                          \
    index = word_index(name, word);                                            \
    split = slice(name, 0, index);                                             \
    _da_append_split(&splits, split, sso);                                     \
    name = offset(name, index + len(word));                                    \
  }while(index != -1);                                                         \
  return splits;

#define _split_by_words(str, count, words, sso)                                \
  vstr_da splits = {};                                                         \
  int index = -1;                                                              \
  vstr split = {};                                                             \
  vstr name = slice(str, 0, -1);                                               \
  do{                                                                          \
    index = first_word_index(name, count, words);                              \
    split = slice(name, 0, index);                                             \
    _da_append_split(&splits, split, sso);                                     \
    for(int i = 0; i < count; i++){                                            \
      if(eql(slice(name, 0, len(words[i])), words[i])){                        \
        name = offset(name, index + len(words[i]));                            \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }while(index != -1);                                                         \
  return splits;

//TODO: Maybe make indexing chars and words optionally return additional bit of 
//info: what symbol was indexed at index. This will provide information about 
//the length of a word, what the word/char was. This might be really helpful in 
//some situations

//TODO: Add functions for splitting strings by vstr-ings maybe?

vstr_da cstr_split_by_char(cstr str, char c, StrSplitOptions sso) {
  _split_by_char(str, c, sso);
}
vstr_da vstr_split_by_char(vstr vs, char c, StrSplitOptions sso) {
  _split_by_char(vs, c, sso);
}
vstr_da dstr_split_by_char(dstr ds, char c, StrSplitOptions sso) {
  _split_by_char(ds.items, c, sso);
}

vstr_da cstr_split_by_chars(cstr str, int count, char chars[count], StrSplitOptions sso) {
  _split_by_chars(str, count, chars, sso);
}
vstr_da vstr_split_by_chars(vstr vs, int count, char chars[count], StrSplitOptions sso) {
  _split_by_chars(vs, count, chars, sso);
}
vstr_da dstr_split_by_chars(dstr ds, int count, char chars[count], StrSplitOptions sso) {
  _split_by_chars(ds.items, count, chars, sso);
}

vstr_da cstr_split_by_word(cstr str, cstr word, StrSplitOptions sso) {
  _split_by_word(str, word, sso);
}
vstr_da vstr_split_by_word(vstr vs, cstr word, StrSplitOptions sso) {
  _split_by_word(vs, word, sso);
}
vstr_da dstr_split_by_word(dstr ds, cstr word, StrSplitOptions sso) {
  _split_by_word(ds.items, word, sso);
}

vstr_da cstr_split_by_words(cstr str, int count, cstr words[count], StrSplitOptions sso) {
  _split_by_words(str, count, words, sso);
}
vstr_da vstr_split_by_words(vstr vs, int count, cstr words[count], StrSplitOptions sso) {
  _split_by_words(vs, count, words, sso);
}
vstr_da dstr_split_by_words(dstr ds, int count, cstr words[count], StrSplitOptions sso) {
  _split_by_words(ds.items, count, words, sso);
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
  va_start(vl);
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
  va_list copy;
  va_copy(copy, va);
  int expected = vsnprintf(ds.items, ds.capacity, format, va);
  if(expected < 0){
    printf("[vsprintf] Error occured while formatting %.*s\n", strlen(format), format);
    exit(1);
  }
  da_reserve(&ds, expected+1);

  vsnprintf(ds.items, ds.capacity, format, copy);
  ds.items[expected] = '\0';
  ds.length = expected+1;
  va_end(copy);

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
vstr cstr_capture_by_word(cstr str, int offset, cstr inc,
                                cstr dec) {
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
vstr vstr_capture_by_word(vstr vs, int offset, cstr inc,
                                cstr dec) {
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
vstr dstr_capture_by_word(dstr ds, int offset, cstr inc,
                                cstr dec) {
  return ds.items == 0 ? (vstr){}
                       : capture_by_word(ds.items, offset, inc, dec);
}

#define _count_char_word(name, delim_type, delim)                              \
  int count = 0;                                                               \
  for (int index = 0;                                                          \
       (index = delim_type##_index(name, delim)) != -1; count++) {             \
    name = offset(name, index + 1);                                            \
  }                                                                            \
  return count;

int cstr_count_char(cstr str, char c) { 
  _count_char_word(str, char, c); 
}
int cstr_count_word(cstr str, cstr word) {
  _count_char_word(str, word, word);
}
int vstr_count_char(vstr vs, char c) { 
  _count_char_word(vs, char, c); 
}
int vstr_count_word(vstr vs, cstr word) {
  _count_char_word(vs, word, word);
}
int dstr_count_char(dstr ds, char c) {
  return ds.length == 0 ? 0 : count_char(ds.items, c);
}
int dstr_count_word(dstr ds, cstr word) {
  return ds.length == 0 ? 0 : count_word(ds.items, word);
}

vstr cstr_trim_left(cstr str) {
  int len = strlen(str);
  int start_index = 0;
  for (; start_index < len && isspace(str[start_index]); start_index++) {}
  return offset(slice(str, 0, -1), start_index);   
}
vstr vstr_trim_left(vstr str) {
  int start_index = 0;
  for (; start_index < str.length && isspace(str.items[start_index]); start_index++) {}
  return offset(str, start_index);   
}
vstr dstr_trim_left(dstr str) {
  int start_index = 0;
  for (; start_index < str.length-1 && isspace(str.items[start_index]); start_index++) {}
  return offset(slice(str, 0, -1), start_index);   
}

vstr cstr_trim_right(cstr str) {
  int len = strlen(str);
  int end_index = len-1;
  for (; end_index > 0 && isspace(str[end_index]); end_index--) {}
  return slice(str, 0, end_index+1);
}
vstr vstr_trim_right(vstr str) {
  int end_index = str.length-1;
  for (; end_index > 0 && isspace(str.items[end_index]); end_index--) {}
  return slice(str.items, 0, end_index+1);
}
vstr dstr_trim_right(dstr str) {
  int end_index = str.length-2;
  for (; end_index > 0 && isspace(str.items[end_index]); end_index--) {}
  return slice(str.items, 0, end_index+1);
}

vstr cstr_trim(cstr str) {
  return vstr_trim_left(cstr_trim_right(str));
}
vstr vstr_trim(vstr vs) {
  return vstr_trim_left(vstr_trim_right(vs));
}
vstr dstr_trim(dstr ds) {
  return vstr_trim_left(dstr_trim_right(ds));
}

bool cstr_is_int(cstr str){
  return vstr_is_int((vstr){.items = str, .length = strlen(str)}); 
}
bool dstr_is_int(dstr ds){
  return vstr_is_int((vstr){.items = ds.items, .length = ds.length-1}); 
}

bool vstr_is_int(vstr str){
  for(int i = 0; i < str.length; i++){
    char c = str.items[i];
    if(c == '+' || c == '-'){
      if(i!=0){
        return false;
      }
    }else if(isdigit(c) == false){
      return false;
    }
  }
  return true;
}

typedef enum {
  FFMT_UNDEFINED, 
  FFMT_DECIMAL_PRE, 
  FFMT_DECIMAL_MESO, 
  FFMT_DECIMAL_POST,
  FFMT_HEX_PRE, 
  FFMT_HEX_MESO, 
  FFMT_HEX_POST, 
  FFMT_INF, 
  FFMT_NAN
}FloatFmts;

bool cstr_is_float(cstr str){
  return vstr_is_float((vstr){.items = str, .length = strlen(str)}); 
}
bool dstr_is_float(dstr ds){
  return vstr_is_float((vstr){.items = ds.items, .length = ds.length-1}); 
}

bool vstr_is_float(vstr str){
  char prev = '\0';
  FloatFmts ffmt = FFMT_UNDEFINED;
  
  for(int i = 0; i < str.length; i++){
    char c = tolower(str.items[i]);
    switch(ffmt){
      case FFMT_UNDEFINED:{
        if(c == '+' || c == '-'){
          if(prev == '\0'){
            break; 
          }else{
            return false;
          }
        }else if(c == '0'){
          if(isdigit(prev)){
            ffmt = FFMT_DECIMAL_PRE;
          }
          break;
        }else if(c == 'x'){
          if(prev == '0'){
            ffmt = FFMT_HEX_PRE;
            break;
          }else{
            return false;
          }
        }else if(c == 'i'){
          if(prev == '+' || prev == '-' || prev == '\0'){
            ffmt = FFMT_INF;
            break;
          }else{
            return false;            
          }
        }else if(c == 'n'){
          if(prev == '+' || prev == '-' || prev == '\0'){
            ffmt = FFMT_NAN;
            break;
          }else{
            return false;            
          }
        }
      }break;
      case FFMT_DECIMAL_PRE:{
        if(c == '.'){
          ffmt = FFMT_DECIMAL_MESO;
          break;
        }
        if(c == 'e'){
          ffmt = FFMT_DECIMAL_POST;
          break;
        }
        if(isdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_DECIMAL_MESO:{
        if(c == 'e'){
          ffmt = FFMT_DECIMAL_POST;
          break;
        }
        if(isdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_DECIMAL_POST:{
        if((c == '+' || c == '-') && prev == 'e'){
          break;
        }
        if(isdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_HEX_PRE:{
        if(c == '.'){
          ffmt = FFMT_HEX_MESO;
          break;
        }
        if(c == 'p'){
          ffmt = FFMT_HEX_POST;
          break;
        }
        if(isxdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_HEX_MESO:{
        if(c == 'p'){
          ffmt = FFMT_HEX_POST;
          break;
        }
        if(isxdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_HEX_POST:{
        if((c == '+' || c == '-') && prev == 'p'){
          break;
        }
        if(isxdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_INF:{
        bool has_sign = (str.items[0] == '+' || str.items[0] == '-'); 
        int efflen = str.length - (has_sign ? 1 : 0); 
        if(efflen == 3){
          char* short_inf = "inf";
          char* curstr = str.items + (has_sign ? 1 : 0);
          for(; *short_inf == tolower(*curstr) && curstr < str.items + str.length; curstr++, short_inf++);
          if(curstr == str.items + str.length){
            return true;
          }
        }else if(efflen == 8){
          char* short_inf = "infinity";
          char* curstr = str.items + (has_sign ? 1 : 0);
          for(; *short_inf == tolower(*curstr) && curstr < str.items + str.length; curstr++, short_inf++);
          if(curstr == str.items + str.length){
            return true;
          }
        }
        return false;            
      }break;
      case FFMT_NAN:{
        bool has_sign = (str.items[0] == '+' || str.items[0] == '-'); 
        int efflen = str.length - (has_sign ? 1 : 0); 
        if(efflen < 3){
          return false;            
        }
        char* short_inf = "nan";
        char* curstr = str.items + (has_sign ? 1 : 0);
        for(; *short_inf == tolower(*curstr) && curstr < str.items + str.length; curstr++, short_inf++);
        if(curstr == str.items + (has_sign ? 1 : 0) + 3){
          if(efflen == 3){
            return true;
          }
          for(int j = (has_sign ? 1 : 0) + 3; j < str.length; j++){
            c = tolower(str.items[j]);
            if(isalnum(c) == false && c == '_' == false){
              return false;            
            }
          }
          return true;
        }
        return false;            
      }break;
    }  
    prev = c;
  }
  if(ffmt == FFMT_DECIMAL_POST && prev == 'c'){
    return false;            
  }
  if(ffmt == FFMT_HEX_POST && prev == 'p'){
    return false;            
  }

  return true;
}
