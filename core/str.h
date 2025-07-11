#pragma once
#include "da.h"
#include <stdarg.h>

typedef char *cstr, *cstr_o;

typedef struct {
  char *items;
  int length;
} vstr, vstr_o;

typedef struct {
  char *items;
  int length;
  int capacity;
} dstr, dstr_o;

define_simple_da(vstr, vstr_da);
define_simple_da(cstr, cstr_da);

#define offset(x, offset) _Generic(x, \
  vstr:   vstr_offset, \
  cstr:   cstr_offset  \
)(x, offset)
cstr cstr_offset(const cstr str, int offset);
vstr vstr_offset(vstr vs, int offset);
// dstr_offset is absent deliberately as it would go against what dstr
// promises to user

#define slice(x, begin, end) _Generic(x, \
  vstr:   vstr_slice, \
  cstr:   cstr_slice, \
  dstr:   dstr_slice  \
)(x, begin, end)
vstr cstr_slice(const cstr str, int begin, int end);
vstr vstr_slice(vstr vs, int begin, int end);
vstr dstr_slice(dstr ds, int begin, int end);

#define copy(x) _Generic(x, \
  vstr:   vstr_copy, \
  cstr:   cstr_copy, \
  dstr:   dstr_copy  \
)(x)
cstr_o cstr_copy(const cstr str);
vstr_o vstr_copy(vstr vs);
dstr_o dstr_copy(dstr ds);

#define to_vstr(x) _Generic(x, \
  cstr:   cstr_to_vstr, \
  dstr:   dstr_to_vstr  \
)(x)
#define to_cstr(x) _Generic(x, \
  vstr:   vstr_to_cstr, \
  dstr:   dstr_to_cstr \
)(x)
#define to_dstr(x) _Generic(x, \
  vstr:   vstr_to_dstr, \
  cstr:   cstr_to_dstr \
)(x)

vstr_o cstr_to_vstr(const cstr str);
dstr_o cstr_to_dstr(const cstr str);
dstr_o vstr_to_dstr(vstr vs);
cstr_o vstr_to_cstr(vstr vs);
cstr_o dstr_to_cstr(dstr ds);
vstr_o dstr_to_vstr(dstr ds);

#define ends_with(x, pattern) _Generic(x, \
  vstr:   vstr_ends_with, \
  cstr:   cstr_ends_with, \
  dstr:   dstr_ends_with  \
)(x, pattern)

bool cstr_ends_with(const cstr str, const cstr pattern);
bool vstr_ends_with(vstr vs, const cstr pattern);
bool dstr_ends_with(dstr ds, const cstr pattern);

#define starts_with(x, pattern) _Generic(x, \
  vstr:   vstr_starts_with, \
  cstr:   cstr_starts_with, \
  dstr:   dstr_starts_with  \
)(x, pattern)

bool cstr_starts_with(const cstr str, const cstr pattern);
bool vstr_starts_with(vstr vs, const cstr pattern);
bool dstr_starts_with(dstr ds, const cstr pattern);

void dstr_append(dstr *ds, char c);
void dstr_append_buf(dstr *ds, const char *buf, int buf_length);
void dstr_append_cstr(dstr *ds, const cstr str);
void dstr_append_vstr(dstr *ds, vstr vs);
void dstr_append_dstr(dstr *ds, dstr ds2);
bool dstr_append_file(dstr *ds, const cstr path);

bool cstr_from_file(cstr_o *str, const cstr path);
bool vstr_from_file(vstr_o *vs, const cstr path);
bool dstr_from_file(dstr_o *ds, const cstr path);

#define eql(x, y) _Generic(x, \
  cstr: _Generic(y,           \
      vstr: cstr_eql_vstr,    \
      cstr: cstr_eql_cstr,    \
      dstr: cstr_eql_dstr),   \
  vstr: _Generic(y,           \
      vstr: vstr_eql_vstr,    \
      cstr: vstr_eql_cstr,    \
      dstr: vstr_eql_dstr),   \
  dstr: _Generic(y,           \
      vstr: dstr_eql_vstr,    \
      cstr: dstr_eql_cstr,    \
      dstr: dstr_eql_dstr)    \
)(x, y)

