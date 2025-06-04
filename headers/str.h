#pragma once
#include "types.h"

int_da sv_index_char(SV sv, char c);
int_da sv_index_chars(SV sv, char *chars);

int_da cstr_index_char(const cstr str, char c);
int_da cstr_index_chars(const cstr str, char *chars);

SV_da cstr_split_by_char(const cstr str, char delim);
SV_da cstr_split_by_chars(const cstr str, char *delim);

cstr_o cstr_from_sb(SB *sb);

cstr_o cstr_copy(const cstr str);

cstr_o cstr_concat_many(void *nil, ...);
#define cstr_concat(...) cstr_concat_many(NULL, __VA_ARGS__, NULL)

bool cstr_ends_with(const cstr haystack, const cstr needle);
bool cstr_starts_with(const cstr haystack, const cstr needle);

SV sv_from_cstr(const cstr str, int start, int end);
SV sv_from_sb(SB sb);
SB sb_from_sv(SV sv);

SV_o sv_copy(SV sv);
cstr_o cstr_quote_copy(cstr arg);
cstr_o sv_copy_to_cstr(SV sv);

SV sv_from(SV sv, int index);
SV sv_upto(SV sv, int index);

void sb_clear(SB *sb);

void sb_append(SB *sb, char c);
void sb_append_buf(SB *sb, const char *buf, int buf_length);
void sb_append_cstr(SB *sb, const cstr str);
void sb_append_sv(SB *sb, SV sv);
void sb_append_sb(SB *sb, SB sb2);

bool sv_from_file(SV *sv, const cstr path);
bool sb_append_file(SB *sb, const cstr path);

int sv_get_char_index(SV sv, char c);
int sv_get_first_char_index(SV sv, const cstr str);
int sv_get_word_index(SV sv, const cstr delim);
int sv_get_first_word_index(SV sv, const cstr const *delim, int count);
