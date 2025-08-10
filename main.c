#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "constants.h"
#include "game_state.h"
#include "net.h"
#include "player.h"
#include "render.h"
#include "textures.h"
#include "utilities.h"

#define UTILS_IMPLEMENTATION
#include "common/utils.h"

void handle_wall_collision(SharedState *ss, Vector2 old_pos) {
    bool moved = false;
    if (ss->player->pos.x - PLAYER_SIZE / 2 < 0 ||
        ss->player->pos.x + PLAYER_SIZE / 2 >= WORLD_WIDTH) {
        ss->player->pos.x = old_pos.x;  // Revert to old position
        moved = true;
    }

    if (ss->player->pos.y - PLAYER_SIZE / 2 < 0 ||
        ss->player->pos.y + PLAYER_SIZE / 2 >= WORLD_HEIGHT) {
        ss->player->pos.y = old_pos.y;  // Revert to old position
    } else
        moved = false;
    if (moved) return;

    size_t old_mx = old_pos.x / SCALE;
    size_t old_my = old_pos.y / SCALE;

    for (float i = -PLAYER_SIZE / 2; i <= PLAYER_SIZE / 2; i++) {
        size_t mx = (ss->player->pos.x + i) / SCALE;
        if (MAP[mx + old_my * MAP_WIDTH] != ' ') {
            ss->player->pos.x =
                old_pos.x;  // Revert to old position if there's a collision
            ss->player->pos.y = old_pos.y + (ss->player->pos.y - old_pos.y) *
                                                0.8;  // slowly glide along wall
            break;
        }
    }

    for (float i = -PLAYER_SIZE / 2; i <= PLAYER_SIZE / 2; i++) {
        size_t my = (ss->player->pos.y + i) / SCALE;
        if (MAP[old_mx + my * MAP_WIDTH] != ' ') {
            ss->player->pos.y =
                old_pos.y;  // Revert to old position if there's a collision
            ss->player->pos.x =
                old_pos.x + (ss->player->pos.x - old_pos.x) * 0.8;
            break;
        }
    }
}

void delete_state(GameState *gs) {
    SDL_DestroyTexture(gs->renderState->texture);
    SDL_DestroyRenderer(gs->renderState->renderer);
    SDL_DestroyWindow(gs->renderState->window);

    free(gs->renderState->image);
    free(gs->renderState->minimap);
    free(gs->renderState->walltext);
    free(gs->renderState);
    for (size_t i = 0; i < gs->text_count; i++) {
        free(gs->texts[i].content);
    }
    free(gs->texts);
    free(gs->sharedState->player->id);
    for (size_t i = 0; i < gs->sharedState->entity_count; i++) {
        free(gs->sharedState->entities[i]);
    }
    free(gs->sharedState->entities);
    pthread_mutex_destroy(&gs->sharedState->lock);
    free(gs->sharedState);

    SDL_Quit();
}

void get_player_random_init(Player *player) {
    int start_x, start_y;
    bool found = false;

    start_x = rand() % SCREEN_WIDTH;
    start_y = rand() % SCREEN_HEIGHT;

    for (int radius = 0; radius < SCREEN_WIDTH; radius++) {
        for (int dx = -radius; dx <= radius; dx++) {
            for (int dy = -radius; dy <= radius; dy++) {
                int new_x = start_x + dx;
                int new_y = start_y + dy;
                if (new_x > PLAYER_SIZE &&
                    new_x < (SCREEN_WIDTH - PLAYER_SIZE) &&
                    new_y > PLAYER_SIZE &&
                    new_y < (SCREEN_HEIGHT - PLAYER_SIZE)) {
                    if (MAP[(int)(new_x / SCALE) +
                            (int)(new_y / SCALE) * MAP_WIDTH] == ' ') {
                        player->pos.x = new_x;
                        player->pos.y = new_y;
                        found = true;
                        break;
                    }
                }
            }
            if (found) break;
        }
        if (found) break;
    }

    if (!found) {
        fprintf(stderr, "ERROR: No valid spawn position found! Exiting\n");
        exit(1);
    }

    player->theta = 0;
}

