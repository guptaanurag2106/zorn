#pragma once

#include <stdint.h>

#include "game_state.h"
#include "player.h"
#include "common/utils.h"

void draw_rectangle(uint32_t *image, Vector2i pos, uint32_t w, uint32_t h,
                    uint32_t image_w, uint32_t image_h, uint32_t colour);

void draw_player_fov(uint32_t *minimap, Player *player);

void draw_minimap(GameState *gs);

void overlay_minimap(GameState *gs);

// TODO:
void render_ceiling(GameState *gs);
// TODO:
void render_floors(GameState *gs);
void draw_scene(GameState *gs);
