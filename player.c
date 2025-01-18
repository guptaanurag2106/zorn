#include "player.h"
#include <math.h>

void move_player(Player *player, float dt) {
    player->x += dt * player->speed.x;
    player->y += dt * player->speed.y;
}

void move_player_by_coord(Player *player, float dx, float dy, float dz) {
    player->x += dx;
    player->y += dy;
    player->eye_z += dz;
}

void rotate_player(Player *player, float dt) {
    player->theta = fmodf(player->theta + dt * player->rotate_speed, 2 * PI);
}

void rotate_player_by_angle(Player *player, float dtheta) { player->theta = fmodf(player->theta + dtheta, 2 * PI); }
