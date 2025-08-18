#include "net.h"

#include <arpa/inet.h>
#include <bits/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "game_state.h"
#include "utilities.h"

#define PROTOCOL_MAJORV 0
#define PROTOCOL_MINORV 1
#include "net_stuff.h"

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
