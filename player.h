#pragma once

#include "vector.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct Player {
    float x, y;
    float theta;
    float eye_z;
    float hfov;
    float vfov;
    Vector3 velocity;
    float rotate_speed;
    float speed;
    float vert_speed;
    bool is_jumping;
    uint32_t colour;
} Player;

void move_player(Player *player, float dt, int dir);
void move_player_by_coord(Player *player, float dx, float dy, float dz);

void rotate_player(Player *player, float dt, int dir);
void rotate_player_by_angle(Player *player, float dtheta);

void jump_player(Player *player, float dt);
void player_gravity(Player *player, float dt);
