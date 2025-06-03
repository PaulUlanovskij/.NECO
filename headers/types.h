#pragma once
typedef struct {
  char *items;
  int length;
} StringView, StringView_o;

typedef struct {
  char *items;
  int length;
  int capacity;
} StringBuilder;

typedef char *cstr, *cstr_o;

typedef struct {
  void *base_ptr;
  void *alloc_ptr;
  int capacity;
} ArenaAllocator;

typedef struct{
  StringBuilder sb;
  cstr exec_file;
}Cmd;
enum LogLevel {
  LogLevel_Nolog,
  LogLevel_Info,
  LogLevel_Warning,
  LogLevel_Error
};
