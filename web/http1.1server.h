#pragma once 
#include "../core/str.h"
#include "http1.1.h"

typedef void (*client_callback)(int client_fd, HTTPMessage message);
typedef struct{
  int server_fd;
}HTTPServer;

void server_start_receiving(HTTPServer *server, client_callback handle_client, bool* kill_switch);
void server_start(HTTPServer* server, int port);
void server_shutdown(HTTPServer *server);
cstr get_mime_type(cstr target) ;
void server_response_not_found(int client_fd);
void server_response_upgrade(int client_fd);
void server_response_send_cstr(int client_fd, cstr content_type, int content_length, cstr content);
void server_response_send_file(int client_fd, cstr filepath);
