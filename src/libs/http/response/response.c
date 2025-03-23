#include "response.h"

void send_http_status_bodyless(HTTPRequest *request, ushort status_code, char *headers) {
    HTTPResponse *response = create_http_response(
        status_code,
        get_reason_phrase(status_code),
        "",
        0,
        headers
    );
    send_http_response(request, response);
    free_http_response(response);
}

void send_http_status(HTTPRequest *request, ushort status_code, char *body, size_t body_size, char *headers) {
    HTTPResponse *response = create_http_response(
        status_code,
        get_reason_phrase(status_code),
        body_size > 0 ? body : "",
        body_size,
        headers
    );
    send_http_response(request, response);
    free_http_response(response);
}