#include "parser.h"

HTTPRequest *parse_http_request(SocketConnection *connection) {
    // TEMP
    recv_message(connection->fd, connection->buffer, connection->buffer_size, 0);

    HTTPRequest *http_request = (HTTPRequest *) memalloc(sizeof(HTTPRequest));
    if (http_request == NULL) {
        perror("Memory error");
        return NULL;
    }
    http_request->fd = connection->fd;
    http_request->headers_count = 1;
    http_request->headers = (HTTPHeader *) memalloc(sizeof(HTTPHeader) * http_request->headers_count);
    if (http_request->headers == NULL) {
        perror("Memory error");
        free(http_request);
        return NULL;
    }
    
    char *request_save_ptr, *request_line_save_ptr, *header_save_ptr;
    char *body_start = strstr(connection->buffer, "\r\n\r\n");

    char *token = strtok_r(connection->buffer, "\n", &request_save_ptr);
    while (token != NULL) {
        if (!http_request->parse_status.is_request_line_parsed) {
            char request_line[MAX_HTTP_METHOD_LENGTH + MAX_HTTP_URI_LENGTH + MAX_HTTP_VERSION_LENGTH];
            strncpy(request_line, token, sizeof(request_line));
            char *request_line_token = strtok_r(request_line, " ", &request_line_save_ptr);
            while (request_line_token != NULL) {
                if (!http_request->parse_status.is_method_parsed) {
                    strncpy(http_request->method, request_line_token, sizeof(http_request->method));
                    http_request->parse_status.is_method_parsed = 1;
                    printf("Method: %s\n", http_request->method);
                }
                else if (!http_request->parse_status.is_uri_parsed) {
                    strncpy(http_request->uri, request_line_token, sizeof(http_request->uri));
                    http_request->parse_status.is_uri_parsed = 1;
                    printf("URI: %s\n", http_request->uri);
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
                    printf("Version: %s\n", request_line_token);
                    http_request->parse_status.is_version_parsed = 1;
                    break;
                }
                request_line_token = strtok_r(NULL, " ", &request_line_save_ptr);
            }
            http_request->parse_status.is_request_line_parsed = 1;
        }
        else if (!http_request->parse_status.is_header_parsed) {
            http_request->parse_status.is_header_parsed = !strncmp(token, "\r", 2);
            if (http_request->parse_status.is_header_parsed) {
                token = strtok_r(NULL, "\n", &request_save_ptr);
                continue;
            }
            else {
                // Parsing headers
                char *header_copy[BASE_HTTP_REQUEST_SIZE];
                strncpy(header_copy, token, sizeof(header_copy));
                char *header_token = strtok_r(header_copy, ":", &header_save_ptr);
                char is_header_name = 1;
                while (header_token != NULL) {
                    HTTPHeader *header = &(http_request->headers[http_request->headers_count - 1]);
                    if (is_header_name) {
                        strncpy(header->name, header_token, sizeof(header->name));
                        is_header_name = 0;
                    }
                    else {
                        header->value = (char *) memalloc(strlen(header_token));
                        strncpy(header->value, header_token, strlen(header_token));
                        http_request->headers_count++;
                        http_request->headers = (HTTPHeader *) realloc(http_request->headers, sizeof(HTTPHeader) * http_request->headers_count);
                        break;
                    }

                    header_token = strtok_r(NULL, ":", &header_save_ptr);
                }
            }
        }
        else
            break;

        token = strtok_r(NULL, "\n", &request_save_ptr);
    }


    return http_request;
}