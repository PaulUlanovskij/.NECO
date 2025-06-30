#include <stdio.h>
#include <stdlib.h>

#include "csv.h"
#include "../core/da.h"
#include "../core/table.h"
#include "../core/str.h"
#include "../core/helpers.h"

vstr_da csv_tokenize(Csv *ctx);

bool csv_row_has_entries(Csv *ctx){
  return ctx->token_index < ctx->columns * (ctx->row_index+1);
}

void csv_parse_headers(Csv *ctx){
  ctx->headers = (cstr*)malloc(sizeof(cstr) * ctx->columns);   
  while(csv_row_has_entries(ctx)){
    vstr token = ctx->tokens.items[ctx->token_index];
    ctx->headers[ctx->column_index] = to_cstr(token);

    ctx->column_index++;
    ctx->token_index++;
  }
  ctx->row_index++;
  ctx->column_index=0;
}

void csv_parse_begin(Csv *ctx, cstr path){
  ctx->path = path;
  if(vstr_from_file(&ctx->contents, path) == false){
    printf("[CSV] Was not able to get contents of a file at path: %s\n", path);
    exit(1);
  }

  csv_tokenize(ctx);
}
void csv_parse_end(Csv *ctx){
  da_free(&ctx->tokens);
  free(ctx->contents.items);
  free(ctx->headers);
}

cstr csv_next(Csv *ctx){
  if(ctx->headers == nullptr){
    return "";
  }
  return ctx->headers[ctx->column_index];
}
void csv_next_row(Csv *ctx){
  ctx->row_index++;
  ctx->column_index=0;
}
void csv_next_column(Csv *ctx){
  ctx->column_index++;
  ctx->token_index++;
}
bool csv_has_rows(Csv *ctx){
  return ctx->row_index < ctx->rows;
}
long csv_integral(Csv *ctx){
  vstr token = ctx->tokens.items[ctx->token_index];
  if(vstr_is_int(token) == false){
    printf("[CSV] Unknown integral format encountered while parsing %s#cell=%d,%d\n", ctx->path, ctx->column_index, ctx->row_index);
    exit(1);            
  }
  cstr_o str = to_cstr(token);
  defer(free(str));

  csv_next_column(ctx);

  return atol(str);
}
double csv_float(Csv *ctx){
  vstr token = ctx->tokens.items[ctx->token_index];
  if(vstr_is_float(token) == false){
    printf("[CSV] Unknown floating point format encountered while parsing %s#cell=%d,%d\n", ctx->path, ctx->column_index, ctx->row_index);
    exit(1);            
  }
  cstr_o str = to_cstr(token);
  defer(free(str));

  csv_next_column(ctx);

  return atof(str);
}
cstr_o csv_string(Csv *ctx){
  vstr token = ctx->tokens.items[ctx->token_index];
  int_da dqs = index_char(token, '"');
  defer(da_free(&dqs);
    csv_next_column(ctx);
  );

  if(dqs.length == 0){
    return to_cstr(token); 
  }

  dstr ds = {};
  defer(da_free(&ds));

  dstr_append_vstr(&ds, slice(token, 0, dqs.items[0]));
  for(int i = 2; i < dqs.length; i+=2){
    dstr_append_vstr(&ds, slice(token, dqs.items[i-1], dqs.items[i]));
  }
  dstr_append_vstr(&ds, slice(token, dqs.items[dqs.length-1], -1));

  return to_cstr(ds);
}

vstr csv_token_quoted_field(char *start, char* end, int line, cstr path){
  vstr token = {.items = start+1, .length = 0};
  char* next = token.items;

  if(next >= end){
    return token; 
  }

  while(next < end){
    if(*next == '"'){
      if(next+1 < end){
        next++;
        if(*next == '\n' || *next == ','){
          return token; 
        }else if(*next == '"'){
          token.length++;
        }else{
          printf("[CSV] Unexpected token at %s:%d\n", path, line);
          exit(1);
        }
      }else{
        return token;
      }
    }
    token.length++;
    next++;
  }
  printf("[CSV] Unexpected token at %s:%d\n", path, line);
  exit(1);
}

vstr csv_token_field(char *start, char* end, int line, cstr path){
  vstr token = {.items = start, .length = (start == end ? 0 : 1)};
  char* next = token.items+1;

  if(next >= end){
    return token; 
  }

  while(*next != ',' && *next != '\n' && next < end){
    if(*next == '"'){
      printf("[CSV] Unexpected token at %s:%d\n", path, line);
      exit(1);
    }
    token.length++;
    next++;
  }

  return token;
}

vstr_da csv_tokenize(Csv *ctx){
  vstr_da tokens = {};
  int index;
  int line = 0;
  int col = 1;
  int cur_col;

  char* end = ctx->contents.items+ ctx->contents.length;
  while(index < ctx->contents.length){
    char* next = ctx->contents.items + index;
    switch(*next){ 
      case ',':
        if(ctx->contents.items+ index-1 >= ctx->contents.items){
          if(ctx->contents.items[index-1] == '\n'){
            cur_col++;
            da_append(&tokens, ((vstr){.items = ctx->contents.items + index-1, .length = 0}));
          }
        }
        index++;

        if(index < ctx->contents.length){
          if(ctx->contents.items[index] == ',' || ctx->contents.items[index] == '\n'){
            cur_col++;
            da_append(&tokens, ((vstr){.items = ctx->contents.items + index -1, .length = 0}));
          }
        }
        break;
      case '\n':
        if(line == 0){
          col = cur_col;
        }
        if(col != cur_col){
          printf("[CSV] Unmatched amount of fields in %s:%d.\n    Expected %d, but got %d\n", ctx->path, line, col, cur_col);
          exit(1);
        }
        cur_col = 0;
        line++;
        index++;
        break;
      case '"':{
        vstr token = csv_token_quoted_field(next, end, line, ctx->path); 
        da_append(&tokens, token);
        index += token.length+2;
        cur_col++;
      }break;
      default:
        vstr token = csv_token_field(next, end, line, ctx->path); 
        da_append(&tokens, token);
        index += token.length;
        cur_col++;
        break; 
    }
  }

  ctx->columns = col;
  ctx->rows = line;
  ctx->tokens = tokens;
}

