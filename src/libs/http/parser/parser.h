#ifndef GEONS_HTTP_PARSER_H
#define GEONS_HTTP_PARSER_H 1

#include "../http.h"

HTTPRequest *parse_http_request(SocketConnection *connection);

#endif // !GEONS_HTTP_PARSER_H