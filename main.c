#include "player.h"
#include "textures.h"
#include "utils.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024

#define WORLD_WIDTH 1024
#define WORLD_HEIGHT 1024

#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define MAP                                                                                                            \
    ("0000222222220000"                                                                                                \
     "1              0"                                                                                                \
     "1      11111   0"                                                                                                \
     "1     0        0"                                                                                                \
     "0     0  1110000"                                                                                                \
     "0     3        0"                                                                                                \
     "0   10000      0"                                                                                                \
     "0   3   11100  0"                                                                                                \
     "5   4   0      0"                                                                                                \
     "5   4   1  00000"                                                                                                \
     "0       1      0"                                                                                                \
     "2       1      0"                                                                                                \
     "0       0      0"                                                                                                \
     "0 0000000      0"                                                                                                \
     "0              0"                                                                                                \
     "0002222222200000")

#define SCALE ((float)WORLD_WIDTH / MAP_WIDTH)
#define MINIMAP_SCALE 1.0f / 4

void draw_rectangle(uint32_t *image, float x, float y, float w, float h, uint32_t colour) {
    for (int i = 0; i < w; i++) {
        size_t cx = x + i;
        for (int j = 0; j < h; j++) {
            size_t cy = y + j;
            if (cx >= SCREEN_WIDTH || cy >= SCREEN_HEIGHT || cx < 0 || cy < 0)
                continue;
            image[cx + cy * SCREEN_WIDTH] = colour;
        }
    }
}

void draw_player_fov(uint32_t *image, Player *player, uint32_t colour) {
    for (size_t i = 0; i < SCREEN_WIDTH; i += 10) {
        float theta = (player->theta - player->hfov / 2) + i * player->hfov / SCREEN_WIDTH;

        for (size_t j = 0; j < SCREEN_HEIGHT; j++) {
            float cx = player->x + j * cos(theta);
            float cy = player->y + j * sin(theta);
            if (cx >= SCREEN_WIDTH || cy >= SCREEN_HEIGHT || cx < 0 || cy < 0)
                continue;
            if (MAP[(int)(cx / SCALE) + (int)(cy / SCALE) * MAP_WIDTH] != ' ')
                break;

            image[(int)cx + (int)cy * SCREEN_WIDTH] = colour;
        }
    }
}

void draw_minimap(uint32_t *image, uint32_t *minimap) {
    uint32_t minimap_width = SCREEN_WIDTH * MINIMAP_SCALE;
    uint32_t minimap_height = SCREEN_HEIGHT * MINIMAP_SCALE;
    uint32_t start_x = SCREEN_WIDTH - minimap_width;
    uint32_t start_y = SCREEN_HEIGHT - minimap_height;

    for (uint32_t y = 0; y < minimap_height; y++) {
        for (uint32_t x = 0; x < minimap_width; x++) {
            uint32_t original_x = x * SCREEN_WIDTH / minimap_width;
            uint32_t original_y = y * SCREEN_HEIGHT / minimap_height;

            image[(start_y + y) * SCREEN_WIDTH + (start_x + x)] = minimap[original_y * SCREEN_WIDTH + original_x];
        }
    }
}

void draw_scene(uint32_t *image, Player *player, uint32_t *walltext, size_t walltext_size, size_t walltext_cnt) {

    for (size_t i = 0; i < SCREEN_WIDTH; i++) {
        float theta = (player->theta - player->hfov / 2) + i * player->hfov / SCREEN_WIDTH;

        for (float j = 1; j < SCREEN_HEIGHT; j++) {
            float cx = player->x + j * cos(theta);
            float cy = player->y + j * sin(theta);
            if (!(cx < SCREEN_WIDTH && cy < SCREEN_WIDTH))
                continue;

            float mx = cx / SCALE, my = cy / SCALE; // x,y within map
            char id = MAP[(int)mx + (int)my * MAP_WIDTH];
            if (id == ' ')
                continue;

            int textid = id - '0';
            assert(textid >= 0 && textid < walltext_cnt);

            int tx = 0, ty = 0; // x,y within a particular texture

            float hitx = mx - floor(mx + 0.5);
            float hity = my - floor(my + 0.5);

            ty = hitx * walltext_size;
            if (fabsf(hity) > fabsf(hitx)) {
                ty = hity * walltext_size;
            }
            if (ty < 0)
                ty += walltext_size;
            assert(ty >= 0 && ty < (int)walltext_size);

            uint column_height =
                (uint)SCREEN_HEIGHT * 50 / (j * cos(theta - player->theta)); // 50 is the scaling factor for walls
            for (size_t x = 0; x < column_height; x++) {
                tx = x * walltext_size / column_height;
                uint32_t colour = walltext[(ty + textid * walltext_size) + tx * (walltext_size * walltext_cnt)];

                draw_rectangle(image, i, SCREEN_HEIGHT / 2.0f - column_height / 2 + player->eye_z + x, 1 / SCALE, 1,
                               colour);
            }

            break;
        }
    }
}

