#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/utils.h"

typedef struct Player {
    char id[37];
    Vector2 pos;
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
    float hp;  // 0-100
} Player;

void move_player(Player *player, float dt, int dir);
void move_player_by_coord(Player *player, Vector2 dl);

void rotate_player(Player *player, float dt, int dir);
void rotate_player_by_angle(Player *player, float dtheta);

void jump_player(Player *player, float dt);
void player_gravity(Player *player, float dt);
