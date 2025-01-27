#pragma once

#include "player.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

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
