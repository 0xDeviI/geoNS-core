#ifndef GEONS_HTTP_ROUTER_H
#define GEONS_HTTP_ROUTER_H 1

#include <stdarg.h>
#include <curl/curl.h>
#include "../http.h"
#include "../../core/strmap.h"
#include "home.h"
#include "about.h"

static inline void test(void *args, ...) {
    if (args == NULL) {
        return;
    }
    
    HTTPRequest *http_request = (HTTPRequest *) args;
    va_list ap;
    va_start(ap, args);
    StringMap *params = va_arg(ap, StringMap *);
    va_end(ap);

    printf("list is:\n");
    string_map_print(params);

    // CURL *curl = curl_easy_init();
    // username = curl_easy_unescape(curl, username, 0, NULL);
    // printf("username: %s\n", username);
    // curl_free(username);
    // curl_easy_cleanup(curl);

    callback_about(http_request);
}

void setup_geons_http_router(HTTPServer *server);

#endif // !GEONS_HTTP_ROUTER_H