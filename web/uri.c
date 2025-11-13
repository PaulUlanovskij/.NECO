#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../core/str.h"
#include "../core/helpers.h"

#include "uri.h"

cstr gen_delims = ":/?#[]@";
cstr sub_delims = "!$&'()*+,;=";

bool uri_valid_pct_encoded(vstr pct_encoded){
  if(pct_encoded.length != 3) return false;

  bool valid = pct_encoded.items[0] == '%' && 
         isxdigit(pct_encoded.items[1]) && 
         isxdigit(pct_encoded.items[2]);

  try(valid);

  return true;
}

bool uri_valid_unreserved(char c){
  return isalpha(c) || isdigit(c) || 
         c == '-' || c == '.' || c == '_' || c == '~';
}
char uri_parse_pct_encoded(vstr pct){
  char fst = pct.items[1]; 
  char snd = pct.items[2]; 
  char res = fst - (isdigit(fst) ? '0' : 'A');

  res*=16;
  res = snd - (isdigit(snd) ? '0' : 'A');
  return res;
}
cstr uri_decode(vstr str){
  dstr ds = {};
  defer(da_free(&ds));

  da_for(&str, i, c){
    if(*c == '%'){
      dstr_append(&ds, uri_parse_pct_encoded(slice(str, i, i+3)));
      i+=3;
    }else{
      dstr_append(&ds, *c);
    }
  }
  return to_cstr(ds);
}

#define uri_valid_pct_encoded_or(c, i, vstr, expr)                             \
do{                                                                            \
  if((c) == '%'){                                                              \
    if((i) + 2 >= (vstr).length) return false;                                 \
    try(uri_valid_pct_encoded(slice((vstr), (i), (i)+3)));                     \
    (i)+=2;                                                                    \
  }else{                                                                       \
    try((expr));                                                               \
  }                                                                            \
}while(0)

bool uri_valid_reg_name(vstr reg){
  da_for(&reg, i, c){
    uri_valid_pct_encoded_or(*c, i, reg, 
                             uri_valid_unreserved(*c) || 
                             strchr(sub_delims, *c));
  }
  return true;
}

bool uri_valid_ipv4(vstr ip){
  if(ip.length < 7 || ip.length > 15) return false;
  
  vstr_da splits = split_by_char(ip, '.', SSO_NONE);
  defer(da_free(&splits));
  if(splits.length != 4) return false;

  da_foreach(&splits, split){
    if(split->length == 0 || split->length > 3) return false;
    switch(split->length){
      case 1:{
        try(isdigit(split->items[0]));
      }break;
      case 2:{
        if(split->items[0] < 0x31 || 
           split->items[0] > 0x39 || 
           isdigit(split->items[1]) == false) return false;
      }break;
      case 3:{
        if(split->items[0] != '2' || 
           split->items[1] < 0x30 || 
           split->items[1] > 0x35)            return false;
        if(split->items[1] != '5' && 
           isdigit(split->items[2]) == false) return false;
        if(split->items[1] == '5' && 
           split->items[2] > 0x35)            return false;
      }break;
    }
  }

  return true;
}
bool uri_valid_ipv6(vstr ip){
  if(ip.length < 2 || ip.length > 45) return false;

  int dccount = count_word(ip, "::");
  if(dccount > 1) return false;

  vstr_da splits = split_by_char(ip, ':', SSO_REMOVE_EMPTY);
  defer(da_free(&splits));
  
  bool ends_with_ipv4 = splits.length == 0 ? false 
                      : uri_valid_ipv4(slice(ip, splits.items[splits.length-1].items - ip.items, -1));
  if(splits.length > 8 - dccount - ends_with_ipv4) return false;

  for(int i = 0; i < splits.length - ends_with_ipv4; i++){
    vstr split = splits.items[i];
    if(split.length > 4) return false;
    da_foreach(&split, c){
      try(isxdigit(*c));
    }
  }

  return true;
}
bool uri_valid_ipvfuture(vstr ip){
  if(ip.length < 4)             return false;
  if(ip.items[0] != 'v')        return false; 
  
  int dot_index = char_index(ip, '.');
  if(dot_index == -1)           return false;
  if(dot_index == ip.length-1)  return false;

  for(int i = 1; i < dot_index; i++){
    try(isxdigit(ip.items[i]));
  }

  for(int i = dot_index+1; i < ip.length; i++){
    char c = ip.items[i];
    bool valid = uri_valid_unreserved(c) || strchr(sub_delims, c) || c == ':';
    try(valid);
  }

  return true;
}
bool uri_valid_ip_literal(vstr ip){
  if(ip.length == 0) return false;
  if(ip.items[0] == 'v'){
    return uri_valid_ipvfuture(ip);
  }
  return uri_valid_ipv6(ip);
}
bool uri_valid_host(vstr host){
  if(host.length == 0) return false;
  if(host.items[0] == '['){
    if(host.items[host.length-1] == ']'){
      vstr ip_literal = slice(host, 1, host.length-1);
      return uri_valid_ip_literal(ip_literal);
    }
    return false;
  }
  return uri_valid_ipv4(host) || uri_valid_reg_name(host);;
}

