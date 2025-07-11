#include <ctype.h>

#include "../core/ht.h"
#include "../core/str.h"
#include "../core/helpers.h"
#include "uri.h"
#include "http1.1.h"

bool http_parse_method(vstr method, HTTPMethod *m){
       if(eql(method, "GET"))     *m = HTTP_GET;
  else if(eql(method, "HEAD"))    *m = HTTP_HEAD;
  else if(eql(method, "POST"))    *m = HTTP_POST;
  else if(eql(method, "PUT"))     *m = HTTP_PUT;
  else if(eql(method, "DELETE"))  *m = HTTP_DELETE;
  else if(eql(method, "CONNECT")) *m = HTTP_CONNECT;
  else if(eql(method, "OPTIONS")) *m = HTTP_OPTIONS;
  else if(eql(method, "TRACE"))   *m = HTTP_TRACE;
  else return false;

  return true;
}
bool http_parse_version(vstr version_field, HTTPMessage* msg){
  if(version_field.length != 8)
    return false;

  if(starts_with(version_field, "HTTP/") && 
     isdigit(version_field.items[5])    &&
     isdigit(version_field.items[7])    && 
     version_field.items[6] == '.'       ){
    if(msg){
      msg->version.major = version_field.items[5] - '0';
      msg->version.minor = version_field.items[7] - '0';
    }
    return true;
  }
  return false;
}
bool http_parse_code(vstr code_field, HTTPStatusCode *c){
  if(code_field.length != 3) return false;

  if(isdigit(code_field.items[0]) &&
     isdigit(code_field.items[1]) && 
     isdigit(code_field.items[2]) ){
    short code  = (code_field.items[0] - '0') * 100;
          code += (code_field.items[1] - '0') * 10;
          code += (code_field.items[2] - '0') * 1;

    for(int i = 0; i < len(HTTPValidStatusCodes); i++){
      if(HTTPValidStatusCodes[i] == code){
        if(c) *c = code;
        return true;
      }
    }
  }
  return false;
}
bool http_valid_token(vstr token){
  bool valid = false;
  da_foreach(&token, c){
    try(strchr("!#$%&'*+-.^_`|~", *c) || isdigit(*c) || isalpha(*c));
    valid = true;
  }
  return valid;
}

bool http_valid_field_name(vstr field_name){
  return http_valid_token(field_name);
}
bool http_valid_field_value(vstr field_value){
  if(field_value.length == 0) return true;

  try(isgraph(field_value.items[0]) || field_value.items[0] & 0x80);
  if(field_value.length == 1) return true;

  bool valid = false;
  for(int i = 1; i < field_value.length-1; i++){
    char c = field_value.items[i];
    try(c == ' ' || c == '\t' || isgraph(c) || c & 0x80);
    valid = true;
  }
  try(valid);

  try(isgraph(field_value.items[field_value.length-1]) || 
      field_value.items[field_value.length-1] & 0x80);

  return true;
}
bool http_parse_field_line(vstr field_line, vstr* field_name, vstr* field_value){
  vstr_da parts = split_by_char(field_line, ':', SSO_NONE);
  defer(da_free(&parts));

  if(parts.length < 2) return false;

  try(http_valid_field_name(parts.items[0]));
  try(http_valid_field_value(trim(parts.items[1])));

  if(field_name) *field_name = parts.items[0];
  if(field_value) *field_value = trim(parts.items[1]);

  return true;
}

