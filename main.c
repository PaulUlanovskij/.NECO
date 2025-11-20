/*#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

*/

#include "core/str.h"
#include "core/helpers.h"

#include "ffs/json.h"

#include "web/uri.h"
#include "web/http1.1.h"
#include "web/http1.1server.h"

#define PORT 8080

void handle_client(int client_fd, HTTPMessage msg) {

  if(msg.version.major == 0){
    server_response_upgrade(client_fd);
    return;
  }

  cstr target = uri_decode(msg.request.target);
  defer(free(target));

  if (eql(target, "/") == false) {
    server_response_not_found(client_fd);
    return;
  }

  server_response_send_file(client_fd, "index.html");

  dstr response = {};
  defer(da_free(&response));
}

int server(int argc, cstr argv[argc]){
  HTTPServer server = {};
  bool shutdown = false;

  server_start(&server, PORT);
  server_start_receiving(&server, handle_client, &shutdown);
  server_shutdown(&server);
}

int json_test(int argc, cstr argv[argc]){
  if(argc < 2){
    puts("json where?");
    exit(1);
  }
  Json json = {};
  if(json_parse_begin(&json, argv[1])){;
    puts("Yay, Prased json");
  }else{
    puts("Failed to parse");
  }
  json_parse_end(&json);
}

int main(int argc, cstr argv[argc]) {
  return json_test(argc, argv);
}
