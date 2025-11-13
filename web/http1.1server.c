#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
//#include <sys/types.h>

#include "../core/helpers.h"
#include "../core/str.h"
#include "../core/da.h"
#include "http1.1.h"
#include "http1.1server.h"

#define BUFFER_SIZE 1024*1024*100 

void server_start_receiving(HTTPServer *server, client_callback handle_client, bool* kill_switch){
  while (*kill_switch == false) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = 0;

    if ((client_fd = accept(server->server_fd, 
                            (struct sockaddr *)&client_addr, 
                            &client_addr_len)) < 0) {
      perror("accept failed");
      continue;
    }

    cstr buffer = (cstr )malloc(BUFFER_SIZE * sizeof(char));
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);

    HTTPMessage msg = {};

    if (bytes_received > 0 &&
        http_parse(slice(buffer, 0, -1), &msg) && 
        msg.type == HTTP_REQUEST) {
      handle_client(client_fd, msg);
    }
    free(buffer);
    close(client_fd);
  }
}
void server_start(HTTPServer* server, int port){
  // create server socket
  if ((server->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(1);
  }

  // config socket
  struct sockaddr_in server_addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(port)
  };

  // bind socket to port
  if (bind(server->server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(1);
  }

  // listen for connections
  if (listen(server->server_fd, 10) < 0) {
    perror("listen failed");
    exit(1);
  }

  printf("Server listening on port %d\n", port);
}

void server_shutdown(HTTPServer *server){
  close(server->server_fd);
  server->server_fd = 0;
}

cstr get_mime_type(cstr target) {
  if (ends_with(target, ".html") || ends_with(target, ".htm")) {
    return "text/html";
  } else if (ends_with(target, ".txt")) {
    return "text/plain";
  } else if (ends_with(target, ".jpg") || ends_with(target, ".jpeg")) {
    return "image/jpeg";
  } else if (ends_with(target, ".png")) {
    return "image/png";
  } else {
    return "application/octet-stream";
  }
}

void server_response_not_found(int client_fd){
  dstr response = {};
  defer(da_free(&response));
  dstr_append_cstr(&response,
                   "HTTP/1.1 404 Not Found\r\n"
                   "Content-Type: text/plain\r\n"
                   "\r\n"
                   "404 Not Found");
  send(client_fd, response.items, response.length-1, 0);
}
void server_response_upgrade(int client_fd){
  dstr response = {};
  defer(da_free(&response));
  dstr_append_cstr(&response,
                   "HTTP/1.1 426 Upgrade Required\r\n"
                   "Upgrade: HTTP/1.1\r\n"
                   "Connection: Upgrade\r\n"
                   "Content-Type: text/plain\r\n"
                   "\r\n"
                   "This host only accepts HTTP/1.1");
  send(client_fd, response.items, response.length-1, 0);
}

void server_response_send_cstr(int client_fd, cstr content_type, int content_length, cstr content){
  dstr response = {};
  defer(da_free(&response));
  dstr_appendf(&response, 
               "HTTP/1.1 200 OK\r\n"
               "Content-Type: %s; charset=utf-8\r\n"
               "Content-Length: %d\r\n"
               "\r\n"
               "%.*s",
               content_type, 
               content_length, 
               content_length, content);
  send(client_fd, response.items, response.length-1, 0);
}

void server_response_send_file(int client_fd, cstr filepath){
   vstr file_contents = {};
  if(vstr_from_file(&file_contents, filepath) == false){
    server_response_not_found(client_fd); 
    return;
  }

  defer(free(file_contents.items));

  cstr mime_type = get_mime_type(filepath);
  server_response_send_cstr(client_fd, mime_type, file_contents.length, file_contents.items);
}
