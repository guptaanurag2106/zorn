#include "constants.h"
#include "player.h"
#include "textures.h"
#include "utils.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void draw_rectangle(uint32_t *image, float x, float y, float w, float h, uint32_t image_w, uint32_t image_h,
                    uint32_t colour) {
    for (int i = 0; i < w; i++) {
        size_t cx = x + i;
        for (int j = 0; j < h; j++) {
            size_t cy = y + j;
            if (cx >= image_w || cy >= image_h || cx < 0 || cy < 0)
                continue;
            image[cx + cy * image_w] = colour;
        }
    }
}

void draw_player_fov(uint32_t *minimap, Player *player, uint32_t colour) {
    for (size_t i = 0; i < SCREEN_WIDTH; i += 100) {
        float theta = (player->theta - player->hfov / 2) + i * player->hfov / SCREEN_WIDTH;

        for (size_t j = 0; j < 80; j++) {
            float cx = player->x + j * sin(theta);
            float cy = player->y + j * cos(theta);
            if (cx >= SCREEN_WIDTH || cy >= SCREEN_HEIGHT || cx < 0 || cy < 0 ||
                fabsf(cx - player->x) >= MINIMAP_RANGE / 2 || fabsf(cy - player->y) >= MINIMAP_RANGE / 2)
                continue;
            if (MAP[(int)(cx / SCALE) + (int)(cy / SCALE) * MAP_WIDTH] != ' ')
                break;

            minimap[(int)(cx - player->x + MINIMAP_RANGE / 2) +
                    (int)(cy - player->y + MINIMAP_RANGE / 2) * MINIMAP_RANGE] = colour;
        }
    }
}

void draw_minimap(uint32_t *minimap, Player *player) {
    for (int i = 0; i < MINIMAP_RANGE; i++) {
        for (int j = 0; j < MINIMAP_RANGE; j++) {
            int cx = ((player->x - MINIMAP_RANGE / 2) + i) / SCALE;
            int cy = ((player->y - MINIMAP_RANGE / 2) + j) / SCALE;
            if (cx >= MAP_WIDTH || cy >= MAP_HEIGHT || cx < 0 || cy < 0)
                continue;

            if (MAP[cx + cy * MAP_WIDTH] != ' ') {

                draw_rectangle(minimap, i, j, 1 / SCALE, 1 / SCALE, MINIMAP_RANGE, MINIMAP_RANGE,
                               pack_colour(25, 25, 25, 255));
            }
        }
    }
    draw_rectangle(minimap, MINIMAP_RANGE / 2 - 5, MINIMAP_RANGE / 2 - 5, 10, 10, MINIMAP_RANGE, MINIMAP_RANGE,
                   player->colour);
    draw_player_fov(minimap, player, pack_colour(255, 255, 255, 255));
}

void overlay_minimap(uint32_t *image, uint32_t *minimap, float theta) {
    uint32_t minimap_width = SCREEN_WIDTH * MINIMAP_SCALE;
    uint32_t minimap_height = SCREEN_HEIGHT * MINIMAP_SCALE;

    uint32_t start_x = SCREEN_WIDTH - minimap_width;
    uint32_t start_y = SCREEN_HEIGHT - minimap_height;

    uint32_t center_x = minimap_width / 2;
    uint32_t center_y = minimap_height / 2;
    uint32_t radius = minimap_width / 2;

    float cos_theta = -cosf(theta);
    float sin_theta = sinf(theta);

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

                    uint32_t color = minimap[original_x + original_y * MINIMAP_RANGE];

                    image[(start_x + x) + (start_y + y) * SCREEN_WIDTH] = color;
                }
            }
        }
    }
}

void handle_wall_collision(Player *player) {}