bool cstr_eql_cstr(cstr str1, cstr str2);
bool vstr_eql_vstr(vstr vs1, vstr vs2);
bool dstr_eql_dstr(dstr ds1, dstr ds2);
bool cstr_eql_vstr(cstr str, vstr vs);
bool cstr_eql_dstr(cstr str, dstr ds);
bool vstr_eql_cstr(vstr vs, cstr str);
bool vstr_eql_dstr(vstr vs, dstr ds);
bool dstr_eql_cstr(dstr ds, cstr str);
bool dstr_eql_vstr(dstr ds, vstr vs);

#define char_index(x, c) _Generic(x, \
  vstr:   vstr_char_index, \
  cstr:   cstr_char_index, \
  dstr:   dstr_char_index  \
)(x, c)
int cstr_char_index(cstr str, char c);
int vstr_char_index(vstr vs, char c);
int dstr_char_index(dstr ds, char c);

#define first_char_index(x, count, chars) _Generic(x, \
  vstr:   vstr_first_char_index, \
  cstr:   cstr_first_char_index, \
  dstr:   dstr_first_char_index  \
)(x, count, chars)
int cstr_first_char_index(cstr str, int count, const char chars[static count]);
int vstr_first_char_index(vstr vs, int count, const char chars[static count]);
int dstr_first_char_index(dstr ds, int count, const char chars[static count]);

#define word_index(x, word) _Generic(x, \
  vstr:   vstr_word_index, \
  cstr:   cstr_word_index, \
  dstr:   dstr_word_index  \
)(x, word)
int cstr_word_index(cstr str, const cstr word);
int vstr_word_index(vstr vs, const cstr word);
int dstr_word_index(dstr ds, const cstr word);

#define first_word_index(x, count, words) _Generic(x, \
  vstr:   vstr_first_word_index, \
  cstr:   cstr_first_word_index, \
  dstr:   dstr_first_word_index  \
)(x, count, words)
int cstr_first_word_index(cstr str, int count, const cstr const words[static count]);
int vstr_first_word_index(vstr vs, int count, const cstr const words[static count]);
int dstr_first_word_index(dstr ds, int count, const cstr const words[static count]);

#define index_char(x, c) _Generic(x, \
  vstr:   vstr_index_char, \
  cstr:   cstr_index_char, \
  dstr:   dstr_index_char  \
)(x, c)
int_da cstr_index_char(const cstr str, char c);
int_da vstr_index_char(vstr vs, char c);
int_da dstr_index_char(dstr ds, char c);

#define index_chars(x, count, chars) _Generic(x, \
  vstr:   vstr_index_chars, \
  cstr:   cstr_index_chars, \
  dstr:   dstr_index_chars  \
)(x, count, chars)
int_da cstr_index_chars(const cstr str, int count, const char chars[static count]);
int_da vstr_index_chars(vstr vs, int count, const char chars[static count]);
int_da dstr_index_chars(dstr ds, int count, const char chars[static count]);

#define index_word(x, word) _Generic(x, \
  vstr:   vstr_index_word, \
  cstr:   cstr_index_word, \
  dstr:   dstr_index_word  \
)(x, word)
int_da cstr_index_word(const cstr str, const cstr word);
int_da vstr_index_word(vstr vs, const cstr word);
int_da dstr_index_word(dstr ds, const cstr word);

#define index_words(x, count, words) _Generic(x, \
  vstr:   vstr_index_words, \
  cstr:   cstr_index_words, \
  dstr:   dstr_index_words  \
)(x, count, words)
int_da cstr_index_words(const cstr str, int count, const cstr const words[static count]);
int_da vstr_index_words(vstr vs, int count, const cstr const words[static count]);
int_da dstr_index_words(dstr ds, int count, const cstr const words[static count]);

typedef enum StrSplitOptions:char{
  SSO_NONE = 0, SSO_REMOVE_EMPTY, SSO_TRIM_ENTRIES
}StrSplitOptions;
#define split_by_char(x, c, sso) _Generic(x, \
  vstr:   vstr_split_by_char, \
  cstr:   cstr_split_by_char, \
  dstr:   dstr_split_by_char  \
)(x, c, sso)


vstr_da cstr_split_by_char(const cstr str, char c, StrSplitOptions sso);
vstr_da vstr_split_by_char(vstr vs, char c, StrSplitOptions sso);
vstr_da dstr_split_by_char(dstr ds, char c, StrSplitOptions sso);

#define split_by_chars(x, count, chars, sso) _Generic(x, \
  vstr:   vstr_split_by_chars, \
  cstr:   cstr_split_by_chars, \
  dstr:   dstr_split_by_chars  \
)(x, count, chars, sso)
vstr_da cstr_split_by_chars(const cstr str, int count, const char chars[static count], StrSplitOptions sso);
vstr_da vstr_split_by_chars(vstr vs, int count, const char chars[static count], StrSplitOptions sso);
vstr_da dstr_split_by_chars(dstr ds, int count, const char chars[static count], StrSplitOptions sso);

