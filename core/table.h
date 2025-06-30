#pragma once

#include "da.h"
#include "mem.h"
#include "str.h"

typedef enum {
  COLUMN_TYPE_STRING, 
  COLUMN_TYPE_INTEGRAL, 
  COLUMN_TYPE_FLOAT
}ColumnType;

typedef union{
  double float_val;
  long integral_val;
  cstr string_val;
}TableCell;

typedef struct{
  cstr name;
  ColumnType type;
}ColumnHeader;

typedef struct{
  ColumnHeader header;
  TableCell *items;
  int length;
  int capacity;
}TableColumn;
typedef struct {
  ColumnHeader header;
  TableCell *items;
  int length;
} TableColumnView;

typedef struct{
  TableColumn* index;
  TableColumn *items;
  int length;
  int capacity;
  ArenaAllocator arena;
}Table;
typedef struct {
  TableColumnView *items;
  int length; 
} TableView;

typedef struct{
  TableCell** cells; 
  ColumnHeader* headers;
  int length;
} TableRow;

// TODO: Maybe add ArenaAllocator as a part of Table and allocate all of the 
// associated memory to the table int that arena. This will eliminate headaches
// with not knowing what and when should be freed and how to manage the memory
// of tables.

int table_width(Table table);
int table_height(Table table);

bool table_has_column(Table table, cstr header);

bool table_add_column(Table table[static 1], TableColumn column);
bool table_add_row(Table table, TableRow row);

#define table_add_rowf(table, fmt, ...) _table_add_rowf(table, fmt, __VA_ARGS__, NULL)
bool _table_add_rowf(Table table, cstr fmt, ...);

int table_get_real_index(Table table, TableCell index);

bool table_remove_column(Table table[static 1], cstr name);
bool table_remove_row(Table table, TableCell index);

bool table_get_column(Table table, cstr name, TableColumn column[1]);
bool table_get_row(Table table, TableCell index, TableRow row[1]);

void table_print(Table table);
