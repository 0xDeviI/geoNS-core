#ifndef GEONS_HTTP_ROUTER_H
#define GEONS_HTTP_ROUTER_H 1

#include <stdarg.h>
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
    uchar *username = string_map_get(params, "username");
    printf("username: %s\n", username);
    va_end(ap);
    string_map_free(params);
    callback_about(http_request);
}

void setup_geons_http_router(HTTPServer *server);

#endif // !GEONS_HTTP_ROUTER_H