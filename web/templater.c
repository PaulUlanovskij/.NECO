#include <stdio.h>
#include <stdlib.h>

#include "../core/str.h"
#include "../core/helpers.h"
#include "../core/da.h"

void print_escaped(FILE* file, cstr content, int length){
  fprintf(file, "OUT(%d,\"", length*4);
  for(int j = 0; j < length; j++){
    fprintf(file, "\\x%02x", content[j]);
  }
  fprintf(file, "\");\n");
}

bool template(cstr path_no_ext){
  vstr file_contents = {};
  try(vstr_from_file(&file_contents, cstr_concat(path_no_ext, ".tt")));
  defer(free(file_contents.items));

  int_da indices = index_char(file_contents, '@');
  defer(da_free(&indices));
  
  FILE* file = fopen(cstr_concat(path_no_ext, ".tt.h"), "w");
  if(file == nullptr) return false;
  defer(fclose(file));

  if(indices.length == 0){
    print_escaped(file, file_contents.items, file_contents.length); 
    return true;
  }

  bool bold = false;

  print_escaped(file, file_contents.items, indices.items[0]);
  bold = true;

  cstr contents = nullptr;
  int length = 0;
  int i = 0;
  for(; i < indices.length-1; i++){
    cstr contents = file_contents.items + indices.items[i]+1;
    int length = indices.items[i+1]-indices.items[i]-1;
    if(bold){
      fprintf(file, "%.*s\n", length, contents); 
    }else{
      print_escaped(file, contents, length);
    }
    bold = !bold;
  }

  contents = file_contents.items + indices.items[i]+1;
  length = file_contents.length - indices.items[i]-1;
  if(bold){
    fprintf(file, "%.*s\n", length, contents); 
  }else{
    print_escaped(file, contents, length);
  }

  return true;
}

int main(int argc, cstr argv[argc]){
  if(argc != 2){
    puts("Usage: templater <path>");
    exit(1);
  }
  if(template(argv[1]) == false){
    puts("Was unable to access files");
  }
  return 0;
}
