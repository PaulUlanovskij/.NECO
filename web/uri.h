#pragma once
#include "../core/str.h"

typedef enum { 
  URI_PATH_ABEMPTY, 
  URI_PATH_ABSOLUTE, 
  URI_PATH_ROOTLESS, 
  URI_PATH_NOSCHEME, 
  URI_PATH_EMPTY
}URIPathType;
typedef enum {
  URI_HOST_IPV4,
  URI_HOST_IP_LITERAL,
  URI_HOST_REG_NAME
}UriHostType;

typedef struct{
  unsigned char a;
  unsigned char b;
  unsigned char c;
  unsigned char d;
}IPV4;
typedef enum {
  IPV6_TYPE_IPV6,
  IPV6_TYPE_IPVFUTURE
}IPV6Type;
typedef struct{
  IPV6Type type;
  union{
    struct{
      vstr digits;
      vstr rest;
    }ipvfuture;
    struct{
      unsigned short a;
      unsigned short b;
      unsigned short c;
      unsigned short d;
      unsigned short e;
      unsigned short f;
      union{
        struct{
          unsigned short g;
          unsigned short h;
        };
        IPV4 ipv4;
      };
    }ipv6; 
  };
}IPV6;

typedef struct{
  vstr scheme;
  vstr authority;
//  struct{
//    vstr userinfo;
//    struct{
//      UriHostType type;
//      union{
//        IPV4 ipv4;
//        IPV6 ipv6;
//        vstr reg_name;
//      };
//    }host;
//  }authority;
  vstr path;
  URIPathType path_type;
  vstr query;
  vstr fragment;
}URI;

bool uri_valid_pct_encoded(vstr pct_encoded);
bool uri_valid_unreserved(char c);
char uri_parse_pct_encoded(vstr pct);
cstr uri_decode(vstr str);
bool uri_valid_reg_name(vstr reg);
bool uri_valid_ipv4(vstr ip);
bool uri_valid_ipv6(vstr ip);
bool uri_valid_ipvfuture(vstr ip);
bool uri_valid_ip_literal(vstr ip);
bool uri_valid_host(vstr host);
bool uri_valid_userinfo(vstr userinfo);
bool uri_valid_port(vstr port);
bool uri_valid_authority(vstr authority);
bool uri_valid_segment(vstr segment);
bool uri_valid_segment_nz(vstr segment);
bool uri_valid_segment_nz_nc(vstr segment);
bool uri_valid_abempty_path(vstr path);
bool uri_valid_absolute_path(vstr path);
bool uri_valid_rootless_path(vstr path);
bool uri_valid_nonscheme_path(vstr path);
bool uri_parse_hier_part(vstr hier, URI *uri);
bool uri_parse_scheme(vstr scheme, URI *uri);
bool uri_parse_query(vstr query, URI *uri);
bool uri_parse_fragment(vstr fragment, URI *uri);
bool uri_parse(vstr raw_uri, URI *uri);
bool uri_parse_absolute(vstr raw_uri, URI *uri);
bool uri_parse_relative_part(vstr rel_part, URI *uri);
bool uri_parse_relative_ref(vstr rel_ref, URI *uri);
bool uri_parse_uri_reference(vstr raw_uri, URI *uri);