bool init_state(GameState *gs) {
    if (gs->game_load_state != INITIAL) return true;
    // srand((unsigned int)time(NULL));
    srand(0);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "ERROR: Failed to initialize SDL2: %s\n",
                SDL_GetError());
        return false;
    }

    gs->renderState->window =
        SDL_CreateWindow("Zorn", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    if (!gs->renderState->window) {
        fprintf(stderr, "ERROR: Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    gs->renderState->renderer = SDL_CreateRenderer(gs->renderState->window, -1,
                                                   SDL_RENDERER_PRESENTVSYNC);
    if (!gs->renderState->renderer) {
        fprintf(stderr, "ERROR: Failed to create renderer: %s\n",
                SDL_GetError());
        SDL_DestroyWindow(gs->renderState->window);
        SDL_Quit();
        return false;
    }

    gs->renderState->texture = SDL_CreateTexture(
        gs->renderState->renderer, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!gs->renderState->texture) {
        fprintf(stderr, "ERROR: Failed to create texture: %s\n",
                SDL_GetError());
        SDL_DestroyWindow(gs->renderState->window);
        SDL_Quit();
        return false;
    }

    // SDL_UpdateTexture(gs->renderState->texture, NULL,
    //                   (void *)(gs->renderState->image), SCREEN_WIDTH * 4);
    // SDL_RenderCopy(gs->renderState->renderer, gs->renderState->texture, NULL,
    //                NULL);
    // SDL_RenderPresent(gs->renderState->renderer);

    gs->renderState->image =
        (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    gs->renderState->minimap =
        (uint32_t *)malloc(MINIMAP_RANGE * MINIMAP_RANGE * sizeof(uint32_t));

    if (gs->renderState->image == NULL) {
        delete_state(gs);
        fprintf(stderr, "Memory allocation for image failed!\n");
        return false;
    }
    if (gs->renderState->minimap == NULL) {
        delete_state(gs);
        fprintf(stderr, "Memory allocation for minimap failed!\n");
        return false;
    }

    if (!load_texture("assets/walltext.png", &(gs->renderState->walltext),
                      &(gs->renderState->walltext_size),
                      &(gs->renderState->walltext_cnt))) {
        delete_state(gs);
        return false;
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "ERROR: Could not initialize SDL_Font\n");
        delete_state(gs);
        return false;
    }
    gs->texts = NULL;
    char *font_file =
        "/usr/share/fonts/TTF/InconsolataNerdFontMono-Regular.ttf";
    TTF_Font *fps_font = TTF_OpenFont(font_file, 32);
    if (!fps_font) {
        fprintf(stderr, "ERROR: Could not open font file: %s %s\n", font_file,
                TTF_GetError());
        delete_state(gs);
        return false;
    }
    SDL_Color fps_textColour = {50, 50, 50, 255};
    SDL_Rect fps_textRect = {SCREEN_WIDTH - 200, 0, 200, 32};

    gs->texts = realloc(gs->texts, sizeof(Text) * (gs->text_count + 1));
    Text *newText = &gs->texts[gs->text_count];
    newText->type = FPS;
    newText->font = fps_font;
    newText->colour = fps_textColour;
    newText->position = fps_textRect;
    gs->text_count++;
    gs->texts[0].content = malloc(sizeof(char) * 15);

    get_player_random_init(gs->sharedState->player);

    pthread_mutex_init(&gs->sharedState->lock, NULL);
    return true;
}

int main() {
    assert(sizeof(MAP) == MAP_WIDTH * MAP_HEIGHT + 1);

    Player player = {
        .id = get_uuid(),
        .pos = {0, 0},  // in world coordinates
        .theta = 0,     // 0 mean facing along y
        .eye_z = SCREEN_HEIGHT / 10.0f,
        .hfov = DEG2RAD(90.0f),
        .vfov = 0.5f,
        .velocity = {.x = 0, .y = 0, .z = 0},
        .rotate_speed = PLAYER_SNAP_ROTATION,
        .speed = PLAYER_SPEED,
        .vert_speed = PLAYER_VERT_SPEED,
        .is_jumping = false,
        .colour = pack_colour(255, 0, 0, 255),
    };
    player.velocity.y = player.speed;

    RenderState *rs = (RenderState *)malloc(sizeof(RenderState));
    if (rs == NULL) {
        fprintf(stderr, "ERROR: Could not malloc RenderState\n");
        exit(1);
    }
    rs->window = NULL;
    rs->renderer = NULL;
    rs->texture = NULL;
    rs->image = NULL;
    rs->minimap = NULL;
    rs->walltext = NULL;

    SharedState *ss = (SharedState *)malloc(sizeof(SharedState));
    if (ss == NULL) {
        fprintf(stderr, "ERROR: Could not malloc SharedState\n");
        exit(1);
    }
    ss->player = &player;
    ss->entities = (Entity **)malloc(sizeof(Entity **));
    ss->entity_count = 0;
    ss->status = DISCONNECTED;
    ss->game_load_state = INITIAL;

    GameState gs = {.renderState = rs,
                    .sharedState = ss,
                    .game_load_state = INITIAL,
                    .texts = NULL,
                    .text_count = 0};

    uint32_t start_time = 0;
    if (init_state(&gs)) gs.game_load_state = PLAYING;
    printf("New Player ID: %s\n", gs.sharedState->player->id);

    pthread_t net_thread;
    if (pthread_create(&net_thread, NULL, network_thread,
                       (void *)gs.sharedState) < 0) {
        fprintf(stderr, "ERROR: Could not create net_thread\n");
        exit(1);  // TODO: Switch to offline mode?
    }

    start_time = SDL_GetTicks64();
    float max1 = 0;

    while (!(gs.game_load_state == QUIT)) {
        float delta_time = (SDL_GetTicks64() - start_time);
        start_time = SDL_GetTicks64();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    gs.game_load_state = QUIT;
                    break;
            }
        }

        sprintf(gs.texts[0].content, "FPS: %.0f", 1000 / delta_time);
        if (delta_time != 0 && 1000 / delta_time > max1) {
            max1 = 1000 / delta_time;
        }
        delta_time = delta_time / 75.0f;

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

        pthread_mutex_lock(&gs.sharedState->lock);
        const Vector2 old_pos = gs.sharedState->player->pos;
        if (keystate[SDL_SCANCODE_W]) {
            move_player(gs.sharedState->player, delta_time, 1);  // Move forward
        }

        if (keystate[SDL_SCANCODE_S]) {
            move_player(gs.sharedState->player, delta_time,
                        -1);  // Move backward
        }

        if (keystate[SDL_SCANCODE_A]) {
            rotate_player(gs.sharedState->player, delta_time,
                          1);  // Rotate left
        }

        if (keystate[SDL_SCANCODE_D]) {
            rotate_player(gs.sharedState->player, delta_time,
                          -1);  // Rotate right
        }

        if (keystate[SDL_SCANCODE_SPACE]) {
            jump_player(gs.sharedState->player, delta_time);
        }
        player_gravity(gs.sharedState->player, delta_time);
        handle_wall_collision(gs.sharedState, old_pos);
        pthread_mutex_unlock(&gs.sharedState->lock);

        draw_minimap(&gs);

        draw_scene(&gs);
        overlay_minimap(&gs);
        // draw_rectangle(gs.image,
        //                (Vector2i){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, 2, 2,
        //                SCREEN_WIDTH, SCREEN_HEIGHT, 0xFF0000AA);

        SDL_RenderClear(gs.renderState->renderer);
        SDL_UpdateTexture(gs.renderState->texture, NULL,
                          (void *)(gs.renderState->image), SCREEN_WIDTH * 4);
        SDL_RenderCopy(gs.renderState->renderer, gs.renderState->texture, NULL,
                       NULL);
        for (size_t i = 0; i < gs.text_count; i++) {
            Text *text = &gs.texts[i];
            SDL_Surface *textSurface =
                TTF_RenderText_Solid(text->font, text->content, text->colour);

            if (!textSurface) {
                fprintf(stderr, "ERROR: Failed to create text surface: %s\n",
                        TTF_GetError());
                return EXIT_FAILURE;
            }

            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(
                gs.renderState->renderer, textSurface);

            if (!textTexture) {
                fprintf(stderr, "ERROR: Failed to create text texture: %s\n",
                        SDL_GetError());
                return EXIT_FAILURE;
            }
            SDL_RenderCopy(gs.renderState->renderer, textTexture, NULL,
                           &text->position);
        }

        SDL_RenderPresent(gs.renderState->renderer);
    }

    printf("%f\n", max1);
    delete_state(&gs);

    return 0;
}
