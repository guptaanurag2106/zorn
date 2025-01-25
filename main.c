#include "constants.h"
#include "player.h"
#include "textures.h"
#include "utils.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum GameLoadState { LOADING, PLAYING, QUIT };

typedef struct GameState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *image;
    uint32_t *minimap;
    Player *player;
    uint32_t *walltext;
    size_t walltext_size;
    size_t walltext_cnt;
    enum GameLoadState game_load_state;
} GameState;

void draw_rectangle(uint32_t *image, Vector2 pos, float w, float h, uint32_t image_w, uint32_t image_h,
                    uint32_t colour) {
    for (int i = 0; i < w; i++) {
        size_t cx = pos.x + i;
        for (int j = 0; j < h; j++) {
            size_t cy = pos.y + j;
            if (cx >= image_w || cy >= image_h || cx < 0 || cy < 0)
                continue;
            image[cx + cy * image_w] = colour;
        }
    }
}

void draw_player_fov(uint32_t *minimap, Player *player, uint32_t colour) {
    for (size_t i = 0; i < WORLD_WIDTH; i += 100) {
        float theta = (player->theta - player->hfov / 2) + i * player->hfov / WORLD_WIDTH;

        for (size_t j = 0; j < 80; j++) {
            float cx = player->pos.x + j * sin(theta);
            float cy = player->pos.y + j * cos(theta);
            if (cx >= WORLD_WIDTH || cy >= WORLD_HEIGHT || cx < 0 || cy < 0 ||
                fabsf(cx - player->pos.x) >= MINIMAP_RANGE / 2 || fabsf(cy - player->pos.y) >= MINIMAP_RANGE / 2)
                continue;
            if (MAP[(int)(cx / SCALE) + (int)(cy / SCALE) * MAP_WIDTH] != ' ')
                break;

            minimap[(int)(cx - player->pos.x + MINIMAP_RANGE / 2) +
                    (int)(cy - player->pos.y + MINIMAP_RANGE / 2) * MINIMAP_RANGE] = colour;
        }
    }
}

void draw_minimap(GameState *gs) {
    for (int i = 0; i < MINIMAP_RANGE; i++) {
        for (int j = 0; j < MINIMAP_RANGE; j++) {
            int cx = ((gs->player->pos.x - MINIMAP_RANGE / 2) + i) / SCALE;
            int cy = ((gs->player->pos.y - MINIMAP_RANGE / 2) + j) / SCALE;
            if (cx >= MAP_WIDTH || cy >= MAP_HEIGHT || cx < 0 || cy < 0)
                continue;

            if (MAP[cx + cy * MAP_WIDTH] != ' ') {

                draw_rectangle(gs->minimap, (Vector2){i, j}, 1 / SCALE, 1 / SCALE, MINIMAP_RANGE, MINIMAP_RANGE,
                               pack_colour(25, 25, 25, 255));
            }
        }
    }
    draw_rectangle(gs->minimap, (Vector2){MINIMAP_RANGE / 2 - 5, MINIMAP_RANGE / 2 - 5}, 10, 10, MINIMAP_RANGE,
                   MINIMAP_RANGE, gs->player->colour);
    // draw_player_fov(gs->minimap, gs->player, pack_colour(255, 255, 255, 255));
}

void overlay_minimap(GameState *gs) {
    uint32_t minimap_width = SCREEN_WIDTH * MINIMAP_SCALE;
    uint32_t minimap_height = SCREEN_HEIGHT * MINIMAP_SCALE;

    uint32_t start_x = SCREEN_WIDTH - minimap_width;
    uint32_t start_y = SCREEN_HEIGHT - minimap_height;

    uint32_t center_x = minimap_width / 2;
    uint32_t center_y = minimap_height / 2;
    uint32_t radius = minimap_width / 2;

    float cos_theta = -cosf(gs->player->theta);
    float sin_theta = sinf(gs->player->theta);

    for (uint32_t y = 0; y < minimap_height; y++) {
        for (uint32_t x = 0; x < minimap_width; x++) {
            int dx = x - center_x;
            int dy = y - center_y;

            if (dx * dx + dy * dy <= radius * radius) {
                int rotated_x = (int)(cos_theta * dx - sin_theta * dy) + center_x;
                int rotated_y = (int)(sin_theta * dx + cos_theta * dy) + center_y;

                if (rotated_x >= 0 && rotated_x < minimap_width && rotated_y >= 0 && rotated_y < minimap_height) {
                    uint32_t original_x = (rotated_x * MINIMAP_RANGE) / minimap_width;
                    uint32_t original_y = (rotated_y * MINIMAP_RANGE) / minimap_height;

                    uint32_t color = gs->minimap[original_x + original_y * MINIMAP_RANGE];

                    gs->image[(start_x + x) + (start_y + y) * SCREEN_WIDTH] = color;
                }
            }
        }
    }
}

