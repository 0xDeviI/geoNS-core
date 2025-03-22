#include "response.h"

void send_http_status_bodyless(HTTPRequest *request, ushort status_code) {
    HTTPResponse *response = create_http_response(
        status_code,
        get_reason_phrase(status_code),
        "",
        0
    );
    send_http_response(request, response);
    kill_http_connection(request);
    free_http_response(response);
}

void send_http_status(HTTPRequest *request, ushort status_code, char *body, size_t body_size) {
    HTTPResponse *response = create_http_response(
        status_code,
        get_reason_phrase(status_code),
        body_size > 0 ? body : "",
        body_size
    );
    send_http_response(request, response);
    kill_http_connection(request);
    free_http_response(response);
}