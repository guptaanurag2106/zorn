#pragma once

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>

#include "player.h"

typedef struct Text {
    char *content;
    SDL_Color colour;
    SDL_Rect position;
    TTF_Font *font;
} Text;

enum GameLoadState { LOADING, PLAYING, QUIT, INITIAL };

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

    Text *texts;
    size_t text_count;
} GameState;
