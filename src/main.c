#include "server.h"
#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

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
