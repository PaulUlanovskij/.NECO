#pragma once
#include "types.h"

cstr_o cstr_from_sb(StringBuilder *sb);

cstr_o cstr_copy(const cstr str);

cstr_o cstr_concat_many(void *nil, ...);
#define cstr_concat(...) cstr_concat_many(NULL, __VA_ARGS__, NULL)

char cstr_ends_with(cstr haystack, cstr needle);
char cstr_starts_with(cstr haystack, cstr needle);

StringView sv_from_cstr(const cstr str, int start, int end);
StringView sv_from_sb(StringBuilder sb);
StringBuilder sb_from_sv(StringView sv);

StringView_o sv_copy(StringView sv);
cstr_o cstr_quote_copy(cstr arg);
cstr_o sv_copy_to_cstr(StringView sv);

StringView sv_from(StringView sv, int index);
StringView sv_upto(StringView sv, int index);

void sb_clear(StringBuilder *sb);

void sb_append(StringBuilder *sb, char c);
void sb_append_buf(StringBuilder *sb, const char *buf, int buf_length);
void sb_append_cstr(StringBuilder *sb, const cstr str);
void sb_append_sv(StringBuilder *sb, StringView sv);
void sb_append_sb(StringBuilder *sb, StringBuilder sb2);

bool sv_from_file(StringView *sv, const cstr path);
bool sb_append_file(StringBuilder *sb, const cstr path);

int sv_get_char_index(StringView sv, char c);
int sv_get_first_char_index(StringView sv, const cstr str);
int sv_get_word_index(StringView sv, const cstr delim);
int sv_get_first_word_index(StringView sv, const cstr const *delim, int count);
