#ifndef GEONS_HTTP_ROUTER_H
#define GEONS_HTTP_ROUTER_H 1

#include <stdarg.h>
#include <curl/curl.h>
#include "../http.h"
#include "../../core/strmap.h"
#include "home.h"
#include "about.h"


void get(HTTPServer *server, uchar *route, HTTPCallback callback);
void post(HTTPServer *server, uchar *route, HTTPCallback callback);
void put(HTTPServer *server, uchar *route, HTTPCallback callback);
void patch(HTTPServer *server, uchar *route, HTTPCallback callback);
void delete(HTTPServer *server, uchar *route, HTTPCallback callback);
void head(HTTPServer *server, uchar *route, HTTPCallback callback);
void options(HTTPServer *server, uchar *route, HTTPCallback callback);
void route(HTTPServer *server, uchar *method, uchar *route, HTTPCallback callback);


static inline void test(void *args, ...) {
    if (args == NULL) {
        return;
    }
    
    HTTPRequest *http_request = (HTTPRequest *) args;
    va_list ap;
    va_start(ap, args);
    StringMap *params = va_arg(ap, StringMap *);
    va_end(ap);

    uchar *username_str = string_map_get(params, "username");

    // CURL *curl = curl_easy_init();
    // char *username = curl_easy_unescape(curl, username_str, 0, NULL);
    printf("username: %s\n", username_str);
    // curl_free(username);
    // curl_easy_cleanup(curl);

    free(username_str);

    callback_about(http_request);
}

void setup_geons_http_router(HTTPServer *server);

#endif // !GEONS_HTTP_ROUTER_H