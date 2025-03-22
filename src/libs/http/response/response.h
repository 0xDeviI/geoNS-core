#ifndef GEONS_HTTP_RESPONSE_H
#define GEONS_HTTP_RESPONSE_H 1

#include "../http.h"
#include "../specs.h"

void send_http_status_bodyless(HTTPRequest *request, ushort status_code);
void send_http_status(HTTPRequest *request, ushort status_code, char *body, size_t body_size);

#endif  //! GEONS_HTTP_RESPONSE_H