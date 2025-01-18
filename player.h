#pragma once

#include "vector.h"

typedef struct Player {
    float x, y;
    float theta;
    float eye_z;
    float hfov;
    float vfov;
    Vector2 speed;
    float rotate_speed;
} Player;

void move_player(Player *player, float dt);
void move_player_by_coord(Player *player, float dx, float dy, float dz);

void rotate_player(Player *player, float dt);
void rotate_player_by_angle(Player *player, float dtheta);

