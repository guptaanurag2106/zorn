#pragma once

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <bits/pthreadtypes.h>
#include <netinet/in.h>
#include <stdint.h>

#include "net_stuff.h"
#include "player.h"

enum TextType { FPS_Text, HP_Text };

typedef struct Text {
    enum TextType type;
    char *content;
    SDL_Color colour;
    SDL_Rect position;
    TTF_Font *font;
} Text;

typedef Player Entity;

enum GameLoadState { LOADING, PLAYING, QUIT, INITIAL };
enum ServerStatus { CONNECTED, RECONNECTING, DISCONNECTED, OFFLINE };

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

typedef struct NetState {
    const int port;
    char server_name[SERVER_NAME_LEN_MAX + 1];
    int fd;
    struct sockaddr_in serv_addr;
    enum ServerStatus status;
} NetState;

typedef struct SharedState {
    Player *player;

    Entity **entities;
    uint8_t entity_count;

    pthread_mutex_t lock;
} SharedState;

typedef struct GameState {
    NetState *netState;

    RenderState *renderState;
    SharedState *sharedState;

    enum GameLoadState game_load_state;

    Text *texts;
    size_t text_count;
} GameState;
