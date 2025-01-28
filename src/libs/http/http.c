#include "http.h"
#include "../logger/logger.h"


HTTPServer *HTTP_SERVER = NULL;


void send_http_response(HTTPRequest *request, uchar *response, ssize_t size_of_response) {
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
    if (socket_server != NULL) {
        HTTPServer *http_server = (HTTPServer *) memalloc(sizeof(HTTPServer));
        http_server->public_dir = (uchar *) memalloc(strlen(public_dir_path));
        http_server->socket_server = socket_server;
        strncpy(http_server->public_dir, public_dir_path, strlen(public_dir_path));

        HTTP_SERVER = http_server;
        return http_server;
    }
    msglog(ERROR, "Failed while creating HTTP server on %s:%d", server_addr, port);
    return NULL;
}


void route(HTTPServer *server, uchar *route, HTTPCallback *callback) {
}


uchar *get_file_extension(uchar *file_name) {
    uchar *dot = strrchr(file_name, '.');
    if (!dot || dot == file_name) {
        return "";
    }
    return dot + 1;
}

uchar *get_mime_type(uchar *file_ext) {
    if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
        return "text/html";
    } else if (strcasecmp(file_ext, "txt") == 0) {
        return "text/plain";
    } else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcasecmp(file_ext, "png") == 0) {
        return "image/png";
    } else {
        return "application/octet-stream";
    }
}


void kill_http_server(HTTPServer *server) {
    if (server != NULL) {
        kill_socket_server(server->socket_server);
        free(server->public_dir);
        free(server);
        server = NULL;
    }
}


void http_server_callback(int fd, uchar *request, PeerInfo *peer_info) {
    // Parsing request:
        // 1. storing headers
        // 2. storing method
        // 3. storing body
        // 4. storing URI
        // 5. looking for URI within public folder
        // 6. if this is not URI, search for routes

    
    HTTPRequest *http_request = (HTTPRequest *) memalloc(sizeof(HTTPRequest));
    http_request->fd = fd;
    uchar *request_copy = (uchar *) memalloc(strlen(request));
    strncpy(request_copy, request, strlen(request));
    request_copy[strlen(request)] = '\0'; // Ensure null-termination
    char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello world form C web server!\n";
    send_http_response(http_request, response, strlen(response));

    // // Parse the request line (method, URI, version)
    // uchar *line = strtok(request_copy, "\r\n");
    // if (line == NULL) {
    //     send_http_response(http_request, "400: Bad Request", 16);
    //     free(request_copy);
    //     return;
    // }

    // // Extract method
    // uchar *token = strtok(line, " ");
    // if (token == NULL) {
    //     send_http_response(http_request, "400: Bad Request", 16);
    //     free(request_copy);
    //     return;
    // }
    // strncpy(http_request->method, token, MAX_HTTP_METHOD_LENGTH - 1);
    // http_request->method[MAX_HTTP_METHOD_LENGTH - 1] = '\0';

    // // Extract URI
    // token = strtok(NULL, " ");
    // if (token == NULL) {
    //     send_http_response(http_request, "400: Bad Request", 16);
    //     free(request_copy);
    //     return;
    // }
    // strncpy(http_request->uri, token, MAX_HTTP_URI_LENGTH - 1);
    // http_request->uri[MAX_HTTP_URI_LENGTH - 1] = '\0';

    // // Extract HTTP version
    // token = strtok(NULL, " ");
    // if (token == NULL) {
    //     send_http_response(http_request, "400: Bad Request", 16);
    //     free(request_copy);
    //     return;
    // }
    // strncpy(http_request->version, token, MAX_HTTP_VERSION_LENGTH - 1);
    // http_request->version[MAX_HTTP_VERSION_LENGTH - 1] = '\0';


    // http_request->size_of_headers = 0;
    // strncpy(request_copy, request, strlen(request));
    // line = strtok(request_copy, "\r\n");

    // while (line != NULL) {
    //     // Parse headers
    //     uchar *colon = strchr(line, ':');
    //     if (colon) {
    //         *colon = '\0'; // Split the line into key and value
    //         uchar *key = line;
    //         uchar *value = colon + 2; // Skip the ": " part
    //         HTTPHeader *header = &http_request->headers[http_request->size_of_headers++];
    //         header->name = key;
    //         header->value = value;
    //     }
    //     line = strtok(NULL, "\r\n");
    // }


    // // Check for body
    // uchar *body_start = strstr(request, "\r\n\r\n");
    // if (body_start != NULL) {
    //     body_start += 4; // Move past the "\r\n\r\n"
    //     http_request->body = (uchar *) memalloc(strlen(body_start));
    //     memcpy(http_request->body, body_start, strlen(body_start));    //! SIZE SHOULE BE CAME FROM HTTP HEADER, OR CALCULATED IN TIME
    // }

    // for (int i = 0; i < http_request->size_of_headers; i++) {
    //     HTTPHeader header = http_request->headers[i];
    //     printf("%s: %s\n", header.name, header.value);
    // }

    // uchar file_path[MAX_HTTP_URI_LENGTH];
    // snprintf(file_path, MAX_HTTP_URI_LENGTH, "%s/%s", HTTP_SERVER->public_dir, http_request->uri);
    // char is_directory = is_directory_path(file_path);
    // if (is_directory == -1) {
    //     send_http_response(http_request, "404: Not found", 14);
    // }
    // else if (!is_directory) {
    //     if (is_file_exist(file_path)) {
    //         ulong file_size = get_file_size(file_path);
    //         FILE *file_ptr = fopen(file_path, "r");
    //         char file[file_size];
    //         fread(file, file_size, 1, file_ptr);
    //         fclose(file_ptr);
    //         send_http_response(http_request, file, file_size);
    //     }
    //     else {
    //         send_http_response(http_request, "404: Not found", 14);
    //     }
    // }
    // else {
    //     // Start indexing it
    //     uchar *result = exec("ls -la %s", file_path);
    //     if (result != NULL) {
    //         send_http_response(http_request, result, strlen(result));
    //         free(result);
    //     }
    //     else {
    //         send_http_response(http_request, "500: Server error", 17);
    //     }
    // }
    // free(request_copy);
}