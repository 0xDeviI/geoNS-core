/* TODO/TEMP:
    1. Send proper HTTP response error on failures
    2. Accessing config to check for any method allow check
    3. Add piped HTTP response formats (for exmaple, all responses should contain header, showing web server)
    4. Accessing config to check for connection-close when header length exceeds the limits
    5. Accessing config to check for connection-close when body length exceeds the limits
*/

#include "parser.h"

uchar is_body_required(char *method) {
    uchar *body_less_methods[] = {
        "GET",
        "HEAD",
        "DELETE",
        "OPTIONS"
    };
    uchar size_of_body_less_methods = sizeof(body_less_methods) / sizeof(body_less_methods[0]);
    for (int i = 0; i < size_of_body_less_methods; i++)
        if (!strncmp(method, body_less_methods[i], strlen(body_less_methods[i])))
            return 0;
    return 1;
}


uchar to_lower_string(char *string) {
    if (string == NULL)
        return 0;

    size_t size_of_string = strlen(string);
    for (int i = 0; i < size_of_string; i++)
        string[i] = tolower(string[i]);
    
    return 1;
}

HTTPRequest *parse_http_request(SocketConnection *connection) {
    HTTPRequest *http_request = (HTTPRequest *) memalloc(sizeof(HTTPRequest));
    if (http_request == NULL) {
        perror("Memory error");
        return NULL;
    }
    http_request->fd = connection->fd;

    // Accessing body
    char *request_save_ptr, *request_line_save_ptr, *header_save_ptr;
    char *body_start = strstr(connection->buffer, "\r\n\r\n");

    // Accessing headers
    char *end_of_request_line = strstr(connection->buffer, "\n");
    http_request->request_line_offset = end_of_request_line - connection->buffer;
    http_request->headers_offset = (body_start - connection->buffer) + 4;
    http_request->headers_size = http_request->headers_offset - http_request->request_line_offset;
    http_request->headers_size = http_request->headers_size > BASE_HTTP_REQUEST_HEADER_SIZE
    ? BASE_HTTP_REQUEST_HEADER_SIZE
    : http_request->headers_size;
    // TODO: consider closing the connection if the security configuration allows
    // TODO: check the config from the config manager.

    http_request->headers = (char *) memalloc(http_request->headers_size);
    if (http_request->headers == NULL) {
        perror("Memory error");
        free(http_request);
        return NULL;
    }
    strncpy(http_request->headers, connection->buffer + http_request->request_line_offset + 1, http_request->headers_size);

    char *token = strtok_r(connection->buffer, "\n", &request_save_ptr);
    while (token != NULL) {
        if (!http_request->parse_status.is_request_line_parsed) {
            char request_line[MAX_HTTP_METHOD_LENGTH + MAX_HTTP_URI_LENGTH + MAX_HTTP_VERSION_LENGTH];
            strncpy(request_line, token, sizeof(request_line));
            char *request_line_token = strtok_r(request_line, " ", &request_line_save_ptr);
            while (request_line_token != NULL) {
                if (!http_request->parse_status.is_method_parsed) {
                    strncpy(http_request->method.name, request_line_token, sizeof(http_request->method.name));
                    http_request->method.is_body_required = is_body_required(http_request->method.name);
                    http_request->parse_status.is_method_parsed = 1;
                }
                else if (!http_request->parse_status.is_uri_parsed) {
                    strncpy(http_request->uri, request_line_token, sizeof(http_request->uri));
                    http_request->parse_status.is_uri_parsed = 1;
                }
                else {
                    if (!strncmp("HTTP/0.9", request_line_token, MAX_HTTP_VERSION_LENGTH))
                        http_request->version = HTTP_0_9;
                    else if (!strncmp("HTTP/1.0", request_line_token, MAX_HTTP_VERSION_LENGTH))
                        http_request->version = HTTP_1_0;
                    else if (!strncmp("HTTP/1.1", request_line_token, MAX_HTTP_VERSION_LENGTH))
                        http_request->version = HTTP_1_1;
                    else if (!strncmp("HTTP/2.0", request_line_token, MAX_HTTP_VERSION_LENGTH))
                        http_request->version = HTTP_2_0;
                    else if (!strncmp("HTTP/3.0", request_line_token, MAX_HTTP_VERSION_LENGTH))
                        http_request->version = HTTP_3_0;
                    else
                        http_request->version = UNKNOWN;
                    http_request->parse_status.is_version_parsed = 1;
                    break;
                }
                request_line_token = strtok_r(NULL, " ", &request_line_save_ptr);
            }
            http_request->parse_status.is_request_line_parsed = 1;
        }
        else
            break;

        token = strtok_r(NULL, "\n", &request_save_ptr);
    }

    if (!http_request->method.is_body_required)
        recv_message(connection->fd, connection->buffer, connection->buffer_size, 0);
    else {
        char *content_length_header = get_http_header_value(http_request, "Content-Length");
        if (content_length_header == NULL)
            http_request->body_size = 0;
        else {
            char *endptr;
            http_request->body_size = strtol(content_length_header, &endptr, 10);
            if (endptr == content_length_header)
                http_request->body_size = 0;
        }
        free(content_length_header);

        if (http_request->body_size != 0) {
            connection->buffer_size = http_request->headers_offset + http_request->body_size;
            if (connection->buffer_size > MAX_HTTP_REQUEST_SIZE) {
                send_http_status_bodyless(http_request, 413);
                return NULL;
            }
            else {
                connection->buffer = (char *) realloc(connection->buffer, connection->buffer_size);
                if (connection->buffer == NULL) {
                    send_http_status_bodyless(http_request, 413);
                    return NULL;
                }
                recv_message(connection->fd, connection->buffer, connection->buffer_size, 0);
            }
        }
    }

    return http_request;
}