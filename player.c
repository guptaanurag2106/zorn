#include "player.h"
#include "constants.h"
#include "vector.h"
#include <math.h>
#include <stdbool.h>

void move_player(Player *player, float dt, int dir) {
    player->x += dt * player->velocity.x * dir;
    player->y += dt * player->velocity.y * dir;
}

void move_player_by_coord(Player *player, float dx, float dy, float dz) {
    player->x += dx;
    player->y += dy;
    player->eye_z += dz;
}

void rotate_player(Player *player, float dt, int dir) {
    player->theta = fmodf(player->theta + dt * player->rotate_speed * dir, 2 * PI);
    player->velocity.x = player->speed * sin(player->theta);
    player->velocity.y = player->speed * cos(player->theta);
    if (fabsf(player->theta - 0) < 0.01)
        player->theta = 0;
    if (fabsf(player->theta - PI_2) < 0.01)
        player->theta = PI_2;
    if (fabsf(player->theta - PI_3_4) < 0.01)
        player->theta = PI_3_4;
    if (fabsf(player->theta - 2 * PI) < 0.01)
        player->theta = 0;
}

void rotate_player_by_angle(Player *player, float dtheta) { player->theta = fmodf(player->theta + dtheta, 2 * PI); }

void jump_player(Player *player, float dt) {
    if (!player->is_jumping && player->eye_z == 100.0f)
        player->velocity.z = player->vert_speed;
    player->is_jumping = true;
}

void player_gravity(Player *player, float dt) {
    if (player->eye_z < 100.0f) {
        player->eye_z = 100.0f;
        player->is_jumping = false;
        player->velocity.z = 0;
    } else if (player->eye_z > 400.0f) {
        player->eye_z = 400.0f;
    }

    if (!player->is_jumping && fabsf(player->velocity.z - 0) < 1) {
        player->velocity.z = 0;
        player->eye_z = 100.0f;
        return;
    }

    player->velocity.z -= dt * GRAVITY;

    player->eye_z += player->velocity.z * dt;
}
