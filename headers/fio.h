#pragma once
#include "types.h"
#include <stdio.h>

#ifndef _WIN32
#include <dirent.h>
#include <sys/types.h>
#else
//TODO: Add windows support
#endif

#define path_join(...) path_join_many(NULL, __VA_ARGS__, NULL)
cstr_o path_join_many(void *nil, ...);

bool dir_open(cstr path, Dir *dir);

bool fexists(const cstr path);
int flength(FILE *file);

void fwrite_vstr(vstr sv, FILE *file);
void fwrite_dstr(dstr sb, FILE *file);
void fwrite_cstr(const cstr str, FILE *file);

cstr_o fread_cstr(FILE *file);

#define fwrite_val(value, file) fwrite(value, sizeof(*value), 1, file)
#define fread_val(value, file) fread(value, sizeof(*value), 1, file)
