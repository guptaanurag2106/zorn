#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "constants.h"
#include "game_state.h"
#include "player.h"
#include "textures.h"
#include "utilities.h"
#include "utils.h"
#include "vector.h"

void draw_rectangle(uint32_t *image, Vector2i pos, uint32_t w, uint32_t h,
                    uint32_t image_w, uint32_t image_h, uint32_t colour) {
    for (int i = 0; i < w; i++) {
        int cx = pos.x + i;
        if (cx >= image_w) break;
        for (int j = 0; j < h; j++) {
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
            if (cx >= MAP_WIDTH || cy >= MAP_HEIGHT || cx < 0 || cy < 0)
                continue;

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
                int rotated_x =
                    (int)(cos_theta * dx - sin_theta * dy) + center_x;
                int rotated_y =
                    (int)(sin_theta * dx + cos_theta * dy) + center_y;

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

void handle_wall_collision(SharedState *ss, Vector2 old_pos) {
    if (ss->player->pos.x - PLAYER_SIZE / 2 < 0 ||
        ss->player->pos.x + PLAYER_SIZE / 2 >= WORLD_WIDTH) {
        ss->player->pos.x = old_pos.x;  // Revert to old position
    }

    if (ss->player->pos.y - PLAYER_SIZE / 2 < 0 ||
        ss->player->pos.y + PLAYER_SIZE / 2 >= WORLD_HEIGHT) {
        ss->player->pos.y = old_pos.y;  // Revert to old position
    }

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

// TODO
void render_ceiling(GameState *gs) {
    for (uint32_t j = 0;
         j <= (SCREEN_HEIGHT / 2 + gs->sharedState->player->eye_z); j++) {
        uint32_t colour = darken_color(
            0xFFEBCE87, j, SCREEN_HEIGHT / 2 + gs->sharedState->player->eye_z);

        draw_rectangle(gs->renderState->image, (Vector2i){0, j}, SCREEN_WIDTH,
                       1, SCREEN_WIDTH, SCREEN_HEIGHT, colour);
    }
}

// TODO
void render_floors(GameState *gs) {
    for (uint32_t j = 0;
         j <= (SCREEN_HEIGHT / 2 - gs->sharedState->player->eye_z); j++) {
        uint32_t colour =
            darken_color(pack_colour(0x18, 0x18, 0x18, 0xFF), j,
                         SCREEN_HEIGHT / 2 - gs->sharedState->player->eye_z);

        draw_rectangle(gs->renderState->image, (Vector2i){0, SCREEN_HEIGHT - j},
                       SCREEN_WIDTH, 1, SCREEN_WIDTH, SCREEN_HEIGHT, colour);
    }
}

void draw_scene(GameState *gs) {
    render_ceiling(gs);
    render_floors(gs);

    for (uint32_t i = 0; i < SCREEN_WIDTH; i++) {
        float theta = (gs->sharedState->player->theta -
                       gs->sharedState->player->hfov / 2) +
                      i * gs->sharedState->player->hfov / SCREEN_WIDTH;

        for (uint32_t j = NEAR_CLIPPING_PLANE; j <= FAR_CLIPPING_PLANE; j++) {
            float cx = gs->sharedState->player->pos.x + j * sin(theta);
            float cy = gs->sharedState->player->pos.y + j * cos(theta);
            if (cx >= WORLD_WIDTH || cy >= WORLD_HEIGHT || cx < 0 || cy < 0)
                break;

            float mx = cx / SCALE, my = cy / SCALE;  // x,y within map

            char id = MAP[(int)mx + (int)my * MAP_WIDTH];
            if (id == ' ')  // floor
                continue;

            int textid = id - '0';

            float column_height =
                WORLD_HEIGHT * 20 /
                (j * cos(theta - gs->sharedState->player->theta));
            // 20 is the scaling factor for walls

            if (!(textid >= 0 && textid < gs->renderState->walltext_cnt)) {
                fprintf(stderr,
                        "ERROR: missing/incorrect texture %d, setting to 0, "
                        "for position {%d, %d}\n",
                        textid, (int)mx, (int)my);
                textid = 0;
            }

            int ty = 0, tx = 0;  // x,y within a particular texture

            float hitx = mx - floor(mx + 0.5);
            float hity = my - floor(my + 0.5);

            tx = hitx * gs->renderState->walltext_size;
            if (fabsf(hity) > fabsf(hitx)) {
                tx = hity * gs->renderState->walltext_size;
            }
            if (tx < 0) tx += gs->renderState->walltext_size;

            for (size_t y = 0; y < column_height; y++) {
                ty = y * gs->renderState->walltext_size / column_height;
                uint32_t colour =
                    gs->renderState
                        ->walltext[(tx +
                                    textid * gs->renderState->walltext_size) +
                                   ty * (gs->renderState->walltext_size *
                                         gs->renderState->walltext_cnt)];
                colour = darken_color(colour, j, FAR_CLIPPING_PLANE);

                draw_rectangle(
                    gs->renderState->image,
                    (Vector2i){SCREEN_WIDTH - i,
                               SCREEN_HEIGHT / 2.0f - column_height / 2 +
                                   gs->sharedState->player->eye_z + y},
                    1, 1, SCREEN_WIDTH, SCREEN_HEIGHT, colour);
            }

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

    SDL_UpdateTexture(gs->renderState->texture, NULL,
                      (void *)(gs->renderState->image), SCREEN_WIDTH * 4);
    SDL_RenderCopy(gs->renderState->renderer, gs->renderState->texture, NULL,
                   NULL);
    SDL_RenderPresent(gs->renderState->renderer);

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

    get_player_random_init(gs->sharedState->player);

    printf("New Player ID: %s\n", gs->sharedState->player->id);

    return true;
}

int main() {
    assert(sizeof(MAP) == MAP_WIDTH * MAP_HEIGHT + 1);

    Player player = {
        .id = get_uuid(36),
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
    ss->entities = NULL;
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
    start_time = SDL_GetTicks64();

    while (!(gs.game_load_state == QUIT)) {
        float delta_time = (SDL_GetTicks64() - start_time) / 75.0f;
        start_time = SDL_GetTicks64();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    gs.game_load_state = QUIT;
                    break;
            }
        }

        gs.texts[0].content = malloc(sizeof(char) * 15);
        sprintf(gs.texts[0].content, "FPS: %.0f", 1000 / delta_time);

        const Uint8 *keystate = SDL_GetKeyboardState(NULL);

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

        draw_minimap(&gs);

        draw_scene(&gs);
        overlay_minimap(&gs);
        // draw_rectangle(gs.image,
        //                (Vector2i){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, 2, 2,
        //                SCREEN_WIDTH, SCREEN_HEIGHT, 0xFF0000AA);

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

    delete_state(&gs);

    return 0;
}
