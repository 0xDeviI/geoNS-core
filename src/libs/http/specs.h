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

typedef struct sHTTPHeader
{
    uchar name[MAX_HTTP_HEADER_NAME_LENGTH];
    uchar *value;
} HTTPHeader;

typedef struct sHTTPRequestParseStatus {
    uchar is_request_line_parsed: 1;
    uchar is_header_parsed: 1;
    uchar is_method_parsed: 1;
    uchar is_uri_parsed: 1;
    uchar is_version_parsed: 1;
} HTTPRequestParseStatus;

typedef struct sHTTPRequest
{
    HTTPRequestParseStatus parse_status;
    uchar method[MAX_HTTP_METHOD_LENGTH];
    uchar uri[MAX_HTTP_URI_LENGTH];
    HTTPVersion version;
    // char *headers_buffer;
    size_t headers_count;
    HTTPHeader *headers;
    // HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    char *body;
    int fd;
} HTTPRequest;

typedef struct sHTTPResponse
{
    // HTTPHeader headers[MAX_HTTP_HEADERS];
} HTTPResponse;

typedef struct sHTTPServer
{
    uchar *public_dir;
    SocketServer *socket_server;
} HTTPServer;


typedef void (HTTPCallback)(void *args, ...);

extern HTTPServer *HTTP_SERVER;

#endif // !GEONS_HTTP_SPECS_H