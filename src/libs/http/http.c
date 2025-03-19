#include "http.h"
#include "../logger/logger.h"


HTTPServer *HTTP_SERVER = NULL;

char *get_http_header_value(HTTPRequest *request, uchar *header_name) {
    // TODO: should be implemented
    const char *pattern = "^([^:]+):[[:space:]]*(.*)$";
    regex_t regex;
    regmatch_t matches[MAX_HTTP_HEADER_REGEX_MATCH];

    // Compile the regex
    if (regcomp(&regex, pattern, REG_EXTENDED)) {
        perror("Regex error");
        return NULL;
    }

    char *header_copy = (char *) memalloc(request->headers_size);
    if (header_copy == NULL) {
        perror("Memory error");
        regfree(&regex);
        return NULL;
    }
    strncpy(header_copy, request->headers, request->headers_size);

    char *header_token_ptr;
    char *header_field_name = NULL;
    char *header_field_value = NULL;
    char *header_token = strtok_r(header_copy, "\n", &header_token_ptr);
    while (header_token != NULL) {
        if (!regexec(&regex, header_token, MAX_HTTP_HEADER_REGEX_MATCH, matches, 0)) {
            int name_start = matches[1].rm_so;
            int name_end = matches[1].rm_eo;
            size_t size_of_header_name = name_end - name_start;
            if (header_field_name == NULL)
                header_field_name = (char *) memalloc(size_of_header_name);
            else
                header_field_name = (char *) realloc(header_field_name, size_of_header_name);

            if (header_field_name == NULL) {
                regfree(&regex);
                free(header_copy);
                return NULL;
            }

            memset(header_field_name, '\0', size_of_header_name);
            strncpy(header_field_name, header_token + name_start, name_end - name_start);
            header_field_name[name_end - name_start] = '\0';

            if (!strncmp(header_name, header_field_name, sizeof(header_field_name))) {
                // Extract header value
                int value_start = matches[2].rm_so;
                int value_end = matches[2].rm_eo;
                size_t size_of_header_value = value_end - value_start;
                header_field_value = (char *) memalloc(size_of_header_value);
                if (header_field_value == NULL) {
                    free(header_field_name);
                    regfree(&regex);
                    free(header_copy);
                    return NULL;
                }
                
                strncpy(header_field_value, header_token + value_start, value_end - value_start);
                header_field_value[value_end - value_start] = '\0';
                free(header_field_name);
                regfree(&regex);
                free(header_copy);
                return header_field_value;
            }
            else
                header_token = strtok_r(NULL, "\n", &header_token_ptr);
        }
        else
            break;
    }

    regfree(&regex);
    free(header_copy);
    return NULL;
}


void clear_http_headers(HTTPRequest *request) {
    if (request->headers != NULL)
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


char *get_mime_type(const char *file_path) {
    const char *mime_type;
    magic_t magic_cookie;

    magic_cookie = magic_open(MAGIC_MIME_TYPE);
    if (magic_cookie == NULL) {
        perror("Unable to initialize magic library");
        return NULL;
    }

    if (magic_load(magic_cookie, NULL) != 0) {
        perror("Cannot load magic database");
        magic_close(magic_cookie);
        return NULL;
    }

    mime_type = magic_file(magic_cookie, file_path);
    if (mime_type == NULL) {
        perror("Cannot determine mime type");
        magic_close(magic_cookie);
        return NULL;
    }

    char *mime_type_copy = strdup(mime_type);
    magic_close(magic_cookie);
    return mime_type_copy;
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

    // Step 5: Looking for URI within public folder
    char file_path[MAX_SYS_PATH_LENGTH];
    snprintf(file_path, sizeof(file_path), "%s/%s", HTTP_SERVER->public_dir, http_request->uri);

    if (is_file_exist(file_path)) {
        FILE *file = fopen(file_path, "rb");
        if (file == NULL) {
            perror("File error");
            kill_http_connection(http_request);
            return -1;
        }

        size_t file_size = get_file_size(file_path);

        char *file_content = (char *) memalloc(file_size + 1);
        if (file_content == NULL) {
            perror("Memory error");
            fclose(file);
            kill_http_connection(http_request);
            return -1;
        }

        fread(file_content, 1, file_size, file);
        fclose(file);

        char *mime_type = get_mime_type(file_path);
        if (mime_type == NULL) {
            mime_type = "application/octet-stream";
        }

        char response_header[256];
        snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\n\r\n", mime_type, file_size);

        send_http_response(http_request, response_header, strlen(response_header));
        send_http_response(http_request, file_content, file_size);

        free(file_content);
        free(mime_type);
    } else {
        // Step 6: If this is not URI, search for routes
        char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\n";
        send_http_response(http_request, response, strlen(response));
    }

    kill_http_connection(http_request);
    return 1;
}