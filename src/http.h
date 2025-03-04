#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

enum status_code {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_IMPLEMENTED = 501,
};

// builds a dummy http response message
void get_ok_http_response(char *buffer, size_t buffer_size);
void get_bad_request_http_response(char *buffer, size_t buffer_size);
void get_not_implemented_http_response(char *buffer, size_t buffer_size);
enum status_code get_response_status_code(char *client_buffer);
void handle_client(int connection_sockfd);

#endif
