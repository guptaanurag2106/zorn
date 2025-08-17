#include "render.h"

#include <math.h>
#include <stdint.h>

#include "constants.h"
#include "game_state.h"
#include "player.h"
#include "utilities.h"

static inline void PUTPIX(uint32_t *buf, Vector2i pos, uint32_t colour) {
    if (pos.y >= SCREEN_HEIGHT || pos.y < 0) return;
    if (pos.x >= SCREEN_WIDTH || pos.x < 0) return;
    buf[pos.x + pos.y * SCREEN_WIDTH] = colour;
}

void draw_rectangle(uint32_t *image, Vector2i pos, uint32_t w, uint32_t h,
                    uint32_t image_w, uint32_t image_h, uint32_t colour) {
    for (uint32_t i = 0; i < w; i++) {
        int cx = pos.x + i;
        if (cx >= image_w) break;
        for (uint32_t j = 0; j < h; j++) {
            int cy = pos.y + j;
            if (cy >= image_h) break;
            image[cx + cy * image_w] = colour;
        }
    }
}

void draw_player_fov(uint32_t *minimap, Player *player) {
    for (uint32_t i = 0; i < WORLD_WIDTH;
         i += 100) {  // WORLD_WIDTH/100 number of rays
        float theta =
            (player->theta - player->hfov / 2) + i * player->hfov / WORLD_WIDTH;

        for (uint32_t j = 0; j < MINIMAP_RANGE / 2; j++) {
            float cx = player->pos.x + j * sin(theta);
            float cy = player->pos.y + j * cos(theta);
            if (cx >= WORLD_WIDTH || cy >= WORLD_HEIGHT || cx < 0 || cy < 0)
                break;
            if (MAP[(int)(cx / SCALE) + (int)(cy / SCALE) * MAP_WIDTH] != ' ')
                break;

            minimap[(int)(cx - player->pos.x + MINIMAP_RANGE / 2) +
                    (int)(cy - player->pos.y + MINIMAP_RANGE / 2) *
                        MINIMAP_RANGE] = pack_colour(255, 255, 255, 255);
        }
    }
}

void draw_minimap(GameState *gs) {
    for (uint32_t i = 0; i < MINIMAP_RANGE * MINIMAP_RANGE; i++) {
        (gs->renderState->minimap)[i] = pack_colour(25, 5, 25, 255);
    }

    for (uint32_t i = 0; i < MINIMAP_RANGE; i++) {
        for (uint32_t j = 0; j < MINIMAP_RANGE; j++) {
            uint32_t cx =
                ((gs->sharedState->player->pos.x - MINIMAP_RANGE / 2) + i) /
                SCALE;  // in MAP scale
            uint32_t cy =
                ((gs->sharedState->player->pos.y - MINIMAP_RANGE / 2) + j) /
                SCALE;
            if (cx >= MAP_WIDTH || cy >= MAP_HEIGHT) continue;

            if (MAP[cx + cy * MAP_WIDTH] == ' ') {
                draw_rectangle(gs->renderState->minimap, (Vector2i){i, j}, 1, 1,
                               MINIMAP_RANGE, MINIMAP_RANGE, 0xFFAAAAAA);
            }
        }
    }
    draw_rectangle(gs->renderState->minimap,
                   (Vector2i){MINIMAP_RANGE / 2 - PLAYER_SIZE / 2,
                              MINIMAP_RANGE / 2 - PLAYER_SIZE / 2},
                   PLAYER_SIZE, PLAYER_SIZE, MINIMAP_RANGE, MINIMAP_RANGE,
                   gs->sharedState->player->colour);
    // draw_player_fov(gs->renderState->minimap, gs->sharedState->player);
}

void overlay_minimap(GameState *gs) {
    uint32_t minimap_width = SCREEN_HEIGHT * MINIMAP_SCALE;
    uint32_t minimap_height = minimap_width;

    uint32_t start_x = SCREEN_WIDTH - minimap_width;
    uint32_t start_y = SCREEN_HEIGHT - minimap_height;

    uint32_t center_x = minimap_width / 2;
    uint32_t center_y = minimap_height / 2;
    uint32_t radius = minimap_width / 2;

    float cos_theta = -cosf(gs->sharedState->player->theta);
    float sin_theta = sinf(gs->sharedState->player->theta);

    for (uint32_t y = 0; y < minimap_height; y++) {
        for (uint32_t x = 0; x < minimap_width; x++) {
            int dx = x - center_x;
            int dy = y - center_y;

            if (dx * dx + dy * dy <= radius * radius) {
                float rotated_x = (cos_theta * dx - sin_theta * dy) + center_x;
                float rotated_y = (sin_theta * dx + cos_theta * dy) + center_y;

                if (rotated_x >= 0 && rotated_x < minimap_width &&
                    rotated_y >= 0 && rotated_y < minimap_height) {
                    uint32_t original_x =
                        (rotated_x * MINIMAP_RANGE) / minimap_width;
                    uint32_t original_y =
                        (rotated_y * MINIMAP_RANGE) / minimap_height;

                    uint32_t color =
                        gs->renderState
                            ->minimap[original_x +
                                      (int)(original_y * MINIMAP_RANGE)];

                    gs->renderState
                        ->image[(start_x + x) + (start_y + y) * SCREEN_WIDTH] =
                        color;
                }
            }
        }
    }
}