int main() {
    assert(WORLD_WIDTH % MAP_WIDTH == 0);
    assert(WORLD_HEIGHT % MAP_HEIGHT == 0);
    assert(sizeof(MAP) == MAP_WIDTH * MAP_HEIGHT + 1);

    // const char *scene_file = "scene.ppm";
    const char *map_file = "map.ppm";

    uint32_t *image = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    uint32_t *minimap = (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
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
        .x = 200,
        .y = 200,
        .theta = PI_2,
        .eye_z = 0.0f,
        .hfov = DEG2RAD(90.0f),
        .vfov = 0.5f,
        .velocity = {.x = 0, .y = 0},
        .rotate_speed = 0.1,
        .speed = 10,
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

    // for (int i = 0; i < 10; i++) {
    //     float dt = 2.0f * 10 / 60;
    //     for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
    //         image[i] = 0xFFFFFFFF;
    //         minimap[i] = 0xFFAAAAAA;
    //     }
    //     rotate_player(&player, dt, 1);
    //     move_player(&player, dt, 1);
    //
    //     char scene_file[50];
    //     sprintf(scene_file, "scene%d.ppm", i);
    //
    //     // Draw minimap
    //     for (int i = 0; i < MAP_HEIGHT; i++) {
    //         for (int j = 0; j < MAP_WIDTH; j++) {
    //
    //             if (MAP[i + j * MAP_WIDTH] != ' ') {
    //
    //                 draw_rectangle(minimap, i * SCALE, j * SCALE, 1 * SCALE, 1 * SCALE, pack_colour(25, 25, 25,
    //                 255));
    //             }
    //         }
    //     }
    //     draw_rectangle(minimap, player.x, player.y, 10, 10, pack_colour(255, 0, 0, 255));
    //     draw_player_fov(minimap, &player, pack_colour(255, 255, 255, 255));
    //     // dump_ppm(map_file, minimap, SCREEN_WIDTH, SCREEN_HEIGHT);
    //
    //     // Draw scene
    //     draw_scene(image, &player, walltext, walltext_size, walltext_cnt);
    //
    //     // Draw minimap at bottom corner of size MINIMAP_SCALE
    //     draw_minimap(image, minimap);
    //     dump_ppm(scene_file, image, SCREEN_WIDTH, SCREEN_HEIGHT);
    // }

    bool quit = false;
    // uint64_t NOW = SDL_GetPerformanceCounter();
    // uint64_t LAST = 0;
    double delta_time = 0;

    while (!quit) {
        // delta_time = (double)((NOW - LAST) * 1 / (double)SDL_GetPerformanceFrequency());
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
                // if ('a' == ev.key.keysym.sym)
                //     rotate_player(&player, delta_time, -1);
                // if ('d' == ev.key.keysym.sym)
                //     rotate_player(&player, delta_time, 1);
                // if ('s' == ev.key.keysym.sym)
                //     move_player(&player, delta_time, -1);
                // if ('w' == ev.key.keysym.sym)
                //     move_player(&player, delta_time, 1);
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
            rotate_player(&player, delta_time, -1); // Rotate left
        }

        if (keystate[SDL_SCANCODE_D]) {
            rotate_player(&player, delta_time, 1); // Rotate right
        }

        for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            image[i] = 0xFFFFFFFF;
            minimap[i] = 0xFFAAAAAA;
        }
        draw_scene(image, &player, walltext, walltext_size, walltext_cnt);
        SDL_UpdateTexture(texture, NULL, (void *)image, SCREEN_WIDTH * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    return 0;
}
