#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "table.h"
#include "helpers.h"
#include "str.h"

int table_width(Table table){
  return table.length;
}
int table_height(Table table){
  int height = 0;
  da_foreach(&table, column){
    if(column->length > height){
      height = column->length;
    }
  }
  return height;
}

bool table_has_column(Table table, cstr name){
  da_foreach(&table, column){
    if(cstr_eql_cstr(column->header.name, name)){
      return true;
    }
  }
  return false;
}

bool table_add_column(Table table[static 1], TableColumn column){
  if(column.header.name == nullptr){
    return false;
  }
  if(table_has_column(*table, column.header.name)){
    return false;
  }
  if(column.length > table_height(*table)){
    da_foreach(table, col){
      da_reserve(col, column.length);
      col->length = column.length;
    }
  }else{
    da_reserve(&column, table_height(*table));
  }
  da_append(table, column);
  return true;    
}

bool table_add_row(Table table, TableRow row){
  if(row.length > table.length){
    return false;
  }

  int* sets_by = calloc(table.length, sizeof(int));
  defer(free(sets_by));
  
  for(int ri = 0; ri < row.length; ri++){ 
    da_for(&table, ci, column){
      if(cstr_eql_cstr(row.headers[ri].name, column->header.name)){
        sets_by[ri] = ci;
        goto next_cycle;
      }
    }
    return false;
  next_cycle:
  }

  for(int i = 0; row.length; i++){
    da_append(&table.items[sets_by[i]], *row.cells[i]);
  }

  da_foreach(&table, column){
    if(column->length != table.items[sets_by[0]].length){
      da_append(column, (TableCell){});
    }
  }
  return true;
}

bool _table_add_rowf(Table table, cstr fmt, ...){ 
  auto splits = split_by_char(fmt, ',', SSO_REMOVE_EMPTY);
  defer(da_free(&splits));
 
  if(splits.length == 0){
    return false;
  }

  int* sets_by = calloc(table.length, sizeof(int));
  defer(free(sets_by));
  
  for(int ri = 0; ri < splits.length; ri++){ 
    da_for(&table, ci, column){
      if(vstr_eql_cstr(splits.items[ri], column->header.name)){
        sets_by[ri] = ci;
        goto next_cycle;
      }
    }
    return false;
  next_cycle:
  }

  va_list vl;
  va_start(vl);
  void* arg = nullptr;
  for(int i = 0; (arg = (void*)va_arg(vl, void*)); i++){
    da_append(&table.items[sets_by[i]], (TableCell){.string_val = arg});
  }
  va_end(vl);

  da_foreach(&table, column){
    if(column->length != table.items[sets_by[0]].length){
      da_append(column, (TableCell){});
    }
  }
  return true;
}

int table_get_real_index(Table table, TableCell index){
  int search_index = index.integral_val;
  if(table.index != nullptr){
    da_for(table.index, i, entry){
      switch(table.index->header.type){
        case COLUMN_TYPE_STRING:{
          if(cstr_eql_cstr(entry->string_val, index.string_val)){
            search_index = i; 
          }
        }break;
        case COLUMN_TYPE_INTEGRAL:{
          if(entry->integral_val == index.integral_val){
            search_index = i; 
          }
        }break;
        case COLUMN_TYPE_FLOAT:{
          if(entry->float_val == index.float_val){
            search_index = i; 
          }
        }break;
      }
    }
  }
  return search_index;
}

bool table_remove_column(Table table[static 1], cstr name){
  int ci = -1;
  da_for(table, i, column){
    if(cstr_eql_cstr(column->header.name, name)){
      ci = i;
      break;
    }
  }
  if(ci == -1){
    return false;
  }
  free(table->items[ci].header.name);
  if(table->items[ci].header.type == COLUMN_TYPE_STRING){
    da_foreach(&table->items[ci], entry){
      free(entry);
    }
  }
  da_free(&table->items[ci]);
  memcpy(&table->items[ci], &table->items[table->length-1], sizeof(TableColumn));
  table->length--;

  return true;
}
bool table_remove_row(Table table, TableCell index){
  int search_index = table_get_real_index(table, index); 
  if(search_index < table_height(table)){
    da_foreach(&table, column){
      column->items[search_index] = column->items[column->length - 1];
      column->length--;
    }
  }
  return false;
}

bool table_get_column(Table table, cstr name, TableColumn column[1]){
  da_foreach(&table, col){
    if(cstr_eql_cstr(col->header.name, name)){
      *column = *col;
      return true;
    }
  }
  return false;
}
bool table_get_row(Table table, TableCell index, TableRow row[1]){
  int search_index = table_get_real_index(table, index);

  if(search_index < table_height(table)){
    row->headers = calloc(table.length, sizeof(ColumnHeader));
    row->cells = calloc(table.length, sizeof(TableCell*));
    da_for(&table, i, column){
      memcpy(&row->headers[i], &column->header, sizeof(ColumnHeader));
      row->cells[i] = &column->items[search_index];
    }
    return true;
  }
  return false;
}

void table_print(Table table){
  printf("| ");
  da_foreach(&table, column){
    printf("%.8s", column->header.name);
    printf(" | ");
  }
  printf("\n");
  for(int i = 0; i < table_height(table); i++){
    printf("| ");
    da_foreach(&table, column){
      switch(column->header.type){
        case COLUMN_TYPE_STRING:{
          printf("%.8s", column->items[i].string_val);
        }break;
        case COLUMN_TYPE_INTEGRAL:{
          printf("%.8d", column->items[i].integral_val);
        }break;
        case COLUMN_TYPE_FLOAT:{
          printf("%.8f", column->items[i].float_val);
        }break;
      }
      printf(" | ");
    }
    printf("\n");
  }
}

