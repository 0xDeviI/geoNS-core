#ifndef GEONS_HTTP_PARSER_H
#define GEONS_HTTP_PARSER_H 1

#include "../http.h"

HTTPRequest *parse_http_request(int fd, uchar *request);

#endif // !GEONS_HTTP_PARSER_H