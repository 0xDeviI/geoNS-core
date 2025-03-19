#ifndef GEONS_HTTP_H
#define GEONS_HTTP_H 1

#include <string.h>
#include "specs.h"
#include "parser/parser.h"


char *get_http_header_value(HTTPRequest *request, uchar *header_name);
void clear_http_headers(HTTPRequest *request);
void send_http_response(HTTPRequest *request, uchar *response, size_t size_of_response);
void kill_http_connection(HTTPRequest *request);
HTTPServer *create_http_server(uchar *server_addr, ushort port, uchar *public_dir);
// void route(HTTPServer *server, uchar *route, HTTPCallback *callback);
void kill_http_server(HTTPServer *server);
ssize_t http_server_callback(SocketConnection *connection);

#endif // !GEONS_HTTP_H