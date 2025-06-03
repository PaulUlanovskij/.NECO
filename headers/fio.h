#pragma once
#include "types.h"
#include <stdio.h>

#ifndef _WIN32
#include <dirent.h>
#include <sys/types.h>
#else

#endif

typedef struct {
  ino_t ino;
  cstr name;
  unsigned short reclen;
  unsigned char type;
} DirEntry;

typedef struct {
  DirEntry *items;
  int capacity;
  int length;
} DirEntry_da;

DirEntry_da dir_get_all_items(DIR *dir);

bool fexists(const cstr path);
int flength(FILE *file);

void fwrite_sv(StringView sv, FILE *file);
void fwrite_sb(StringBuilder sb, FILE *file);
void fwrite_cstr(const cstr str, FILE *file);

cstr_o fread_cstr(FILE *file);

#define fwrite_val(value, file) fwrite(value, sizeof(*value), 1, file)
#define fread_val(value, file) fread(value, sizeof(*value), 1, file)
