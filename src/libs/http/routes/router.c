#include "router.h"

void setup_geons_http_router(HTTPServer *server) {
    if (server == NULL) return;

    // Web Routes
    route(server, "/", &callback_home);
    route(server, "/about", &callback_about);
    route(server, "/users/{username}", &test);
}