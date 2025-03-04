#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define BACKLOG 5

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
