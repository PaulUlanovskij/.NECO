#include <stdio.h>
#include <stdlib.h>
#include "web/uri.h"
#include "web/http1.1.h"
#include "core/str.h"

int main(int argc, cstr argv[argc]){
  if(argc < 2){
    puts("path to http where?");
    exit(1);
  }
  vstr http = {};
  if(vstr_from_file(&http, argv[1]) == false){
    puts("failed top read file");
  }
  HTTPMessage msg = {};
  if(http_parse(http, &msg)){
    puts("---   HTTP   ---");
    printf("type: %s\n", (msg.type == HTTP_RESPONSE) ? "Response" : "Request"); 
    if(msg.type == HTTP_REQUEST){
      printf("method: %d\n", msg.request.method);
      printf("request_target: %.*s\n", msg.request.target.length, msg.request.target.items);
    }else{
      printf("status: %d\n", msg.response.status);
      printf("reason: %.*s\n", msg.response.reason.length, msg.response.reason.items);
    }
    printf("version: %d.%d\n", msg.version.minor, msg.version.major);
    puts("fields:");
    ht_for(&msg.fields, i, k, v){
      printf("%.*s: %.*s\n", k.length, k.items, v.length, v.items);
    }
    printf("body: %.*s\n", msg.body.length, msg.body.items);
  }else{
    puts("Failed to parse");
  }
}
void uri_parser(int argc, cstr argv[argc]){
  if(argc < 2){
    puts("uri where?");
    exit(1);
  }
  vstr uri = cstr_to_vstr(argv[1]);
  URI parsed = {};
  if(uri_parse(uri, &parsed)){
    puts("---   URI   ---");
    printf("Scheme: %.*s\n", parsed.scheme.length, parsed.scheme.items);
    printf("Authority: %.*s\n", parsed.authority.length, parsed.authority.items);
    printf("Path: %.*s\n", parsed.path.length, parsed.path.items);
    printf("Query: %.*s\n", parsed.query.length, parsed.query.items);
    printf("Fragment: %.*s\n", parsed.fragment.length, parsed.fragment.items);
  }else{
    puts("Invalid URI!");
  }  
  puts("");
}
