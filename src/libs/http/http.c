#include "http.h"
#include "../logger/logger.h"


HTTPServer *HTTP_SERVER = NULL;


void send_http_response(HTTPRequest *request, uchar *response, size_t size_of_response) {
    send_message(request->fd, response, size_of_response, 0);
    kill_http_connection(request);
}


void kill_http_connection(HTTPRequest *request) {
    kill_socket(request->fd);
    if (request->body != NULL)
        free(request->body);
    free(request);
    request = NULL;
}


HTTPServer *create_http_server(uchar *server_addr, ushort port, uchar *public_dir) {
    if (public_dir == NULL)
        return NULL;

    if (!strlen(public_dir))
        return NULL;

    uchar public_dir_path[MAX_SYS_PATH_LENGTH];
    if (!is_absolute_path(public_dir)) {
        get_cwd_path(public_dir_path, sizeof(public_dir_path));
        strncat(public_dir_path, public_dir, sizeof(public_dir_path) - 1);
        public_dir = public_dir_path;
    }
    else
        strncpy(public_dir_path, public_dir, sizeof(public_dir_path) - 1);

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

    size_t public_dir_path_size = strlen(public_dir_path);
    http_server->public_dir = (uchar *) memalloc(public_dir_path_size + 1);
    if (http_server->public_dir == NULL) {
        perror("Memory error");
        free(http_server);
        return NULL;
    }

    http_server->socket_server = socket_server;
    strncpy(http_server->public_dir, public_dir_path, public_dir_path_size);
    http_server->public_dir[public_dir_path_size + 1] = '\0';

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


char http_server_callback(SocketConnection *connection) {
    // Parsing request:
        // 1. storing headers
        // 2. storing method
        // 3. storing body
        // 4. storing URI
        // 5. looking for URI within public folder
        // 6. if this is not URI, search for routes

    HTTPRequest *http_request = parse_http_request(connection->fd, connection->buffer);
    if (http_request == NULL) {
        return 0;
    }

    http_request->fd = connection->fd;
    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello world from C web server!\n";
    send_http_response(http_request, response, strlen(response));
    return 1;
}