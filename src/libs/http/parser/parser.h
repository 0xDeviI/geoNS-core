#ifndef GEONS_HTTP_PARSER_H
#define GEONS_HTTP_PARSER_H 1

#include <regex.h>
#include <ctype.h>
#include "../http.h"

uchar is_body_required(char *method);
uchar to_lower_string(char *string);
HTTPRequest *parse_http_request(SocketConnection *connection);

#endif // !GEONS_HTTP_PARSER_H