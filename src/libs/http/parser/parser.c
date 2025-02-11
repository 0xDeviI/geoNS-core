/* TODO/TEMP:
    1. Accessing config to check for any method allow check
    2. Handling socket buffer based on method
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
    
    const char *pattern = "^([^:]+):[[:space:]]*(.*)$";
    regex_t regex;
    regmatch_t matches[MAX_HTTP_HEADER_REGEX_MATCH];

    // Compile the regex
    if (regcomp(&regex, pattern, REG_EXTENDED)) {
        perror("Regex error");
        free(http_request->headers);
        free(http_request);
        return NULL;
    }
    
    char *request_save_ptr, *request_line_save_ptr, *header_save_ptr;
    char *body_start = strstr(connection->buffer, "\r\n\r\n");
    size_t header_size = 0;

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
        else if (!http_request->parse_status.is_header_parsed) {
            http_request->parse_status.is_header_parsed = !strncmp(token, "\r", 2);
            if (http_request->parse_status.is_header_parsed) {
                token = strtok_r(NULL, "\n", &request_save_ptr);
                continue;
            }
            else {
                if (!regexec(&regex, token, MAX_HTTP_HEADER_REGEX_MATCH, matches, 0)) {
                    HTTPHeader *header = &(http_request->headers[http_request->headers_count - 1]);
                    // Extract header name
                    int name_start = matches[1].rm_so;
                    int name_end = matches[1].rm_eo;
                    size_t size_of_header_name = name_end - name_start;
                    if (size_of_header_name > sizeof(header->name) - 1) {
                        token = strtok_r(NULL, "\n", &request_save_ptr);
                        continue;
                    }
                    strncpy(header->name, token + name_start, name_end - name_start);
                    header->name[name_end - name_start] = '\0';

                    // Extract header value
                    int value_start = matches[2].rm_so;
                    int value_end = matches[2].rm_eo;
                    size_t size_of_header_value = value_end - value_start;
                    size_t current_header_size = size_of_header_name + size_of_header_value;
                    
                    if (!http_request->method.is_body_required) {
                        if (header_size + current_header_size > BASE_HTTP_REQUEST_SIZE) {
                            token = strtok_r(NULL, "\n", &request_save_ptr);
                            continue;
                        }
                    }
                    else {
                        if (http_request->headers_count - 1 > 256) {
                            token = strtok_r(NULL, "\n", &request_save_ptr);
                            continue;
                        }
                    }

                    header->value = (char *) memalloc(
                        (size_of_header_value > BASE_HTTP_HEADER_VALUE_LENGTH
                        ? BASE_HTTP_HEADER_VALUE_LENGTH
                        : size_of_header_value) + 1
                    );
                    strncpy(header->value, token + value_start, value_end - value_start);
                    header->value[value_end - value_start] = '\0';

                    header_size += current_header_size;
                    http_request->headers_count++;
                    http_request->headers = (HTTPHeader *) realloc(http_request->headers, sizeof(HTTPHeader) * http_request->headers_count);
                }
            }
        }
        else
            break;

        token = strtok_r(NULL, "\n", &request_save_ptr);
    }
    regfree(&regex);

    return http_request;
}