#include "router.h"

void GET(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "GET", route, callback);
}

void POST(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "POST", route, callback);
}

void PUT(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "PUT", route, callback);
}

void PATCH(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "PATCH", route, callback);
}

void DELETE(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "DELETE", route, callback);
}

void HEAD(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "HEAD", route, callback);
}

void OPTIONS(HTTPServer *server, uchar *route, HTTPCallback callback) {
    set_http_route(server, "OPTIONS", route, callback);
}

void setup_geons_http_router(HTTPServer *server) {
    if (server == NULL) return;

    // Web Routes
    GET(server, "/", &callback_home);
    GET(server, "/about", &callback_about);
    GET(server, "/users/{username}", &test);
}