void handle_wall_collision(GameState *gs, Vector2 old_pos) {
    if (gs->player->pos.x - PLAYER_SIZE < 0 || gs->player->pos.x + PLAYER_SIZE >= WORLD_WIDTH) {
        gs->player->pos.x = old_pos.x; // Revert to old position
    }

    if (gs->player->pos.y - PLAYER_SIZE < 0 || gs->player->pos.y + PLAYER_SIZE >= WORLD_HEIGHT) {
        gs->player->pos.y = old_pos.y; // Revert to old position
    }

    int old_mx = old_pos.x / SCALE;
    int old_my = old_pos.y / SCALE;

    for (int i = -PLAYER_SIZE / 2; i <= PLAYER_SIZE / 2; i++) {
        int mx = (gs->player->pos.x + i) / SCALE;
        int my = gs->player->pos.y / SCALE;
        if (MAP[mx + old_my * MAP_WIDTH] != ' ') {
            gs->player->pos.x = old_pos.x; // Revert to old position if there's a collision
            gs->player->pos.y = old_pos.y + (gs->player->pos.y - old_pos.y) * 0.4;
            break;
        }
    }

    for (int i = -PLAYER_SIZE / 2; i <= PLAYER_SIZE / 2; i++) {
        int mx = gs->player->pos.x / SCALE;
        int my = (gs->player->pos.y + i) / SCALE;
        if (MAP[old_mx + my * MAP_WIDTH] != ' ') {
            gs->player->pos.y = old_pos.y; // Revert to old position if there's a collision
            gs->player->pos.x = old_pos.x + (gs->player->pos.x - old_pos.x) * 0.4;
            break;
        }
    }
}

// TODO
// void render_ceiling(GameState *gs) {
// }

// TODO
// void render_floors(GameState *gs) {
// }

void draw_scene(GameState *gs) {
    // render_ceiling(gs);
    // render_floors(gs);

    for (size_t i = 0; i < WORLD_WIDTH; i++) {
        float theta = (gs->player->theta - gs->player->hfov / 2) + i * gs->player->hfov / WORLD_WIDTH;

        for (float j = NEAR_CLIPPING_PLANE; j <= FAR_CLIPPING_PLANE; j++) {
            float cx = gs->player->pos.x + j * sin(theta);
            float cy = gs->player->pos.y + j * cos(theta);
            if (cx >= WORLD_WIDTH || cy >= WORLD_HEIGHT || cx < 0 || cy < 0)
                continue;

            float mx = cx / SCALE, my = cy / SCALE; // x,y within map

            char id = MAP[(int)mx + (int)my * MAP_WIDTH];
            if (id == ' ') // floor
                continue;

            int textid = id - '0';

            uint column_height =
                (uint)WORLD_HEIGHT * 50 / (j * cos(theta - gs->player->theta)); // 50 is the scaling factor for walls

            for (size_t y = (SCREEN_HEIGHT / 2.0f + column_height / 2 + gs->player->eye_z); y < SCREEN_HEIGHT; y++) {
                uint32_t colour = pack_colour(0x18, 0x18, 0x18, 0xFF);

                draw_rectangle(gs->image, (Vector2){SCREEN_WIDTH - i, y}, 1 / SCALE, 1, SCREEN_WIDTH, SCREEN_HEIGHT,
                               colour);
            }

            if (!(textid >= 0 && textid < gs->walltext_cnt))
                break;

            int ty = 0, tx = 0; // x,y within a particular texture

            float hitx = mx - floor(mx + 0.5);
            float hity = my - floor(my + 0.5);

            tx = hitx * gs->walltext_size;
            if (fabsf(hity) > fabsf(hitx)) {
                tx = hity * gs->walltext_size;
            }
            if (tx < 0)
                tx += gs->walltext_size;

            for (size_t y = 0; y < column_height; y++) {
                ty = y * gs->walltext_size / column_height;
                uint32_t colour =
                    gs->walltext[(tx + textid * gs->walltext_size) + ty * (gs->walltext_size * gs->walltext_cnt)];

                draw_rectangle(
                    gs->image,
                    (Vector2){SCREEN_WIDTH - i, SCREEN_HEIGHT / 2.0f - column_height / 2 + gs->player->eye_z + y},
                    1 / SCALE, 1, SCREEN_WIDTH, SCREEN_HEIGHT, colour);
            }

            break;
        }
    }
}

void delete_state(GameState *gs) {
    free(gs->image);
    free(gs->minimap);
    free(gs->walltext);
    SDL_DestroyTexture(gs->texture);
    SDL_DestroyRenderer(gs->renderer);
    SDL_DestroyWindow(gs->window);
    SDL_Quit();
}

