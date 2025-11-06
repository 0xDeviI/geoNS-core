#ifndef GEONS_HTTP_SPECS_H
#define GEONS_HTTP_SPECS_H 1

#include "../core/core.h"
#include "../socket/socket.h"

#define HTTP_VERSION "HTTP/1.1"

typedef enum eHTTPVersion {
    HTTP_0_9,
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2_0,
    HTTP_3_0,
    UNKNOWN=-1
} HTTPVersion;

typedef struct sHTTPStatusCode {
    ushort code;
    uchar *reason_phrase;
} HTTPStatusCode;

typedef struct sHTTPRequestParseStatus {
    uchar is_request_line_parsed: 1;
    uchar is_method_parsed: 1;
    uchar is_uri_parsed: 1;
    uchar is_version_parsed: 1;
} HTTPRequestParseStatus;

typedef struct sHTTPMethod {
    uchar name[MAX_HTTP_METHOD_LENGTH];
    uchar is_body_required : 1;
} HTTPMethod;

typedef struct sHTTPRequest {
    HTTPRequestParseStatus parse_status;
    HTTPMethod method;
    uchar uri[MAX_HTTP_URI_LENGTH];
    HTTPVersion version;
    ushort request_line_offset;
    ushort headers_size;
    char *headers;
    ushort headers_offset;
    char *body;
    size_t body_size;
    int fd;
} HTTPRequest;

typedef struct sHTTPResponse {
    ushort status_code;
    uchar *reason_phrase;
    char *headers;
    char *body;
    size_t body_size;
} HTTPResponse;

typedef struct sHTTPServer {
    uchar *public_dir;
    SocketServer *socket_server;
} HTTPServer;

typedef void (HTTPCallback)(void *args, ...);

typedef struct sRoute {
    uchar *value;
    struct sRoute *next;
    uchar is_parametric : 1;
} Route;

typedef struct sHTTPRoute {
    Route *route;
    ushort segment_size;
    HTTPCallback *callback;
} HTTPRoute;

extern HTTPServer *HTTP_SERVER;

#endif // !GEONS_HTTP_SPECS_H