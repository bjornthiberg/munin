#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define BACKLOG 5
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
        exit(EXIT_FAILURE);
    }

    // make socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sockfd == -1) {
        perror("socket() failure");
        exit(EXIT_FAILURE);
    }

    // to avoid "Address already in use"
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    } 

    // bind socket to port
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind() failure");
        freeaddrinfo(res);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    // listen on port
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen() failure");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %s...\n", argv[1]);

    while (1) {
        // accept connection
        addr_size = sizeof client_addr;
        connection_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);

        if (connection_sockfd == -1) {
            perror("accept() failure");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        int bytes_received = recv(connection_sockfd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received == -1) {
            perror("recv() failure");
            close(connection_sockfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        } else if (bytes_received == 0) {
            printf("Client disconnected.\n");
        } else if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // null-terminate received data
            printf("Received from client: %s\n", buffer);
        }

        char *msg = "online!";

        if (send(connection_sockfd, msg, strlen(msg), 0) == -1) {
            perror("send() failure");
            close(connection_sockfd);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        close(connection_sockfd);
    }
    
    close(sockfd);

    return 0;
}
