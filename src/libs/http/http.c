#include "http.h"
#include "../logger/logger.h"

const HTTPStatusCode HTTP_STATUSES[] = {
    // 1xx Informational
    {100, "Continue"},
    {101, "Switching Protocols"},
    {102, "Processing"},
    {103, "Early Hints"},
    
    // 2xx Success
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {207, "Multi-Status"},
    {208, "Already Reported"},
    {226, "IM Used"},
    
    // 3xx Redirection
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    
    // 4xx Client Errors
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Payload Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a Teapot"},
    {421, "Misdirected Request"},
    {422, "Unprocessable Entity"},
    {423, "Locked"},
    {424, "Failed Dependency"},
    {425, "Too Early"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {451, "Unavailable For Legal Reasons"},
    
    // 5xx Server Errors
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Variant Also Negotiates"},
    {507, "Insufficient Storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"}
};


char *get_reason_phrase(ushort status_code) {
    ushort size_of_status_codes = sizeof(HTTP_STATUSES) / sizeof(HTTP_STATUSES[0]);
    for (ushort i = 0; i < size_of_status_codes; i++) {
        if (HTTP_STATUSES[i].code == status_code)
            return HTTP_STATUSES[i].reason_phrase;
    }
    return "Unknown Status Code";
}


char *get_http_header_value(HTTPRequest *request, uchar *header_name) {
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


void free_http_request_headers(HTTPRequest *request) {
    if (request->headers != NULL)
        free(request->headers);
}


uchar set_http_response_header(char **headers, uchar *header_name, uchar *header_value) {
    // TODO: make this a variadic function to accept formatting
    if (headers == NULL || header_name == NULL || header_value == NULL)
        return 0;
    
    size_t required_size = snprintf(NULL, 0, "%s: %s\r\n", header_name, header_value);

    if (*headers == NULL) {
        *headers = (char *) malloc(required_size + 1);
        if (*headers == NULL)
            return 0;
        snprintf(*headers, required_size + 1, "%s: %s\r\n", header_name, header_value);
    }
    else {
        size_t previous_header_size = strlen(*headers);
        char *new_headers = (char *) realloc(*headers, previous_header_size + required_size + 1);
        if (new_headers == NULL) {
            return 0; // Don't free *headers here; keep the original buffer.
        }
        *headers = new_headers;
        snprintf(*headers + previous_header_size, required_size + 1, "%s: %s\r\n", header_name, header_value);
    }
    
    return 1;
}



void free_http_response(HTTPResponse *response) {
    if (response == NULL) return;
    if (response->headers != NULL) free(response->headers);
    free(response);
    response = NULL;
}


HTTPResponse *create_http_response(ushort status, uchar *reason_phrase, char *body, size_t body_size, char *headers) {
    HTTPResponse *response = (HTTPResponse *) memalloc(sizeof(HTTPResponse));
    if (response == NULL)
        return NULL;
    
    response->status_code = status;
    response->reason_phrase = reason_phrase;
    response->body = body;
    response->body_size = body_size;
    if (headers != NULL) response->headers = headers;
    return response;
}


uchar send_http_response(HTTPRequest *request, HTTPResponse *response) {
    char server_header[64];
    snprintf(server_header, sizeof(server_header), "Server: %s", GEONS_WEBSERVER_INFO);

    size_t headers_size = (sizeof(HTTP_VERSION) - 1) + 5 + strlen(response->reason_phrase) + 2
                        + strlen(server_header) + 2
                        + (response->headers != NULL ? strlen(response->headers) : 0) + 2;
    
    size_t response_size = headers_size + response->body_size;

    char *response_buffer = (char *) memalloc(response_size + 1);
    if (response_buffer == NULL)
        return 0;

    int written = snprintf(
        response_buffer, headers_size + 1,
        "%s %d %s\r\n%s\r\n%s\r\n",
        HTTP_VERSION,
        response->status_code,
        response->reason_phrase,
        server_header,
        response->headers != NULL ? response->headers : ""
    );

    if (response->body != NULL && response->body_size > 0) {
        memcpy(response_buffer + written, response->body, response->body_size);
    }

    send_message(request->fd, response_buffer, response_size, 0);
    free(response_buffer);
    
    return 1;
}


void kill_http_connection(HTTPRequest *request) {
    kill_socket(request->fd);
    if (request->body != NULL) free(request->body);
    free_http_request_headers(request);
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
    
    SocketServer *socket_server = open_server_socket(server_addr, port, (void *) http_server);
    if (socket_server == NULL) {
        msglog(ERROR, "Failed while creating HTTP server on %s:%d", server_addr, port);
        return NULL;
    }

    socket_server->buffer_size_per_client = BASE_HTTP_REQUEST_SIZE;
    http_server->socket_server = socket_server;
    strncpy(http_server->public_dir, public_dir_path, public_dir_path_size);
    http_server->public_dir[public_dir_path_size] = '\0';

    return http_server;
}


HTTPRoute *is_route_exists(HTTPServer *server, uchar *requested_route, StringMap *out_params) {
    if (!requested_route || !out_params) return NULL;

    if (strcmp(requested_route, "/") == 0) {
        for (ushort i = 0; i < server->size_of_http_routes; i++) {
            if (server->http_routes[i].segment_size == 0) {
                return &(server->http_routes[i]);
            }
        }
        return NULL;
    }

    ushort req_len = strlen(requested_route);
    uchar *copy_uri = memalloc(req_len + 1);
    if (!copy_uri) return NULL;
    strcpy(copy_uri, requested_route);

    char *ptr = NULL;
    char *seg = strtok_r(copy_uri, "/", &ptr);

    uchar *req_segments[128];
    ushort req_count = 0;

    while (seg != NULL) {
        ushort len = strlen(seg);
        uchar *cpy = memalloc(len + 1);
        strcpy(cpy, seg);

        req_segments[req_count++] = cpy;
        seg = strtok_r(NULL, "/", &ptr);
    }

    HTTPRoute *found_route = NULL;

    for (ushort i = 0; i < server->size_of_http_routes; i++) {
        HTTPRoute *route = &(server->http_routes[i]);

        if (route->segment_size != req_count)
            continue;

        RouteSegment *rseg = route->segment;
        uchar matched = 1;

        for (ushort idx = 0; idx < req_count; idx++) {
            if (!rseg) { matched = 0; break; }

            if (!rseg->is_parametric) {
                if (strcmp(rseg->value, req_segments[idx]) != 0) {
                    matched = 0;
                    break;
                }
            } else {
                if (out_params != NULL) {
                    string_map_put(out_params, rseg->value, req_segments[idx]);
                }
            }

            rseg = rseg->next;
        }

        if (matched) {
            found_route = route;
            break;
        }
    }

    // Clean up allocated segments
    for (ushort i = 0; i < req_count; i++) {
        free(req_segments[i]);
    }
    free(copy_uri);
    
    return found_route;
}


RouteSegment *create_route_segment(uchar *segment_value, uchar is_parametric) {
    RouteSegment *segment = (RouteSegment *) memalloc(sizeof(RouteSegment));
    if (segment == NULL) {
        perror("Memory error");
        return NULL;
    }
    segment->value = (uchar *) memalloc(strlen(segment_value) + 1);
    if (segment == NULL) {
        perror("Memory error");
        return NULL;
    }

    segment->is_parametric = is_parametric;
    segment->next = NULL;
    strcpy(segment->value, segment_value);

    return segment;
}


uchar route(HTTPServer *server, uchar *route_str, HTTPCallback *callback) {
    if (!server || !route_str || !callback) return 0;

    if (server->size_of_http_routes >= MAX_HTTP_ROUTES) {
        msglog(ERROR, "Can't route for '%s'. This version of geoNS supports %d routes.",
               route_str, MAX_HTTP_ROUTES);
        return 0;
    }

    if (!strncmp(route_str, "/", 2)) {
        RouteSegment *segment = create_route_segment("/", 0);

        server->http_routes[server->size_of_http_routes++] = (HTTPRoute) {
            .segment = segment,
            .callback = callback,
            .segment_size = 0
        };
        return 1;
    }

    ushort route_len = strlen(route_str);
    uchar *route_copy = memalloc(route_len + 1);
    if (!route_copy) {
        perror("Memory error");
        return 0;
    }
    strcpy(route_copy, route_str);

    // Tokenization
    char *route_ptr = NULL;
    char *segment_str = strtok_r(route_copy, "/", &route_ptr);

    RouteSegment *head = NULL;
    RouteSegment *tail = NULL;
    ushort segment_count = 0;

    while (segment_str != NULL) {

        ushort seg_len = strlen(segment_str);
        uchar is_parametric = (seg_len >= 2 &&
                               segment_str[0] == '{' &&
                               segment_str[seg_len - 1] == '}');

        RouteSegment *node;
        if (!is_parametric) {
            node = create_route_segment(segment_str, is_parametric);
        } else {
            // Parametric → remove { }
            ushort name_len = seg_len - 2;
            uchar *param_name = memalloc(name_len + 1);
            strncpy(param_name, segment_str + 1, name_len);
            param_name[name_len] = '\0';
            node = create_route_segment(param_name, is_parametric);
            free(param_name);
        }

        // Append to linked list
        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }

        segment_count++;

        segment_str = strtok_r(NULL, "/", &route_ptr);
    }

    // No segments → invalid route
    if (segment_count == 0) {
        free(route_copy);
        return 0;
    }

    // Store route definition
    server->http_routes[server->size_of_http_routes++] = (HTTPRoute) {
        .segment = head,
        .callback = callback,
        .segment_size = segment_count
    };

    free(route_copy);
    return 1;
}


void free_route(HTTPRoute route) {
    RouteSegment *current = route.segment;
    while (current != NULL) {
        RouteSegment *next = current->next;
        if (current->value != NULL) {
            free(current->value);
        }
        free(current);
        current = next;
    }
}


void kill_http_server(HTTPServer *server) {
    if (server == NULL) return;

    for (ushort i = 0; i < server->size_of_http_routes; i++)
        free_route(server->http_routes[i]);

    kill_socket_server(server->socket_server);
    if (server->public_dir != NULL) free(server->public_dir);
    free(server);
    server = NULL;
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


ssize_t http_server_callback(void *args, ...) {
    SocketConnection *connection = (SocketConnection *) args;
    va_list ap;
    va_start(ap, args);
    HTTPServer *http_server = va_arg(ap, HTTPServer *);
    va_end(ap);

    HTTPRequest *http_request = parse_http_request(connection);
    if (http_request == NULL) return -1;
    if (http_request->version == UNKNOWN) {
        send_http_status_bodyless(http_request, 400, NULL);
        kill_http_connection(http_request);
        return -1;
    }
    if (http_request->version > HTTP_1_1) {
        send_http_status_bodyless(http_request, 505, NULL);
        kill_http_connection(http_request);
        return -1;
    }
    if (!is_standard_http_method(http_request->method.name) && !CONFIG->http_config.accept_any_method) {
        send_http_status_bodyless(http_request, 501, NULL);
        kill_http_connection(http_request);
        return -1;
    }

    // Step 5: Looking for URI within public folder
    char file_path[MAX_SYS_PATH_LENGTH];
    snprintf(file_path, sizeof(file_path), "%s/%s", http_server->public_dir, http_request->uri);

    StringMap *parameter_map = create_string_map();
    // TODO: here; check why it fails on some route checks
    HTTPRoute *route = is_route_exists(http_server, http_request->uri, parameter_map);
    if (route != NULL) {
        route->callback(http_request, parameter_map);
        return 0;
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

            char *headers = NULL;
            uchar size_str[32];
            snprintf(size_str, sizeof(size_str), "%zu", file_size);
            set_http_response_header(&headers, "Content-Type", mime_type);
            set_http_response_header(&headers, "Content-Length", size_str);
            set_http_response_header(&headers, "Connection", "Close");
            send_http_status(http_request, 200, file_content, file_size, &headers);
            kill_http_connection(http_request);
            free(file_content);
            free(mime_type);
            return 0;
        } else {
            if (CONFIG->http_config.directory_indexing) {
                if (is_directory_exists(file_path)) {
                    char *body = NULL;
                    size_t body_size = 0;
                    if (!get_directory_entries(file_path, http_request->uri, &body, &body_size)) {
                        char *body = "Internal Server Error\n";
                        send_http_status(http_request, 500, body, strlen(body), NULL);
                        kill_http_connection(http_request);
                        return -1;
                    }
        
                    if (body == NULL) {
                        body = strdup("No files or directories found.\n");
                        body_size = strlen(body);
                    }
        
                    char *headers = NULL;
                    set_http_response_header(&headers, "Content-Type", "text/html");
                    set_http_response_header(&headers, "Content-Length", (uchar *)&body_size);
                    set_http_response_header(&headers, "Connection", "Close");
                    send_http_status(http_request, 200, body, body_size, &headers);
                    kill_http_connection(http_request);
                    free(body);
                    return 0;
                }
                else {
                    char *body = "Not found\n";
                    send_http_status(http_request, 404, body, strlen(body), NULL);
                    kill_http_connection(http_request);
                    return -1;
                }
            }
            else {
                char *body = "Forbidden\n";
                send_http_status(http_request, 403, body, strlen(body), NULL);
                kill_http_connection(http_request);
                return -1;
            }
        }
    
        kill_http_connection(http_request);
    }
    return 1;
}