#include <string.h>
#include <ctype.h>

#include "../core/str.h"
#include "../core/helpers.h"
#include "../core/da.h"

#include "json.h"

bool json_parse_begin(Json *json, cstr path){
  json->path = path;
  try(vstr_from_file(&json->contents, path));
  json->tokens = (vstr_da){};
  json->token_types = (JSONTokenType_da){};
  json->token_index = 0;
  try(json_tokenize(json, json->contents));
  return true;
}
void json_parse_end(Json *json){
  free(json->contents.items);
  da_free(&json->tokens);
  da_free(&json->token_types);
}

bool json_object_has_members(Json *json){
  return json->token_types.items[json->token_index] != JSON_OBJECT_END;
}
bool json_array_has_members(Json *json){
  return json->token_types.items[json->token_index] != JSON_ARRAY_END;
}
vstr json_next(Json *json, JSONTokenType* type){
  *type = json->token_types.items[json->token_index];
  return json->tokens.items[json->token_index];
}

bool json_tokenize(Json *json, vstr contents){
  try(contents.length != 0);
  return json_parse_value(json, trim(contents)).length != 0;
}

vstr json_parse_value(Json *json, vstr val_bucket){
  if(val_bucket.length == 0) return (vstr){};

  vstr value = {};

  switch(val_bucket.items[0]){
    case '{':{
      vstr obj = capture_by_char(val_bucket, 0, '{', '}');
      if(obj.length == 0) return (vstr){};

      value = obj;

      da_append(&json->tokens, (vstr){});
      da_append(&json->token_types, JSON_OBJECT_START);

      obj = trim(slice(obj, 1, obj.length - 1));
      if(json_parse_object(json, obj) == false) return (vstr){};

      da_append(&json->tokens, (vstr){});
      da_append(&json->token_types, JSON_OBJECT_END);
    }break;
    case '[':{
      vstr arr = capture_by_char(val_bucket, 0, '[', ']');
      if(arr.length == 0) return (vstr){};

      value = arr;

      da_append(&json->tokens, (vstr){});
      da_append(&json->token_types, JSON_ARRAY_START);

      arr = trim(slice(arr, 1, arr.length - 1));
      if(json_parse_array(json, arr) == false) return (vstr){};

      da_append(&json->tokens, (vstr){});
      da_append(&json->token_types, JSON_ARRAY_END);
    }break;
    case '"':{
      vstr sym = json_capture_string(val_bucket);
      if(sym.length == 0) return (vstr){};

      value = sym;

      sym = trim(slice(sym, 1, sym.length - 1));
      if(json_valid_string(sym) == false) return (vstr){};

      da_append(&json->tokens, sym); 
      da_append(&json->token_types, JSON_SYMBOL);

    }break;
    default:{
      vstr num = slice(val_bucket, 0, char_index(val_bucket, ',')); 
      if(num.length == 0) return (vstr){};

      value = num;

      if(json_valid_number(num) == false) return (vstr){};

      da_append(&json->tokens, num); 
      da_append(&json->token_types, JSON_NUMBER);
    }break;
  }

  return value;
}

bool json_parse_object(Json *json, vstr object_body){
  vstr temp = trim(object_body);
  int assign_index = -1;
  while((assign_index = char_index(temp, ':')) != -1){
    if(assign_index == -1){
      try(temp.length == 0);
      return true;
    }

    vstr name = trim(slice(temp, 0, assign_index));
    try(name.length >= 2);
    try(name.items[0]  == '"' && name.items[name.length-1] == '"');
    name = slice(name, 1, name.length-1);
    try(json_valid_string(name));
    da_append(&json->tokens, name);
    da_append(&json->token_types, JSON_NAME);

    vstr val_bucket = trim_left(slice(temp, assign_index + 1, -1));

    vstr value = json_parse_value(json, val_bucket);
    try(value.length != 0);

    temp = trim_left(offset(val_bucket, value.length));
    try(temp.length == 0 || temp.items[0] == ',');
    temp = offset(temp, 1);
  }
  return true;
}

bool json_parse_array(Json *json, vstr arr_body){
  bool got_type = false;
  JSONTokenType type;

  vstr temp = trim(arr_body);
  while(temp.length != 0){
    vstr val_bucket = trim_left(temp);

    vstr value = json_parse_value(json, val_bucket);
    try(value.length != 0);

    temp = trim_left(offset(val_bucket, value.length));
    try(temp.length == 0 || temp.items[0] == ',');
    temp = offset(temp, 1);

    if(got_type){
      try(type == da_top(&json->token_types));
    }else{
      got_type = true;
      type = da_top(&json->token_types);
    }
  }
  return true;
}

vstr json_capture_string(vstr val){
  if(val.length < 2) return (vstr){};
  if(val.items[0] != '"') return (vstr){};

  vstr temp = offset(val, 1);

  int index = -1;
  while((index = char_index(temp, '"')) != -1){
    if(temp.items[index-1] != '\\'){
      temp = offset(temp, index+1);
      return slice(val, 0, temp.items - val.items);
    }
    temp = offset(temp, index+1);
  }
  return (vstr){};
}

bool json_valid_string(vstr string){
  for(int i = 0; i < string.length; i++){
    unsigned char c = string.items[i];
    if(c == '\\'){
      vstr rest = slice(string, i+1, -1);
      try(rest.length != 0);
      if(rest.items[0] == 'u'){
        try(rest.length >= 5);
        for(int j = 1; j < 5; i++){
          try(isxdigit(rest.items[j]));
        }
        i+=4;
      }else{
        i++;
        c = string.items[i];
        try(strchr("\"\\/bfnrt", c));
      }
    }else{
      if(c < 0x20) return false;
    }
  }
  return true;
}

bool json_valid_number(vstr number){
  int fraction_index = char_index(number, '.');
  int exponent_index = first_char_index(number, 2, "eE");

  vstr integral = slice(number, 0, fraction_index == -1 ? exponent_index : fraction_index);
  try(json_valid_integer(integral));

  if(fraction_index != -1){
    vstr fraction = slice(number, fraction_index + 1, exponent_index);
    try(json_valid_fraction(fraction));
  }

  if(exponent_index != -1){
    vstr exponent = slice(number, exponent_index + 1, -1);
    try(json_valid_exponent(exponent));
  }

  return true;
}
bool json_valid_exponent(vstr exponent){
  if(exponent.length == 0) return true;

  try(tolower(exponent.items[0]) == 'e');
  if(exponent.length == 1) return true;

  try(isdigit(exponent.items[1]) ||
      exponent.items[1] == '-' ||
      exponent.items[1] == '+');

  for(int i = 2; i < exponent.length; i++){
    try(isdigit(exponent.items[i]));
  }

  return true;
}
bool json_valid_fraction(vstr fraction){
  if(fraction.length == 0) return true;

  try(fraction.items[0] == '.');
  if(fraction.length == 1) return false;

  for(int i = 1; i < fraction.length; i++){
    try(isdigit(fraction.items[i]));
  }

  return true;
}

bool json_valid_integer(vstr integer){
  if(integer.length == 0) return false;
  if(integer.length == 1 && isdigit(integer.items[0])) return true;
  if(integer.length == 2 && 
     integer.items[0] == '-' && 
     isdigit(integer.items[1])) return true;
  int offset = integer.items[0] == '-';
  try(integer.items[offset] != '0');
  for(int i = offset; i < integer.length; i++){
    try(isdigit(integer.items[i]));
  }
  return true;
}
