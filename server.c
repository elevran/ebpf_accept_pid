//go:build ignore

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345
#define BACKLOG 5

int main() {
    int sockfd;
    struct sockaddr_in listener;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // Configure server address structure
    listener.sin_family = AF_INET;
    listener.sin_port = htons(PORT);
    listener.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(listener.sin_zero), '\0', 8);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&listener, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Listen on socket
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("PID %d waiting for connections...\n", getpid());

    // Accept connections in a loop
    while (1) {
        struct sockaddr_in remote;
        socklen_t sin_size = sizeof(struct sockaddr_in);
        int new_fd;

        if ((new_fd = accept(sockfd, (struct sockaddr *)&remote, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf(".");

        close(new_fd);
    }

    close(sockfd);
    return 0;
}
