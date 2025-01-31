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
    HTTP_3_0
} HTTPVersion;

typedef struct sHTTPHeader
{
    uchar name[MAX_HTTP_HEADER_NAME_LENGTH];
    uchar value[MAX_HTTP_HEADER_VALUE_LENGTH];
} HTTPHeader;

typedef struct sHTTPRequest
{
    uchar uri[MAX_HTTP_URI_LENGTH];
    uchar method[MAX_HTTP_METHOD_LENGTH];
    char *headers_buffer;
    HTTPHeader *headers;
    HTTPVersion version;
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