#ifndef GEONS_HTTP_H
#define GEONS_HTTP_H 1

#include <magic.h>
#include <string.h>
#include "specs.h"
#include "parser/parser.h"
#include "../config/config.h"
#include "../xxhash/xxhash.h"

#define GEONS_WEBSERVER_NAME "GeoNSWS"
#define GEONS_WEBSERVER_INFO (GEONS_WEBSERVER_NAME "/" GEONS_VERSION " (" OS_NAME ")")

extern const HTTPStatusCode HTTP_STATUSES[];
extern HTTPRoute HTTP_ROUTES[];

char *get_reason_phrase(ushort status_code);
char *get_http_header_value(HTTPRequest *request, uchar *header_name);
void free_http_request_headers(HTTPRequest *request);
uchar set_http_response_header(char **headers, uchar *header_name, uchar *header_value);
void free_http_response(HTTPResponse *response);
HTTPResponse *create_http_response(ushort status, uchar *reason_phrase, char *body, size_t body_size, char *headers);
uchar send_http_response(HTTPRequest *request, HTTPResponse *response);
void kill_http_connection(HTTPRequest *request);
HTTPServer *create_http_server(uchar *server_addr, ushort port, uchar *public_dir);
short is_route_exists(uchar *route);
ullint get_hash_based_on_route(uchar *route);
short get_free_route_index(uchar *route);
uchar route(HTTPServer *server, uchar *route, HTTPCallback *callback);
void kill_http_server(HTTPServer *server);
ssize_t http_server_callback(SocketConnection *connection);

#endif // !GEONS_HTTP_H