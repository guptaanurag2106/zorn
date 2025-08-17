#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#define UTILS_IMPLEMENTATION
#include "net_stuff.h"
#include "utils.h"

#define BACKLOG 10
#define TICK_RATE 30  // Hz

typedef struct Client {
    int new_socket_fd;
    struct sockaddr_in client_address;
    char buf[MAX_PAYLOAD_SIZE];
} Client;

typedef struct Message {
} Message;

typedef struct {
    clock_t last_time;
    float frequency;
} Timer;
void call_if_due(Timer *timer, void (*func)(void *), void *arg) {
    clock_t current_time = clock();
    float diff = (float)(current_time - timer->last_time) / CLOCKS_PER_SEC -
                 1.0 / timer->frequency;

    if (diff) {
        func(arg);
        timer->last_time = current_time;
    } else
        usleep(diff * 1000 * 1000);
}

void handle_client(void *arg) {
    Client *args = (Client *)arg;
    char *buf = args->buf;
    size_t n = recv(args->new_socket_fd, buf, sizeof(buf), 0);
    if (n == 0) return;
    buf[n] = '\0';

    printf("INFO: client %d says %zu %s\n", args->new_socket_fd, n, buf);
}

void *pthread_routine(void *arg) {
    Client *args = (Client *)arg;
    printf("INFO: Client %d connected\n", args->new_socket_fd);

    Timer tick_timer = {0, TICK_RATE};

    while (1) {
        // call_if_due(&tick_timer, handle_client, arg);
        char *buf = args->buf;
        size_t n = recv(args->new_socket_fd, buf, MAX_PAYLOAD_SIZE, 0);
        if (n == 0) continue;
        buf[n] = '\0';
        printf("INFO: client %d says %zu %s\n", args->new_socket_fd, n, buf);
    }

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
        Client *pthread_arg = (Client *)malloc(sizeof(Client));
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
            fprintf(stderr, "ERROR: cant create new thread, for client: %d\n",
                    cfd);
            fprintf(stderr, "ERROR: CLIENT NOT CONNECTED\n");
            // TODO:send message to client
            free(pthread_arg);
        }
    }
    close(sockfd);

    return 0;
}
