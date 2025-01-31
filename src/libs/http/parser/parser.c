#include "parser.h"

HTTPRequest *parse_http_request(int fd, uchar *request) {
    HTTPRequest *http_request = (HTTPRequest *) memalloc(sizeof(HTTPRequest));
    if (http_request == NULL) {
        perror("Memory error");
        return NULL;
    }

    http_request->fd = fd;

    // TODO: needs to be implemented

    return http_request;
}