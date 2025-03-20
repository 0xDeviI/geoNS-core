#include "response.h"

void send_http_error_413(HTTPRequest *request) {
    HTTPResponse *response = create_http_response(
        413,
        "Content Too Large",
        "",
        0
    );
    send_http_response(request, response);
    kill_http_connection(request);
    free_http_response(response);
}