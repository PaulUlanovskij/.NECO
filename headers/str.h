#pragma once
#include <stdarg.h>
#include "da.h"

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

cstr cstr_offset(const cstr str, int offset);
vstr vstr_offset(vstr vs, int offset);
// dstr_offset is absent deliberately as it would go against what dstr
// promises to user

vstr cstr_slice(const cstr str, int begin, int end);
vstr vstr_slice(vstr vs, int begin, int end);
vstr dstr_slice(dstr ds, int begin, int end);

cstr_o cstr_copy(const cstr str);
vstr_o vstr_copy(vstr vs);
dstr_o dstr_copy(dstr ds);

vstr_o cstr_to_vstr(const cstr str);
dstr_o cstr_to_dstr(const cstr str);
dstr_o vstr_to_dstr(vstr vs);
cstr_o vstr_to_cstr(vstr vs);
cstr_o dstr_to_cstr(dstr ds);
vstr_o dstr_to_vstr(dstr ds);

bool cstr_ends_with(const cstr str, const cstr pattern);
bool cstr_starts_with(const cstr str, const cstr pattern);
bool vstr_ends_with(vstr vs, const cstr pattern);
bool vstr_starts_with(vstr vs, const cstr pattern);
bool dstr_ends_with(dstr ds, const cstr pattern);
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

bool cstr_cmp_vstr(cstr str, vstr vs);
bool cstr_cmp_dstr(cstr str, dstr ds);
bool vstr_cmp_cstr(vstr vs, cstr str);
bool vstr_cmp_dstr(vstr vs, dstr ds);
bool dstr_cmp_cstr(dstr ds, cstr str);
bool dstr_cmp_vstr(dstr ds, vstr vs);

int cstr_char_index(cstr str, char c);
int cstr_first_char_index(cstr str, const char *chars);
int cstr_word_index(cstr str, const cstr word);
int cstr_first_word_index(cstr str, const cstr const *words, int count);
int vstr_char_index(vstr vs, char c);
int vstr_first_char_index(vstr vs, const char *chars);
int vstr_word_index(vstr vs, const cstr word);
int vstr_first_word_index(vstr vs, const cstr const *words, int count);

int dstr_char_index(dstr ds, char c);
int dstr_first_char_index(dstr ds, const char *chars);
int dstr_word_index(dstr ds, const cstr word);
int dstr_first_word_index(dstr ds, const cstr const *words, int count);

int_da cstr_index_char(const cstr str, char c);
int_da cstr_index_chars(const cstr str, const char *chars);
int_da vstr_index_char(vstr vs, char c);
int_da vstr_index_chars(vstr vs, const char *chars);
int_da dstr_index_char(dstr ds, char c);
int_da dstr_index_chars(dstr ds, const char *chars);

int_da cstr_index_word(const cstr str, const cstr word);
int_da cstr_index_words(const cstr str, const cstr const *words, int count);
int_da vstr_index_word(vstr vs, const cstr word);
int_da vstr_index_words(vstr vs, const cstr const *words, int count);
int_da dstr_index_word(dstr ds, const cstr word);
int_da dstr_index_words(dstr ds, const cstr const *words, int count);

vstr_da cstr_split_by_char(const cstr str, char c);
vstr_da cstr_split_by_chars(const cstr str, const char *chars);
vstr_da vstr_split_by_char(vstr vs, char c);
vstr_da vstr_split_by_chars(vstr vs, const char *chars);
vstr_da dstr_split_by_char(dstr ds, char c);
vstr_da dstr_split_by_chars(dstr ds, const char *chars);

vstr_da cstr_split_by_word(const cstr str, const cstr word);
vstr_da cstr_split_by_words(const cstr str, const cstr const *words, int count);
vstr_da vstr_split_by_word(vstr vs, const cstr word);
vstr_da vstr_split_by_words(vstr vs, const cstr const *words, int count);
vstr_da dstr_split_by_word(dstr ds, const cstr word);
vstr_da dstr_split_by_words(dstr ds, const cstr const *words, int count);

void dstr_reserve_append(dstr *ds, int capacity);
void dstr_rewind(dstr *ds);
void dstr_appendf(dstr *ds, cstr format, ...);

cstr_o cstr_quote(cstr str);
cstr_o cstr_sprintf(cstr format, ...);
cstr_o cstr_vsprintf(cstr format, va_list va);
cstr_o cstr_concat_many(void *nil, ...);
#define cstr_concat(...) cstr_concat_many(NULL, __VA_ARGS__, NULL)

vstr cstr_capture_block_by_char(cstr str, int offset, char inc, char dec);
vstr cstr_capture_block_by_word(cstr str, int offset, const cstr inc, const cstr dec);
vstr vstr_capture_block_by_char(vstr vs, int offset, char inc, char dec);
vstr vstr_capture_block_by_word(vstr vs, int offset, const cstr inc, const cstr dec);
vstr dstr_capture_block_by_char(dstr ds, int offset, char inc, char dec);
vstr dstr_capture_block_by_word(dstr ds, int offset, const cstr inc, const cstr dec);

int cstr_count_char(cstr str, char c);
int cstr_count_word(cstr str, const cstr word);
int vstr_count_char(vstr vs, char c);
int vstr_count_word(vstr vs, const cstr word);
int dstr_count_char(dstr ds, char c);
int dstr_count_word(dstr ds, const cstr word);

cstr_o cstr_trim(const cstr str);
vstr cstr_trim_to_vstr(const cstr str);
vstr vstr_trim(vstr vs);
vstr dstr_trim_to_vstr(dstr ds);
