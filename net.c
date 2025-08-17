#include "net.h"

#include <arpa/inet.h>
#include <bits/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "game_state.h"

#define PROTOCOL_MAJORV 0
#define PROTOCOL_MINORV 1
#include "net_stuff.h"

#define SENDM(fd, type, ...)                                         \
    ({                                                               \
        char *packet = COMBINE("", __VA_ARGS__);                     \
        sendm_(fd, temp_sprintf("%s|%02d|%04d|%s", PROTOCOL_V, type, \
                                strlen(packet), packet));            \
    })

typedef struct {
    struct timespec last_time;
    float frequency;
} Timer;
void call_if_due(Timer *timer, void (*func)(void *), void *arg) {
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    int diff = (current_time.tv_sec - timer->last_time.tv_sec) * 1000000 +
               (current_time.tv_nsec - timer->last_time.tv_nsec) / 1000 -
               1000000 / timer->frequency;

    if (diff >= 0) {
        func(arg);
        clock_gettime(CLOCK_MONOTONIC_RAW, &timer->last_time);
    } else
        usleep(-1 * diff);
}

int sendm_(int fd, const char *message) {
    printf("INFO: sending %lu: %s\n", strlen(message), message);
    return send(fd, message, strlen(message), 0);
    // printf("%s", packet);                                             \
        // size_t packet_len = strlen(packet);                               \
        // sendm_(fd, COMBINE("|", PROTOCOL_V, get_packet_type_string(type), \
        //                    packet_len, packet));                          \
        //
    // int i = 0;
    // for (i = 0; i < MAX_CONNECT_RETRIES; i++) {
    // if (send(fd, message, strlen(message), 0) < 0) {
    // } else
    // return 0;
    // }
    // return -1;
}

void hello(NetState *ns, const char *player_id) {
    if (SENDM(ns->fd, HELLO, player_id) < 0) {
        fprintf(stderr, "ERROR: HELLO, send failed\n");
        fprintf(stderr, "INFO: OFFLINE MODE\n");
        close(ns->fd);
        ns->status = OFFLINE;
        pthread_exit(NULL);
    }
}

void ping(void *arg) {
    GameState *ss = (GameState *)arg;
    SENDM(ss->netState->fd, PING, "ping");
}

void *network_thread(void *arg) {
    GameState *ss = (GameState *)arg;
    NetState *ns = ss->netState;

    ns->fd = socket(AF_INET, SOCK_STREAM, 0);
    ns->serv_addr.sin_family = AF_INET;
    ns->serv_addr.sin_port = htons(ns->port);
    ns->serv_addr.sin_addr.s_addr = inet_addr(ns->server_name);

    socklen_t addr_size = sizeof(ns->serv_addr);
    int i = 0;
    for (i = 0; i < MAX_CONNECT_RETRIES; i++) {
        if (connect(ns->fd, (struct sockaddr *)&(ns->serv_addr), addr_size) <
            0) {
            // TODO: sleep
            fprintf(stderr, "ERROR: connect error, TRIES: %d/%d\n", i + 1,
                    MAX_CONNECT_RETRIES);
        } else {
            break;
        }
    }
    if (i == MAX_CONNECT_RETRIES) {
        fprintf(stderr, "ERROR: Can't connect to server\n");
        fprintf(stderr, "INFO: OFFLINE MODE\n");
        ns->status = OFFLINE;
        pthread_exit(NULL);
    }

    hello(ns, ss->sharedState->player->id);

    enum PACKET_TYPES type;

    Timer ping_timer = {.frequency = 1};  // 1 HZ
    clock_gettime(CLOCK_MONOTONIC_RAW, &ping_timer.last_time);

    while (1) {
        call_if_due(&ping_timer, ping, (void *)ss);
    }

    close(ns->fd);
    pthread_exit(NULL);
}