bool uri_valid_userinfo(vstr userinfo){
  da_for(&userinfo, i, c){
    if(*c == ':'){
      continue; //maybe store something?
    }else if(strchr(sub_delims, *c)){
      continue; //maybe store something?
    }else if(*c == '%'){
      if(i+2 >= userinfo.length) return false;
      try(uri_valid_pct_encoded(slice(userinfo, i, i+3)));
      i+=2;
      continue; //maybe store something?
    }else if(uri_valid_unreserved(*c)){
      continue; //maybe store someting?
    }else{
      return false;
    }
  }
  return true;
}
bool uri_valid_port(vstr port){
  da_foreach(&port, c){
    try(isdigit(*c));
  }
  return true;
}
bool uri_valid_authority(vstr authority){
  int userinfo_end = char_index(authority, '@');
  if(userinfo_end != -1){ //has userinfo
    vstr userinfo = slice(authority, 2, userinfo_end);
    try(uri_valid_userinfo(userinfo));
  }
  authority = slice(authority, userinfo_end+1, -1);

  int host_end = char_index(authority, ':');
  vstr host = slice(authority, 0, host_end);
  try(uri_valid_host(host));

  authority = slice(authority, host_end+1, -1);

  if(host_end != -1){ //has port
    vstr port = authority;
    try(uri_valid_port(port));
  }

  return true;
}
bool uri_valid_segment(vstr segment){
  da_for(&segment, i, c){
    uri_valid_pct_encoded_or(*c, i, segment, 
                             uri_valid_unreserved(*c) || 
                             strchr(sub_delims, *c) || 
                             *c == ':' || *c == '@');
  }
  return true;
}
bool uri_valid_segment_nz(vstr segment){
  bool nz = false;
  da_for(&segment, i, c){
    uri_valid_pct_encoded_or(*c, i, segment, 
                             uri_valid_unreserved(*c) || 
                             strchr(sub_delims, *c) || 
                             *c == ':' || *c == '@');
    nz = true;
  }
  return nz;
}
bool uri_valid_segment_nz_nc(vstr segment){
  bool nz = false;
  da_for(&segment, i, c){
    uri_valid_pct_encoded_or(*c, i, segment, 
                             uri_valid_unreserved(*c) || 
                             strchr(sub_delims, *c) || 
                             *c == ':' || *c == '@');
    nz = true;
  }
  return nz;
}
bool uri_valid_abempty_path(vstr path){
  if(path.length == 0) return true;
  if(path.items[0] != '/') return false;

  vstr_da splits = split_by_char(path, '/', SSO_NONE);
  defer(da_free(&splits));

  da_foreach(&splits, split){
    try(uri_valid_segment(*split));
  }

  return true;
}
bool uri_valid_absolute_path(vstr path){
  if(path.length == 0) return false;
  if(path.items[0] != '/') return false;

  vstr_da splits = split_by_char(path, '/', SSO_NONE);
  defer(da_free(&splits));

  da_for(&splits, i, split){
    if(i == 1){
      try(uri_valid_segment_nz(*split));
    }else{
      try(uri_valid_segment(*split));
    }
  }

  return true; 
} 
bool uri_valid_rootless_path(vstr path){
  if(path.length == 0) return false;
  if(path.items[0] == '/') return false;

  vstr_da splits = split_by_char(path, '/', SSO_NONE);
  defer(da_free(&splits));

  da_for(&splits, i, split){
    if(i == 0){
      try(uri_valid_segment_nz(*split));
    }else{
      try(uri_valid_segment(*split));
    }
  }

  return true; 
}
bool uri_valid_nonscheme_path(vstr path){
  if(path.length == 0) return false;
  if(path.items[0] == '/') return false;

  vstr_da splits = split_by_char(path, '/', SSO_NONE);
  defer(da_free(&splits));

  da_for(&splits, i, split){
    if(i == 0){
      try(uri_valid_segment_nz_nc(*split));
    }else{
      try(uri_valid_segment(*split));
    }
  }

  return true; 
}