void render_ceiling(GameState *gs) {
    RenderState *rs = gs->renderState;
    int top = (int)(SCREEN_HEIGHT / 2 + gs->sharedState->player->eye_z);
    if (top < 0) top = 0;
    uint32_t base = 0xFFEBCE87;
    for (int j = 0; j <= top; ++j) {
        uint32_t colour = darken_colour(base, 1);
        uint32_t *row = &rs->image[j * SCREEN_WIDTH];
        for (int x = 0; x < SCREEN_WIDTH; ++x) row[x] = colour;
    }
}

void render_floors(GameState *gs) {
    RenderState *rs = gs->renderState;
    int top = (int)(SCREEN_HEIGHT / 2 + gs->sharedState->player->eye_z);
    if (top < 0) top = 0;
    uint32_t base = pack_colour(0x18, 0x18, 0x18, 0xFF);
    for (int j = top; j < SCREEN_HEIGHT; ++j) {
        uint32_t colour = darken_colour(base, 1);
        uint32_t *row = &rs->image[j * SCREEN_WIDTH];
        for (int x = 0; x < SCREEN_WIDTH; ++x) row[x] = colour;
    }
}

void draw_scene(GameState *gs) {
    render_ceiling(gs);
    render_floors(gs);

    SharedState *ss = gs->sharedState;
    Player *p = ss->player;
    const float half_hfov = p->hfov * 0.5f;
    const float theta_start = p->theta - half_hfov;
    const float dtheta = p->hfov / (float)SCREEN_WIDTH;
    const size_t walltext_cnt = gs->renderState->walltext_cnt;
    const size_t walltext_size = gs->renderState->walltext_size;

    for (uint32_t i = 0; i < SCREEN_WIDTH; i += 1) {
        float theta = theta_start + i * dtheta;

        float sint = sin(theta);
        float cost = cos(theta);

        for (uint32_t j = NEAR_CLIPPING_PLANE; j <= FAR_CLIPPING_PLANE; j++) {
            float cx = p->pos.x + j * sint;
            float cy = p->pos.y + j * cost;
            if (cx >= WORLD_WIDTH || cy >= WORLD_HEIGHT || cx < 0 || cy < 0)
                break;

            float mx = cx / SCALE, my = cy / SCALE;  // x,y within map

            char id = MAP[(int)mx + (int)my * MAP_WIDTH];
            if (id == ' ')  // floor
                continue;

            size_t textid = id - '0';

            float column_height =
                WORLD_HEIGHT * 20 / (j * cos(theta - p->theta));
            // 20 is the scaling factor for walls

            if (textid >= walltext_cnt) {
                fprintf(stderr,
                        "ERROR: missing/incorrect texture %zu, setting to 0, "
                        "for position {%d, %d}\n",
                        textid, (int)mx, (int)my);
                textid = 0;
            }

            int ty = 0, tx = 0;  // x,y within a particular texture

            float hitx = mx - floor(mx + 0.5);
            float hity = my - floor(my + 0.5);
            float shade_factor_base = 1.0f - (j / (float)FAR_CLIPPING_PLANE);
            if (shade_factor_base < 0.2f) shade_factor_base = 0.2f;

            if (fabsf(hity) > fabsf(hitx)) {
                tx = hity * walltext_size;
                shade_factor_base *= 0.8;
            } else {
                tx = hitx * walltext_size;
            }
            if (tx < 0) tx += walltext_size;

            for (size_t y = 0; y < column_height; y++) {
                ty = y * walltext_size / column_height;
                uint32_t colour =
                    gs->renderState
                        ->walltext[(tx + textid * walltext_size) +
                                   ty * (walltext_size * walltext_cnt)];
                colour = darken_colour(colour, 1);

                PUTPIX(gs->renderState->image,
                       (Vector2i){SCREEN_WIDTH - i, SCREEN_HEIGHT / 2.0f -
                                                        column_height / 2 +
                                                        p->eye_z + y},
                       colour);
            }

            break;
        }
    }
}
