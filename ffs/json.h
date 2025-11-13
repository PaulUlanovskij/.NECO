#pragma once

typedef enum{
  JSON_OBJECT_START,
  JSON_OBJECT_END,
  JSON_ARRAY_START,
  JSON_ARRAY_END,
  JSON_NUMBER,
  JSON_SYMBOL,
  JSON_NAME
}JSONTokenType;
define_simple_da(JSONTokenType, JSONTokenType_da);

typedef struct{
  cstr path;
  vstr_o contents;

  vstr_da tokens;
  JSONTokenType_da token_types;

  int token_index;
} Json;

bool json_parse_begin(Json *json, cstr path);
void json_parse_end(Json *json);

bool json_object_has_members(Json *json);
bool json_array_has_members(Json *json);
vstr json_next(Json *json, JSONTokenType* type);
bool json_tokenize(Json *json, vstr contents);
vstr json_parse_value(Json *json, vstr val_bucket);
bool json_parse_object(Json *json, vstr object_body);
bool json_parse_array(Json *json, vstr arr_body);
vstr json_capture_string(vstr val);

bool json_valid_string(vstr string);
bool json_valid_number(vstr number);
bool json_valid_exponent(vstr exponent);
bool json_valid_fraction(vstr fraction);
bool json_valid_integer(vstr integer);


