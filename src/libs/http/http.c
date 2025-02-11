#include "http.h"
#include "../logger/logger.h"


HTTPServer *HTTP_SERVER = NULL;

void clear_http_headers(HTTPRequest *request) {
    if (request->headers_count == 1) {
        free(request->headers);
        return;
    }

    for (int i = request->headers_count - 1; i > 0; i--) {
        free(request->headers[i - 1].value);
    }

    free(request->headers);
}


void send_http_response(HTTPRequest *request, uchar *response, size_t size_of_response) {
    send_message(request->fd, response, size_of_response, 0);
}


void kill_http_connection(HTTPRequest *request) {
    kill_socket(request->fd);
    if (request->body != NULL)
        free(request->body);
    clear_http_headers(request);
    free(request);
    request = NULL;
}


HTTPServer *create_http_server(uchar *server_addr, ushort port, uchar *public_dir) {
    if (public_dir == NULL)
        return NULL;

    if (!strlen(public_dir))
        return NULL;

    size_t public_dir_path_size = MAX_SYS_PATH_LENGTH;
    uchar public_dir_path[public_dir_path_size];
    if (!is_absolute_path(public_dir))
        get_cwd_path(public_dir_path, public_dir_path_size);
    strncat(public_dir_path, public_dir, strlen(public_dir));

    if (!is_directory_exists(public_dir_path)) {
        msglog(ERROR, "Public directory %s does not exist for HTTP server.", public_dir_path);
        return NULL;
    }
    
    SocketServer *socket_server = open_server_socket(server_addr, port);
    if (socket_server == NULL) {
        msglog(ERROR, "Failed while creating HTTP server on %s:%d", server_addr, port);
        return NULL;
    }

    socket_server->buffer_size_per_client = BASE_HTTP_REQUEST_SIZE;
    HTTPServer *http_server = (HTTPServer *) memalloc(sizeof(HTTPServer));
    if (http_server == NULL) {
        perror("Memory error");
        return NULL;
    }

    public_dir_path_size = strlen(public_dir_path);
    http_server->public_dir = (uchar *) memalloc(public_dir_path_size + 1);
    if (http_server->public_dir == NULL) {
        perror("Memory error");
        free(http_server);
        return NULL;
    }

    http_server->socket_server = socket_server;
    strncpy(http_server->public_dir, public_dir_path, public_dir_path_size);
    http_server->public_dir[public_dir_path_size] = '\0';

    HTTP_SERVER = http_server;
    return http_server;
}


void kill_http_server(HTTPServer *server) {
    if (server != NULL) {
        kill_socket_server(server->socket_server);
        free(server->public_dir);
        free(server);
        server = NULL;
    }
}


ssize_t http_server_callback(SocketConnection *connection) {
    // Parsing request:
        // 1. storing headers
        // 2. storing method
        // 3. storing body
        // 4. storing URI
        // 5. looking for URI within public folder
        // 6. if this is not URI, search for routes

    HTTPRequest *http_request = parse_http_request(connection);
    if (http_request == NULL) {
        return -1;
    }
    // printf("=== Printing headers ===\n");
    // for (int i = 0; i < http_request->headers_count - 1; i++) {
    //     HTTPHeader *header = &(http_request->headers[i]);
    //     printf("\t%s:%s\n", header->name, header->value);
    // }
    // printf("=== Printing headers DONE ===\n");

    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello world from C web server!\n";
    send_http_response(http_request, response, strlen(response));
    kill_http_connection(http_request);
    return 1;
}