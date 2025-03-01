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

// create, bind, and listen on socket.
// returns socket descriptor
int setup_server(const char *port) {
    struct addrinfo hints, *res, *p;
    int sockfd, status;

    // load address structs
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in IP

    status = getaddrinfo(NULL, port, &hints, &res);

    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    // loop through results, bind to first available
    for(p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("server: socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
            perror("setsockopt");
            close(sockfd);
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    // done with res
    freeaddrinfo(res);

    // listen on port
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen() failure");
        exit(1);
    }

    printf("Server is listening on port %s...\n", port);
    return sockfd;
}

// handles a single client connection
void handle_client(int connection_sockfd) {
    char buffer[BUFFER_SIZE];
    
    int bytes_received = recv(connection_sockfd, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received == -1) {
        perror("recv() failure");
        close(connection_sockfd);
        return;
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
        return;
    }

    close(connection_sockfd);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,"usage: main [port number]\n");
        exit(1);
    }

    struct sockaddr_storage client_addr;
    socklen_t addr_size;

    int sockfd = setup_server(argv[1]);
    int connection_sockfd;

    while (1) {
        // accept connection
        addr_size = sizeof client_addr;
        connection_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);

        if (connection_sockfd == -1) {
            perror("accept() failure");
            close(sockfd);
            exit(1);
        }

        handle_client(connection_sockfd);
    }
    
    close(sockfd);
    printf("Server shutting down....\n");
    return 0;
}
