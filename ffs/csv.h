#pragma once
#include "../core/str.h"

typedef struct{
  cstr path;
  vstr_o contents;

  int columns;
  int rows;
  vstr_da tokens;

  cstr* headers;
  int column_index;
  int row_index;
  int token_index;
} Csv;

void csv_parse_begin(Csv *ctx, cstr path);
void csv_parse_end(Csv *ctx);

void csv_parse_headers(Csv *ctx);

bool csv_has_rows(Csv *ctx);
bool csv_row_has_entries(Csv *ctx);
void csv_next_row(Csv *ctx);

cstr csv_next(Csv *ctx);
long csv_integral(Csv *ctx);
double csv_float(Csv *ctx);
cstr_o csv_string(Csv *ctx);