void draw_scene(uint32_t *image, Player *player, uint32_t *walltext, size_t walltext_size, size_t walltext_cnt) {

    for (size_t i = 0; i < SCREEN_WIDTH; i++) {
        float theta = (player->theta - player->hfov / 2) + i * player->hfov / SCREEN_WIDTH;

        for (float j = 1; j < SCREEN_HEIGHT; j++) {
            float cx = player->x + j * sin(theta);
            float cy = player->y + j * cos(theta);
            if (!(cx < SCREEN_WIDTH && cy < SCREEN_HEIGHT))
                continue;

            float mx = cx / SCALE, my = cy / SCALE; // x,y within map
            char id = MAP[(int)mx + (int)my * MAP_WIDTH];
            if (id == ' ')
                continue;

            int textid = id - '0';
            assert(textid >= 0 && textid < walltext_cnt);

            int ty = 0, tx = 0; // x,y within a particular texture

            float hitx = mx - floor(mx + 0.5);
            float hity = my - floor(my + 0.5);

            tx = hitx * walltext_size;
            if (fabsf(hity) > fabsf(hitx)) {
                tx = hity * walltext_size;
            }
            if (tx < 0)
                tx += walltext_size;
            assert(tx >= 0 && tx < (int)walltext_size);

            uint column_height =
                (uint)SCREEN_HEIGHT * 50 / (j * cos(theta - player->theta)); // 50 is the scaling factor for walls
            for (size_t y = 0; y < column_height; y++) {
                ty = y * walltext_size / column_height;
                uint32_t colour = walltext[(tx + textid * walltext_size) + ty * (walltext_size * walltext_cnt)];

                draw_rectangle(image, SCREEN_WIDTH - i, SCREEN_HEIGHT / 2.0f - column_height / 2 + player->eye_z + y,
                               1 / SCALE, 1, SCREEN_WIDTH, SCREEN_HEIGHT, colour);
            }
            for (size_t y = (SCREEN_HEIGHT / 2.0f + column_height / 2 + player->eye_z); y < SCREEN_HEIGHT; y++) {
                uint32_t colour = pack_colour(0x18, 0x18, 0x18, 0xFF);

                draw_rectangle(image, SCREEN_WIDTH - i, y, 1 / SCALE, 1, SCREEN_WIDTH, SCREEN_HEIGHT, colour);
            }

            break;
        }
    }
}

int main() {
    assert(WORLD_WIDTH % MAP_WIDTH == 0);
    assert(WORLD_HEIGHT % MAP_HEIGHT == 0);
    assert(sizeof(MAP) == MAP_WIDTH * MAP_HEIGHT + 1);

    uint32_t *image = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    uint32_t *minimap = (uint32_t *)malloc(MINIMAP_RANGE * MINIMAP_RANGE * sizeof(uint32_t));
    if (image == NULL) {
        fprintf(stderr, "Memory allocation for image failed!\n");
        return 1;
    }
    if (minimap == NULL) {
        fprintf(stderr, "Memory allocation for minimap failed!\n");
        return 1;
    }

    uint32_t *walltext;
    size_t walltext_size; // each texture is a square
    size_t walltext_cnt;
    if (!load_texture("./walltext.png", &walltext, &walltext_size, &walltext_cnt)) {
        return -1;
    }

    Player player = {
        .x = 300,
        .y = 400,
        .theta = 0, // 0 mean facing along y
        .eye_z = 100.0f,
        .hfov = DEG2RAD(90.0f),
        .vfov = 0.5f,
        .velocity = {.x = 0, .y = 0, .z = 0},
        .rotate_speed = 0.1,
        .speed = 10,
        .vert_speed = 100.0f,
        .is_jumping = false,
        .colour = pack_colour(255, 0, 0, 255),
    };
    player.velocity.y = player.speed;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize SDL2: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        fprintf(stderr, "ERROR: Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "ERROR: Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!texture) {
        fprintf(stderr, "ERROR: Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    double delta_time = 0;

    while (!quit) {
        delta_time = 0.2;
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYUP:
                break;
            case SDL_KEYDOWN:
                break;
            }
        }

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_W]) {
            move_player(&player, delta_time, 1); // Move forward
        }

        if (keystate[SDL_SCANCODE_S]) {
            move_player(&player, delta_time, -1); // Move backward
        }

        if (keystate[SDL_SCANCODE_A]) {
            rotate_player(&player, delta_time, 1); // Rotate left
        }

        if (keystate[SDL_SCANCODE_D]) {
            rotate_player(&player, delta_time, -1); // Rotate right
        }

        if (keystate[SDL_SCANCODE_SPACE]) {
            jump_player(&player, delta_time);
        }

        player_gravity(&player, delta_time);

        handle_wall_collision(&player);

        for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            image[i] = 0xFFFFFFB2;
            if (i < MINIMAP_RANGE * MINIMAP_RANGE)
                minimap[i] = 0xFFAAAAAA;
        }

        draw_minimap(minimap, &player);

        // Draw minimap at bottom corner of size MINIMAP_SCALE
        draw_scene(image, &player, walltext, walltext_size, walltext_cnt);
        overlay_minimap(image, minimap, player.theta);

        SDL_UpdateTexture(texture, NULL, (void *)image, SCREEN_WIDTH * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    free(image);
    free(minimap);
    free(walltext);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