bool uri_parse_hier_part(vstr hier, URI *uri){
  if(hier.length == 0){ //path-empty
    if(uri){
      uri->authority = (vstr){};
      uri->path_type = URI_PATH_EMPTY;
      uri->path = (vstr){};
    }
    return true; 
  }
  
  if(hier.length > 2 && starts_with(hier, "//")){ // "//" authority path-abempty
    hier = slice(hier, 2, -1);
    int authority_end = char_index(hier, '/');
    vstr authority = slice(hier, 0, authority_end);
    try(uri_valid_authority(authority));
    vstr path = {};
    if(authority_end != -1){
      path = slice(hier, authority_end, -1);
      try(uri_valid_abempty_path(path));
    }
    if(uri){
      uri->path_type = URI_PATH_ABEMPTY;
      uri->path = path;
      uri->authority = authority;
    }
  }else if(hier.items[0] == '/'){ //path-absolute
    try(uri_valid_absolute_path(hier)); 
    if(uri){
      uri->path_type = URI_PATH_ABSOLUTE;
      uri->path = hier;
      uri->authority = (vstr){};
    }
  }else{//path-rootless
    try(uri_valid_rootless_path(hier)); 
        if(uri){
      uri->path_type = URI_PATH_ROOTLESS;
      uri->path = hier;
      uri->authority = (vstr){};
    }
  }
  return true;
}

bool uri_parse_scheme(vstr scheme, URI *uri){
  if(scheme.length == 0)                return false;
  try(isalpha(scheme.items[0]));
 
  da_foreach(&scheme, c){
    bool valid = isalpha(*c) || isdigit(*c) || 
                 *c == '+' || *c == '-' || *c == '.'; 
    try(valid);
  }
  if(uri) uri->scheme = scheme;
  return true;
}

bool uri_parse_query(vstr query, URI *uri){
  da_for(&query, i, c){
    uri_valid_pct_encoded_or(*c, i, query, 
                             uri_valid_unreserved(*c) || 
                             strchr(sub_delims, *c) || 
                             *c == ':' || *c == '@' ||
                             *c == '/' || *c == '?');
  }
  if(uri) uri->query = query;
  return true;   
}
bool uri_parse_fragment(vstr fragment, URI *uri){
    da_for(&fragment, i, c){
    uri_valid_pct_encoded_or(*c, i, fragment, 
                             uri_valid_unreserved(*c) || 
                             strchr(sub_delims, *c) || 
                             *c == ':' || *c == '@' ||
                             *c == '/' || *c == '?');
  }
  if(uri) uri->fragment = fragment;
  return true;   
}

