#pragma once

#ifndef _WIN32
#include <dirent.h>
#include <sys/types.h>
#else
// TODO: Add windows support
#endif

typedef struct {
  void *base_ptr;
  void *alloc_ptr;
  int capacity;
} ArenaAllocator;

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


#define define_simple_da(T, name)                                              \
  typedef struct {                                                             \
    T *items;                                                                  \
    int length;                                                                \
    int capacity;                                                              \
  } name

define_simple_da(vstr, vstr_da);

define_simple_da(cstr, cstr_da);
define_simple_da(short, short_da);
define_simple_da(unsigned short, ushort_da);
define_simple_da(int, int_da);
define_simple_da(unsigned int, uint_da);
define_simple_da(long, long_da);
define_simple_da(unsigned long, ulong_da);
define_simple_da(float, float_da);
define_simple_da(double, double_da);

typedef cstr_da Cmd;

enum LogLevel {
  LogLevel_Nolog,
  LogLevel_Info,
  LogLevel_Warning,
  LogLevel_Error
};

typedef struct {
  ino_t ino;
  cstr name;
  unsigned short reclen;
  unsigned char type;
} DirEntry;

typedef struct {
  cstr path;
  DirEntry *items;
  int capacity;
  int length;
} Dir;
