#ifndef GEONS_HTTP_ROUTER_H
#define GEONS_HTTP_ROUTER_H 1

#include "../http.h"
#include "home.h"
#include "about.h"

void setup_geons_http_router(HTTPServer *server) {
    if (server == NULL) return;

    // Web Routes
    route(server, "/", callback_home);
    route(server, "/about", callback_about);
}

#endif // !GEONS_HTTP_ROUTER_H