#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define UTILS_IMPLEMENTATION
#include "net_stuff.h"
#include "utils.h"

#define BACKLOG 10
#define TICK_RATE 30  // Hz

typedef struct Message {
    char protocol_version[PROTOCOL_LEN + 1];
    uint8_t packet_type;
    uint16_t payload_length;
    char msg[MAX_MESSAGE_SIZE];  // Actually MAX_MESSAGE_SIZE -
                                 // MSG_HEADER_SIZE];
} Message;

typedef struct Client {
    int new_socket_fd;
    struct sockaddr_in client_address;
    Message last_message;
    char *buffer;

    int buffer_offset;  // Current offset in the buffer
} Client;

typedef struct {
    struct timespec last_time;
    float frequency;
} Timer;
void call_if_due(Timer *timer, void (*func)(Client *), Client *arg) {
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    int diff = (current_time.tv_sec - timer->last_time.tv_sec) * 1000000 +
               (current_time.tv_nsec - timer->last_time.tv_nsec) / 1000 -
               1000000 / timer->frequency;

    if (diff >= 0) {
        timer->last_time = current_time;
        func(arg);
    } else
        usleep(-1 * diff);
}

void handle_client(Client *arg) {
    Message *message = &arg->last_message;
    int bytes_received =
        recv(arg->new_socket_fd, arg->buffer + arg->buffer_offset,
             MAX_MESSAGE_SIZE * 2 - arg->buffer_offset, 0);
    if (bytes_received == 0) return;

    if (bytes_received < 0) {
        fprintf(stderr, "WARN: recv error for client %d: %s\n",
                arg->new_socket_fd, strerror(errno));
        return;
    }
    arg->buffer_offset += bytes_received;

    while (arg->buffer_offset >= MSG_HEADER_SIZE) {
        int ret =
            sscanf(arg->buffer, "%3s|%02d|%4hd|", &message->protocol_version,
                   &message->packet_type, &message->payload_length);

        if (ret != 3) {
            fprintf(stderr, "ERROR: client %d malformed packet received %s\n",
                    arg->new_socket_fd, arg->buffer);
            // TODO: return for now but handle later
            return;
        }

        strncpy(message->msg, arg->buffer + MSG_HEADER_SIZE,
                message->payload_length);

        printf("INFO: Client %d recv ver: %s, type: %d, msg: %.*s\n",
               arg->new_socket_fd, message->protocol_version,
               message->packet_type, message->payload_length, message->msg);

        int remaining =
            arg->buffer_offset - MSG_HEADER_SIZE - message->payload_length;
        memmove(arg->buffer,
                arg->buffer + (MSG_HEADER_SIZE + message->payload_length),
                remaining);
        arg->buffer_offset = remaining;
    }
}

void *pthread_routine(void *arg) {
    Client *args = (Client *)arg;
    args->buffer = (char *)malloc(sizeof(char) * MAX_MESSAGE_SIZE * 2);
    if (args->buffer == NULL) {
        fprintf(stderr, "ERROR: Can't allocate client buffer\n");
        close(args->new_socket_fd);
        free(args);
    }
    memset(&args->last_message, 0, sizeof(Message));
    args->buffer_offset = 0;
    printf("INFO: Client %d connected\n", args->new_socket_fd);

    Timer tick_timer = {.frequency = TICK_RATE};
    clock_gettime(CLOCK_MONOTONIC_RAW, &tick_timer.last_time);

    while (1) {
        call_if_due(&tick_timer, handle_client, args);
    }

    close(args->new_socket_fd);
    free(args->buffer);
    free(args);
    return NULL;
}

int main(int argc, char **argv) {
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