bool init_state(GameState *gs) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize SDL2: %s\n", SDL_GetError());
        return 1;
    }

    gs->window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                                  SDL_WINDOW_ALLOW_HIGHDPI);
    if (!gs->window) {
        fprintf(stderr, "ERROR: Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    gs->renderer = SDL_CreateRenderer(gs->window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!gs->renderer) {
        fprintf(stderr, "ERROR: Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(gs->window);
        SDL_Quit();
        return 1;
    }

    gs->texture = SDL_CreateTexture(gs->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                                    SCREEN_HEIGHT);
    if (!gs->texture) {
        fprintf(stderr, "ERROR: Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyWindow(gs->window);
        SDL_Quit();
        return 1;
    }

    SDL_UpdateTexture(gs->texture, NULL, (void *)(gs->image), SCREEN_WIDTH * 4);
    SDL_RenderCopy(gs->renderer, gs->texture, NULL, NULL);
    SDL_RenderPresent(gs->renderer);

    gs->image = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    gs->minimap = (uint32_t *)malloc(MINIMAP_RANGE * MINIMAP_RANGE * sizeof(uint32_t));

    if (gs->image == NULL) {
        fprintf(stderr, "Memory allocation for image failed!\n");
        return 1;
    }
    if (gs->minimap == NULL) {
        fprintf(stderr, "Memory allocation for minimap failed!\n");
        return 1;
    }

    if (!load_texture("assets/walltext.png", &(gs->walltext), &(gs->walltext_size), &(gs->walltext_cnt))) {
        return -1;
    }

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            delete_state(gs);
            break;
        case SDL_KEYUP:
            break;
        case SDL_KEYDOWN:
            break;
        }
    }

    SDL_Delay(500);

    return true;
}

int main() {
    assert(sizeof(MAP) == MAP_WIDTH * MAP_HEIGHT + 1);

    Player player = {
        .id = 1,
        .pos = {320, 420}, // in world coordinates
        .theta = 0,        // 0 mean facing along y
        .eye_z = SCREEN_HEIGHT / 10.0f,
        .hfov = DEG2RAD(90.0f),
        .vfov = 0.5f,
        .velocity = {.x = 0, .y = 0, .z = 0},
        .rotate_speed = DEG2RAD(5),
        .speed = WORLD_HEIGHT / 120,
        .vert_speed = SCREEN_HEIGHT / 10.0f,
        .is_jumping = false,
        .colour = pack_colour(255, 0, 0, 255),
    };
    player.velocity.y = player.speed;

    GameState gs = {
        .window = NULL,
        .renderer = NULL,
        .texture = NULL,
        .image = NULL,
        .minimap = NULL,
        .player = &player,
        .walltext = NULL,
        .game_load_state = LOADING,
    };

    uint32_t start_time = 0;
    while (!(gs.game_load_state == QUIT)) {
        if (gs.game_load_state == LOADING) {
            if (init_state(&gs))
                gs.game_load_state = PLAYING;
            SDL_RenderClear(gs.renderer);
            start_time = SDL_GetTicks64();
        }

        double delta_time = (SDL_GetTicks64() - start_time) / 75.0f;
        start_time = SDL_GetTicks64();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                gs.game_load_state = QUIT;
                break;
            case SDL_KEYUP:
                break;
            case SDL_KEYDOWN:
                break;
            }
        }

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        const Vector2 old_pos = gs.player->pos;

        if (keystate[SDL_SCANCODE_W]) {
            move_player(gs.player, delta_time, 1); // Move forward
        }

        if (keystate[SDL_SCANCODE_S]) {
            move_player(gs.player, delta_time, -1); // Move backward
        }

        if (keystate[SDL_SCANCODE_A]) {
            rotate_player(gs.player, delta_time, 1); // Rotate left
        }

        if (keystate[SDL_SCANCODE_D]) {
            rotate_player(gs.player, delta_time, -1); // Rotate right
        }

        if (keystate[SDL_SCANCODE_SPACE]) {
            jump_player(gs.player, delta_time);
        }

        player_gravity(gs.player, delta_time);

        handle_wall_collision(&gs, old_pos);

        for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            (gs.image)[i] = 0xFFFFFFB2;
            if (i < MINIMAP_RANGE * MINIMAP_RANGE)
                (gs.minimap)[i] = 0xFFAAAAAA;
        }
        draw_minimap(&gs);

        draw_scene(&gs);
        overlay_minimap(&gs);

        SDL_UpdateTexture(gs.texture, NULL, (void *)(gs.image), SCREEN_WIDTH * 4);
        SDL_RenderCopy(gs.renderer, gs.texture, NULL, NULL);
        SDL_RenderPresent(gs.renderer);
    }

    delete_state(&gs);

    return 0;
}