#define split_by_word(x, word, sso) _Generic(x, \
  vstr:   vstr_split_by_word, \
  cstr:   cstr_split_by_word, \
  dstr:   dstr_split_by_word  \
)(x, word, sso)
vstr_da cstr_split_by_word(const cstr str, const cstr word, StrSplitOptions sso);
vstr_da vstr_split_by_word(vstr vs, const cstr word, StrSplitOptions sso);
vstr_da dstr_split_by_word(dstr ds, const cstr word, StrSplitOptions sso);

#define split_by_words(x, count, words, sso) _Generic(x, \
  vstr:   vstr_split_by_words, \
  cstr:   cstr_split_by_words, \
  dstr:   dstr_split_by_words  \
)(x, count, words, sso)
vstr_da cstr_split_by_words(const cstr str, int count, const cstr const words[static count], StrSplitOptions sso);
vstr_da vstr_split_by_words(vstr vs, int count, const cstr const words[static count], StrSplitOptions sso);
vstr_da dstr_split_by_words(dstr ds, int count, const cstr const words[static count], StrSplitOptions sso);

void dstr_reserve_append(dstr *ds, int capacity);
void dstr_rewind(dstr *ds);
void dstr_appendf(dstr *ds, cstr format, ...);

cstr_o cstr_quote(cstr str);
cstr_o cstr_sprintf(cstr format, ...);
cstr_o cstr_vsprintf(cstr format, va_list va);
cstr_o cstr_concat_many(...);
#define cstr_concat(...) cstr_concat_many(__VA_ARGS__, NULL)

#define capture_by_char(x, offset, inc, dec) _Generic(x, \
  vstr:   vstr_capture_by_char, \
  cstr:   cstr_capture_by_char, \
  dstr:   dstr_capture_by_char  \
)(x, offset, inc, dec)
vstr cstr_capture_by_char(cstr str, int offset, char inc, char dec);
vstr vstr_capture_by_char(vstr vs, int offset, char inc, char dec);
vstr dstr_capture_by_char(dstr ds, int offset, char inc, char dec);

#define capture_by_word(x, offset, inc, dec) _Generic(x, \
  vstr:   vstr_capture_by_word, \
  cstr:   cstr_capture_by_word, \
  dstr:   dstr_capture_by_word  \
)(x, offset, inc, dec)
vstr cstr_capture_by_word(cstr str, int offset, const cstr inc, const cstr dec);
vstr vstr_capture_by_word(vstr vs, int offset, const cstr inc, const cstr dec);
vstr dstr_capture_by_word(dstr ds, int offset, const cstr inc, const cstr dec);

#define count_char(x, c) _Generic(x, \
  vstr:   vstr_count_char, \
  cstr:   cstr_count_char, \
  dstr:   dstr_count_char  \
)(x, c)
int cstr_count_char(cstr str, char c);
int vstr_count_char(vstr vs, char c);
int dstr_count_char(dstr ds, char c);

#define count_word(x, word) _Generic(x, \
  vstr:   vstr_count_word, \
  cstr:   cstr_count_word, \
  dstr:   dstr_count_word  \
)(x, word)
int cstr_count_word(cstr str, const cstr word);
int vstr_count_word(vstr vs, const cstr word);
int dstr_count_word(dstr ds, const cstr word);

#define trim(x) _Generic(x, \
  vstr:   vstr_trim, \
  cstr:   cstr_trim, \
  dstr:   dstr_trim  \
)(x)
vstr cstr_trim(const cstr str);
vstr vstr_trim(vstr vs);
vstr dstr_trim(dstr ds);

#define is_int(x) _Generic(x, \
  vstr:   vstr_is_int, \
  cstr:   cstr_is_int, \
  dstr:   dstr_is_int  \
)(x)
bool cstr_is_int(cstr str);
bool vstr_is_int(vstr str);
bool dstr_is_int(dstr ds);

#define is_float(x) _Generic(x, \
  vstr:   vstr_is_float, \
  cstr:   cstr_is_float, \
  dstr:   dstr_is_float  \
)(x)
bool cstr_is_float(cstr str);
bool vstr_is_float(vstr str);
bool dstr_is_float(dstr ds);
