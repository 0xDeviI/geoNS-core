#include "router.h"

void get(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "GET", route, callback);
}

void post(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "POST", route, callback);
}

void put(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "PUT", route, callback);
}

void patch(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "PATCH", route, callback);
}

void delete(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "DELETE", route, callback);
}

void head(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "HEAD", route, callback);
}

void options(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "OPTIONS", route, callback);
}

void route(HTTPServer *server, uchar *method, uchar *route, HTTPCallback callback) {
    set_http_route(server, method, route, callback);
}

void setup_geons_http_router(HTTPServer *server) {
    if (server == NULL) return;

    // Web Routes
    route(server, "GET|POST", "/", &callback_home);
    // get(server, "/", &callback_home);
    get(server, "/about", &callback_about);
    get(server, "/users/{username}", &test);
}