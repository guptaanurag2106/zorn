#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils.h"

#define BACKLOG 10

typedef struct pthread_arg_t {
    int new_socket_fd;
    struct sockaddr_in client_address;
} pthread_arg_t;

void *pthread_routine(void *arg) {
    pthread_arg_t *args = (pthread_arg_t *)arg;
    printf("args: %d\n", args->new_socket_fd);
    char buf[128];
    recv(args->new_socket_fd, buf, sizeof(buf), 0);

    printf("client %d says %s\n", args->new_socket_fd, buf);
    close(args->new_socket_fd);
    free(args);
    return NULL;
}

int main(int argc, char **argv) {
    printf("Hello server\n");
    char *prog_name = shift(&argc, &argv);

    const int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR: socket returned neg fd: %s\n", strerror(errno));
        exit(1);
    }
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) <
        0) {
        fprintf(stderr, "ERROR: Cannot enable SO_REUSEADDR: %s\n",
                strerror(errno));
        exit(1);
    }

    int port = 8000;
    if (argc > 0) {
        port = atoi(shift(&argc, &argv));
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "ERROR: bind error\n");
        exit(1);
    }

    socklen_t addr_len = sizeof(addr);
    getsockname(sockfd, (struct sockaddr *)&addr, &addr_len);
    printf("Running on port: %d\n", ntohs(addr.sin_port));

    if (listen(sockfd, BACKLOG) < 0) {
        fprintf(stderr, "ERROR: cant listen\n");
        exit(1);
    }

    pthread_t pthread;
    pthread_attr_t pthread_attr;
    if (pthread_attr_init(&pthread_attr) != 0) {
        fprintf(stderr, "ERROR: cant initialize pthread_attr\n");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) !=
        0) {
        fprintf(stderr, "ERROR: cant detach pthread\n");
        exit(1);
    }

    while (1) {
        pthread_arg_t *pthread_arg =
            (pthread_arg_t *)malloc(sizeof(pthread_arg_t));
        if (pthread_arg == NULL) {
            fprintf(stderr, "ERROR: cant allocate pthread_arg_t\n");
            exit(1);
        }

        socklen_t client_len = sizeof(pthread_arg->client_address);
        const int cfd =
            accept(sockfd, (struct sockaddr *)&pthread_arg->client_address,
                   &client_len);
        if (cfd < 0) {
            fprintf(stderr, "ERROR: cant accept client\n");
            free(pthread_arg);
            continue;
        }

        pthread_arg->new_socket_fd = cfd;

        if (pthread_create(&pthread, &pthread_attr, pthread_routine,
                           (void *)pthread_arg) != 0) {
            fprintf(stderr, "ERROR: cant create new thread\n");
            free(pthread_arg);
        }
    }
    close(sockfd);

    return 0;
}
