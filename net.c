#include "net.h"

#include <unistd.h>

#include "game_state.h"

void *network_thread(void *arg) {
    SharedState *ss = (SharedState *)arg;
    while (1) {
        // printf("Networking %f\n", ss->player->pos.x);
        usleep(10000);
    }
    return NULL;
}