bool uri_parse(vstr raw_uri, URI *uri){
  int scheme_end = char_index(raw_uri, ':');
  if(scheme_end == -1) return false;

  vstr scheme = slice(raw_uri, 0, scheme_end);
  try(uri_parse_scheme(scheme, uri));

  raw_uri = slice(raw_uri, scheme_end+1, -1);

  int query_start = char_index(raw_uri, '?');
  int fragment_start = char_index(raw_uri, '#');

  int hier_end = (query_start == -1) ? fragment_start : query_start;

  vstr hier = slice(raw_uri, 0, hier_end);
  try(uri_parse_hier_part(hier, uri));

  if(query_start != -1){
    vstr query = slice(raw_uri, query_start+1, fragment_start);   
    try(uri_parse_query(query, uri));
  }
  if(fragment_start != -1){
    vstr fragment = slice(raw_uri, fragment_start+1, -1);   
    try(uri_parse_fragment(fragment, uri));
  }

  return true;
}

bool uri_parse_absolute(vstr raw_uri, URI *uri){
  int scheme_end = char_index(raw_uri, ':');
  if(scheme_end == -1) return false;

  vstr scheme = slice(raw_uri, 0, scheme_end);
  try(uri_parse_scheme(scheme, uri));

  raw_uri = slice(raw_uri, scheme_end+1, -1);

  int query_start = char_index(raw_uri, '?');

  int hier_end = query_start;

  vstr hier = slice(raw_uri, 0, hier_end);
  try(uri_parse_hier_part(hier, uri));

  if(query_start != -1){
    vstr query = slice(raw_uri, query_start+1, -1);   
    try(uri_parse_query(query, uri));
  }

  return true;
}

bool uri_parse_relative_part(vstr rel_part, URI *uri){
    if(rel_part.length == 0){ //path-empty
    if(uri){
      uri->authority = (vstr){};
      uri->path_type = URI_PATH_EMPTY;
      uri->path = (vstr){};
    }
    return true; 
  }
  
  if(rel_part.length > 2 && starts_with(rel_part, "//")){ // "//" authority path-abempty
    rel_part = slice(rel_part, 2, -1);
    int authority_end = char_index(rel_part, '/');
    vstr authority = slice(rel_part, 0, authority_end);
    try(uri_valid_authority(authority));
    vstr path = {};
    if(authority_end != -1){
      path = slice(rel_part, authority_end, -1);
      try(uri_valid_abempty_path(path));
    }
    if(uri){
      uri->path_type = URI_PATH_ABEMPTY;
      uri->path = path;
      uri->authority = authority;
    }
  }else if(rel_part.items[0] == '/'){ //path-absolute
    try(uri_valid_absolute_path(rel_part)); 
    if(uri){
      uri->path_type = URI_PATH_ABSOLUTE;
      uri->path = rel_part;
      uri->authority = (vstr){};
    }
  }else{//path-noscheme
    try(uri_valid_rootless_path(rel_part)); 
        if(uri){
      uri->path_type = URI_PATH_NOSCHEME;
      uri->path = rel_part;
      uri->authority = (vstr){};
    }
  }
  return true;
}

bool uri_parse_relative_ref(vstr rel_ref, URI *uri){
  int query_start = char_index(rel_ref, '?');
  int fragment_start = char_index(rel_ref, '#');

  int rel_part_end = (query_start == -1) ? fragment_start : query_start;

  vstr rel_part = slice(rel_ref, 0, rel_part_end);
  try(uri_parse_relative_part(rel_part, uri));

  if(query_start != -1){
    vstr query = slice(rel_ref, query_start+1, fragment_start);   
    try(uri_parse_query(query, uri));
  }
  if(fragment_start != -1){
    vstr fragment = slice(rel_ref, fragment_start+1, -1);   
    try(uri_parse_fragment(fragment, uri));
  }
}
bool uri_parse_uri_reference(vstr raw_uri, URI *uri){
  if(uri_parse(raw_uri, uri) ||
     uri_parse_relative_ref(raw_uri, uri))
    return true;
  
  return false;
}
