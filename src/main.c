#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BACKLOG 1
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res;   
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    int sockfd, connection_sockfd, status;

    if (argc != 2) {
        fprintf(stderr,"usage: main [port number]\n");
        exit(1);
    }

    // load address structs
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in IP

    status = getaddrinfo(NULL, argv[1], &hints, &res);

    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    // make socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // bind socket to port
    bind(sockfd, res->ai_addr, res->ai_addrlen);

    // listen on port
    listen(sockfd, BACKLOG);
    printf("Server is listening on port %s...\n", argv[1]);

    // accept connection
    addr_size = sizeof client_addr;
    connection_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);

    char buffer[BUFFER_SIZE];

    int bytes_received = recv(connection_sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // null-terminate received data
        printf("Received: %s\n", buffer);
    }

    char *msg = "online!";
    send(connection_sockfd, msg, strlen(msg), 0);

    close(connection_sockfd);
    close(sockfd);

    freeaddrinfo(res);
}
