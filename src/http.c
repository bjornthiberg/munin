#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define CLIENT_BUFFER_SIZE 1024
#define RESPONSE_BUFFER_SIZE 1024
#define REQUEST_LINE_MAX_LENGTH 100

// builds a dummy http response message
void get_ok_http_response(char *buffer, size_t buffer_size) {
    char *status_line = "HTTP/1.0 200 OK";
    char *content_type_header = "Content-Type: text/html";
    char *html_body = "<html><body>hello world</body></html>";
    int html_body_length = strlen(html_body);

    // create Content-Length Header
    char content_length_header[50];
    snprintf(content_length_header, sizeof(content_length_header), "Content-Length: %d", html_body_length);

    // build response
    snprintf(buffer, buffer_size, "%s\r\n%s\r\n%s\r\n\r\n%s", status_line, content_type_header, content_length_header, html_body);
}

// build 400 bad request response
void get_bad_request_http_response(char *buffer, size_t buffer_size) {
    char *status_line = "HTTP/1.0 400 BAD REQUEST";

    // build response
    snprintf(buffer, buffer_size, "%s\r\n", status_line);
}

void get_not_implemented_http_response(char *buffer, size_t buffer_size) {
    char *status_line = "HTTP/1.0 501 NOT IMPLEMENTED";

    // build response
    snprintf(buffer, buffer_size, "%s\r\n", status_line);
}

// get HTTP response status code based on request
enum status_code get_response_status_code(char *client_buffer) {
    // get first line
    const char *end = strstr(client_buffer, "\r\n");
    size_t line_length = end - client_buffer; // pointer arithmetic
    if (end == NULL || line_length >= REQUEST_LINE_MAX_LENGTH) {
        return BAD_REQUEST; // No CRLF found or first line too long
    }
    char line[REQUEST_LINE_MAX_LENGTH];
    strncpy(line, client_buffer, line_length);
    line[line_length] = '\0'; 
    
    // extract method, request URI and protocol.
    char *method = strtok(line, " ");
    char *request_uri = strtok(NULL, " ");
    char *http_version = strtok(NULL, " ");

    // BAD REQUEST if any tokens missing
    if (method == NULL || request_uri == NULL || http_version == NULL) {
        return BAD_REQUEST;
    }

    // check for valid GET request.
    if (strcmp(http_version, "HTTP/1.0") != 0) {
        return BAD_REQUEST;
    } else if (strcmp(method, "GET") != 0) {
        return NOT_IMPLEMENTED;
    } else if (strcmp(request_uri, "/") == 0) {
        return OK;
    }
    
    return BAD_REQUEST;
}

// handles a single client connection
void handle_client(int connection_sockfd) {
    char client_buffer[CLIENT_BUFFER_SIZE];
    int bytes_received = recv(connection_sockfd, client_buffer, CLIENT_BUFFER_SIZE - 1, 0);
    
    if (bytes_received == -1) {
        perror("recv() failure");
        close(connection_sockfd);
        return;
    } else if (bytes_received == 0) {
        printf("Client disconnected.\n");
    } else if (bytes_received > 0) {
        client_buffer[bytes_received] = '\0';  // null-terminate received data
        printf("Received from client: %s\n", client_buffer);
    }

    // Parse request Status-Line
    enum status_code status_code_result = get_response_status_code(client_buffer);

    char http_response[RESPONSE_BUFFER_SIZE];

    // build http response based on status code result
    switch (status_code_result) {
        case OK: 
            get_ok_http_response(http_response, sizeof(http_response));
            break;
        case BAD_REQUEST:
            get_bad_request_http_response(http_response, sizeof(http_response));
            break;
        case NOT_IMPLEMENTED:
            get_not_implemented_http_response(http_response, sizeof(http_response));
            break;
    }    

    if (send(connection_sockfd, http_response, strlen(http_response), 0) == -1) {
        perror("send() failure");
        close(connection_sockfd);
        return;
    }

    close(connection_sockfd);
}
