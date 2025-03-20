#ifndef GEONS_HTTP_RESPONSE_H
#define GEONS_HTTP_RESPONSE_H 1

#include "../http.h"
#include "../specs.h"

// Errors
void send_http_error_413(HTTPRequest *request);

#endif  //! GEONS_HTTP_RESPONSE_H