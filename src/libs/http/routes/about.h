#ifndef GEONS_HTTP_ROUTE_ABOUT_H
#define GEONS_HTTP_ROUTE_ABOUT_H 1

#include "../http.h"

void callback_about(void *args, ...) {
    HTTPRequest *request = (HTTPRequest *) args;
    char *body = "<h1>Welcome to about page!</h1>";
    uchar body_size[32];
    snprintf(body_size, sizeof(body_size), "%d", strlen(body));
    char *headers = NULL;
    set_http_response_header(&headers, "Content-Type", "text/html");
    set_http_response_header(&headers, "Content-Length", body_size);
    set_http_response_header(&headers, "Connection", "Close");
    send_http_status(request, 200, body, strlen(body), &headers);
    kill_http_connection(request);
}

#endif // !GEONS_HTTP_ROUTE_ABOUT_H