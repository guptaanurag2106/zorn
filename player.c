#include "player.h"

#include <math.h>
#include <stdbool.h>

#include "constants.h"

void move_player(Player *player, float dt, int dir) {
    player->pos.x += dt * player->velocity.x * dir;
    player->pos.y += dt * player->velocity.y * dir;
}

void move_player_by_coord(Player *player, Vector2 dl) {
    player->pos = add2D(player->pos, dl);
}

void rotate_player(Player *player, float dt, int dir) {
    player->theta = player->theta + dt * player->rotate_speed * dir;
    if (player->theta >= 2 * PI)
        player->theta -= -2 * PI;
    else if (player->theta < 0.0f)
        player->theta += 2 * PI;

    player->velocity.x = player->speed * sin(player->theta);
    player->velocity.y = player->speed * cos(player->theta);
}

void rotate_player_by_angle(Player *player, float dtheta) {
    player->theta = fmodf(player->theta + dtheta, 2 * PI);
    player->velocity.x = player->speed * sin(player->theta);
    player->velocity.y = player->speed * cos(player->theta);
}

void jump_player(Player *player, float dt) {
    if (!player->is_jumping) {
        player->velocity.z = player->vert_speed;
        player->is_jumping = true;
    }
}

void player_gravity(Player *player, float dt) {
    if (!player->is_jumping) return;
    if (player->eye_z < 100.0f) {
        player->eye_z = 100.0f;
        player->is_jumping = false;
        player->velocity.z = 0;
        return;
    } else if (player->eye_z > 400.0f) {
        player->eye_z = 400.0f;
    }

    player->velocity.z -= dt * GRAVITY;
    player->eye_z += player->velocity.z * dt;
}
