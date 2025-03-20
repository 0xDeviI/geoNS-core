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


void free_http_headers(HTTPRequest *request) {
    if (request->headers != NULL)
        free(request->headers);
}


uchar set_http_response_header(HTTPResponse *response, uchar *header_name, uchar *header_value, ushort is_final_header) {
    if (response == NULL)
        return 0;

    ushort size_of_header_field = strlen(header_name) + strlen(header_value) + 6;
    if (response->headers == NULL || !strncmp(response->headers, "", 1)) {
        // 6 extra characters, 2 for ': ' and 4 for \r\n\r\n
        response->headers = (char *) memalloc(size_of_header_field);
        if (response->headers == NULL)
            return 0;
        snprintf(response->headers, size_of_header_field, "%s: %s\r\n%s", header_name, header_value, is_final_header ? "\r\n" : "");
    }
    else {
        size_of_header_field = strlen(response->headers) + size_of_header_field;
        response->headers = (char *) realloc(response->headers, size_of_header_field);
        if (response->headers == NULL)
            return 0;
        snprintf(response->headers, size_of_header_field, "%s%s: %s\r\n%s", response->headers, header_name, header_value, is_final_header ? "\r\n" : "");
    }
    return 1;
}


void free_http_response(HTTPResponse *response) {
    if (response == NULL) return;
    if (response->headers != NULL && strncmp(response->headers, "", 1)) free(response->headers);
    free(response);
}


HTTPResponse *create_http_response(ushort status, uchar *reason_phrase, char *body, size_t body_size) {
    HTTPResponse *response = (HTTPResponse *) memalloc(sizeof(HTTPResponse));
    if (response == NULL)
        return NULL;
    
    response->status_code = status;
    response->reason_phrase = reason_phrase;
    response->body = body;
    response->body_size = body_size;
    // TODO: avoid initializing like this. Check for non-nullity whenever you need to access it first.
    response->headers = "";
    return response;
}


uchar send_http_response(HTTPRequest *request, HTTPResponse *response) {
    // TODO: if the file size is too larage (typically more than 1GB, consider sending it in chunks)
    char server_header[64];
    snprintf(server_header, sizeof(server_header), "Server: %s", GEONS_WEBSERVER_INFO);
    size_t response_size = sizeof(HTTP_VERSION) + 5 + strlen(response->reason_phrase) + 2
    + strlen(server_header) + 2
    + strlen(response->headers)
    + response->body_size;
    char *response_buffer = (char *) memalloc(
        response_size
    );

    if (response_buffer == NULL)
        return 0;
    
    snprintf(
        response_buffer, response_size, "%s %d %s\r\n%s\r\n%s",
        HTTP_VERSION,
        response->status_code,
        response->reason_phrase,
        server_header,
        strlen(response->headers) > 0 ? response->headers : "\r\n"
    );

    // Write body to the response buffer
    if (response->body != NULL && response->body_size > 0) {
        memcpy(response_buffer + strlen(response_buffer), response->body, response->body_size);
    }

    // Send the response
    send_message(request->fd, response_buffer, response_size, 0);

    // Free the response buffer
    free(response_buffer);
    return 1;
}


void kill_http_connection(HTTPRequest *request) {
    kill_socket(request->fd);
    if (request->body != NULL)
        free(request->body);
    free_http_headers(request);
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

    if (is_directory_exists(file_path)) {
        // TODO: consider checking config manager in order to allow directory indexing or not
        // If the URI is a directory, return a 403 Forbidden response
        char *body = "Forbidden\n";
        HTTPResponse *response = create_http_response(
            403,
            "Forbidden",
            body,
            strlen(body)
        );
        send_http_response(http_request, response);
        free_http_response(response);
        kill_http_connection(http_request);
        return -1;
    }
    else {
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
    
            HTTPResponse *response = create_http_response(
                200,
                "OK",
                file_content,
                file_size
            );
    
            uchar size_str[32];
            snprintf(size_str, sizeof(size_str), "%zu", file_size);
            set_http_response_header(response, "Content-Type", mime_type, 0);
            set_http_response_header(response, "Content-Length", size_str, 1);
            send_http_response(http_request, response);
            free(file_content);
            free(mime_type);
            free_http_response(response);
        } else {
            // TODO: check for routes first, if nothing found, then follow this approach.
            // Step 6: If this is not URI, search for routes
            char *body = "File not found\n";
            HTTPResponse *response = create_http_response(
                404,
                "Not Found",
                body,
                strlen(body)
            );
            send_http_response(http_request, response);
            free_http_response(response);
        }
    
        kill_http_connection(http_request);
    }
    return 1;
}