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

// sets buffer to HTTP response, based on response status code
void set_http_response(char *buffer, size_t buffer_size, enum status_code response_status_code) {
    char *status_line = NULL;

    if (response_status_code == OK) {
        status_line = "HTTP/1.0 200 OK";
        char *content_type_header = "Content-Type: text/html";
        char *html_body = "<html><body>hello world</body></html>";
        int html_body_length = strlen(html_body);
    
        // create Content-Length Header
        char content_length_header[50];
        snprintf(content_length_header, sizeof(content_length_header), "Content-Length: %d", html_body_length);
    
        // set buffer to HTTP response
        snprintf(buffer, buffer_size, "%s\r\n%s\r\n%s\r\n\r\n%s", status_line, content_type_header, content_length_header, html_body);
        return;
    
    } else if (response_status_code == BAD_REQUEST) {
        status_line = "HTTP/1.0 400 BAD REQUEST";
    
    } else if (response_status_code == NOT_IMPLEMENTED) {
        status_line = "HTTP/1.0 501 NOT IMPLEMENTED";
    }

    snprintf(buffer, buffer_size, "%s\r\n", status_line);
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

    char http_response_buffer[RESPONSE_BUFFER_SIZE];

    // build http response based on status code result
    set_http_response(http_response_buffer, sizeof(http_response_buffer), status_code_result);    

    if (send(connection_sockfd, http_response_buffer, strlen(http_response_buffer), 0) == -1) {
        perror("send() failure");
        close(connection_sockfd);
        return;
    }

    close(connection_sockfd);
}
