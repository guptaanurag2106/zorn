#pragma once

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <bits/pthreadtypes.h>
#include <stdint.h>

#include "player.h"

enum TextType { FPS };

typedef struct Text {
    enum TextType type;
    char *content;
    SDL_Color colour;
    SDL_Rect position;
    TTF_Font *font;
} Text;

typedef Player Entity;

enum GameLoadState { LOADING, PLAYING, QUIT, INITIAL };
enum ServerStatus { CONNECTED, RECONNECTING, DISCONNECTED };

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *image;
    uint32_t *minimap;

    uint32_t *walltext;
    size_t walltext_size;
    size_t walltext_cnt;
} RenderState;

typedef struct SharedState {
    _Atomic enum ServerStatus status;
    Player *player;

    Entity **entities;
    uint8_t entity_count;

    _Atomic enum GameLoadState game_load_state;

    pthread_mutex_t lock;
} SharedState;

typedef struct GameState {
    RenderState *renderState;
    SharedState *sharedState;

    enum GameLoadState game_load_state;

    Text *texts;
    size_t text_count;
} GameState;