bool http_valid_absolute_path(vstr absolute_path){
  if(absolute_path.length == 0) return true;
  if(absolute_path.items[0] != '/') return false;

  vstr_da splits = split_by_char(absolute_path, '/', SSO_NONE);
  defer(da_free(&splits));

  if(splits.length < 1) return false;

  da_foreach(&splits, split){
    try(uri_valid_segment(*split));
  }

  return true;
}
bool http_valid_request_target_origin_form(vstr origin_form){
  int query_start = char_index(origin_form, '?');
  try(http_valid_absolute_path(slice(origin_form, 0, query_start)));

  if(query_start != -1){
    try(uri_parse_query(slice(origin_form, query_start+1, -1), nullptr));
  }
  return true;
}
bool http_valid_request_target_absolute_form(vstr absolute_form){
  return uri_parse_absolute(absolute_form, nullptr);
}
bool http_valid_request_target_authority_form(vstr authority_form){
  vstr_da parts = split_by_char(authority_form, ':', SSO_NONE);
  defer(da_free(&parts));

  if(parts.length != 2) return false;

  try(uri_valid_host(parts.items[0]));
  try(uri_valid_port(parts.items[1]));

  return true;
}
bool http_parse_request_target(vstr request_target, HTTPMessage *msg){
  if(count_char(request_target, ' ') != 0) return false;
  
  if(eql(request_target, "*")){
    msg->request.form = HTTP_REQUEST_TARGET_FORM_ASTERISK;
    msg->request.target = request_target;
    return true;
  }else if(http_valid_request_target_origin_form(request_target)){
    msg->request.form = HTTP_REQUEST_TARGET_FORM_ORIGIN;
    msg->request.target = request_target;
    return true;
  }else if(http_valid_request_target_absolute_form(request_target)){
    msg->request.form = HTTP_REQUEST_TARGET_FORM_ABSOLUTE;
    msg->request.target = request_target;
    return true;
  }else if(http_valid_request_target_authority_form(request_target)){
    msg->request.form = HTTP_REQUEST_TARGET_FORM_AUTHORITY;
    msg->request.target = request_target;
    return true;     
  }
  return false;
}

bool http_parse_status_line(vstr status_line, HTTPMessage *msg){
  int version_end = char_index(status_line, ' ');
  vstr version = slice(status_line, 0, version_end);
  try(http_parse_version(version, msg));

  status_line = slice(status_line, version_end + 1, -1);
  int code_end = char_index(status_line, ' ');
  vstr code = slice(status_line, 0, code_end);
  try(http_parse_code(code, &msg->response.status));

  status_line = slice(status_line, code_end + 1, -1);
  da_foreach(&status_line, c){
    try(*c == ' ' || *c == '\t' || isgraph(*c) || (*c & 0x80));
  }
  msg->response.reason = status_line; 

  return true; 
}

bool http_parse_field_lines(vstr_da lines, HTTPMessage *msg){
  ht_init(&msg->fields, lines.length, vstr_hash, vstr_eql_vstr);
  vstr name, value;
  da_foreach(&lines, line){
    try(http_parse_field_line(*line, &name, &value));
    ht_set(&msg->fields, name, value);
  }
  return true;
}
bool http_parse_request_line(vstr request_line, HTTPMessage *msg){
  vstr_da parts = split_by_char(request_line, ' ', SSO_NONE);
  defer(da_free(&parts));

  if(parts.length != 3) return false;

  try(http_parse_method(parts.items[0], &msg->request.method));   
  try(http_parse_request_target(parts.items[1], msg));   
  try(http_parse_version(parts.items[2], msg)); 

  return true;
}

bool http_parse(vstr raw_message, HTTPMessage *msg){
  int_da double_linebreaks = index_word(raw_message, "\r\n\r\n");
  defer(da_free(&double_linebreaks));

  if(double_linebreaks.length != 1) return false;

  vstr_da header_lines = split_by_word(slice(raw_message, 0, double_linebreaks.items[0]), "\r\n", SSO_NONE);
  defer(da_free(&header_lines));

  if(header_lines.length < 1) return false;

  vstr start_line = header_lines.items[0];
  if(http_parse_status_line(start_line, msg)){
    msg->type = HTTP_RESPONSE;
  } else if(http_parse_request_line(start_line, msg)){
    msg->type = HTTP_REQUEST;
  }else{
    return false;
  }

  vstr_da field_lines = header_lines;
  da_slice(field_lines, 1, -1);

  http_parse_field_lines(field_lines, msg);
  
  vstr body = slice(raw_message, double_linebreaks.items[0] + 4, -1);
  if(msg) msg->body = body;
}
