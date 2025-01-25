#pragma once

// #define SCREEN_WIDTH 1024
// #define SCREEN_HEIGHT 1024
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define MAP_WIDTH 32
#define MAP_HEIGHT 32
#define MAP                                                                                                            \
    ("11111111111111111111111111111111"                                                                                \
     "2              3               2"                                                                                \
     "2  4  0000     3     1111   4  2"                                                                                \
     "2              3               2"                                                                                \
     "2                              2"                                                                                \
     "2  4  1111     4     1111   4  2"                                                                                \
     "2                              2"                                                                                \
     "2     333            555       2"                                                                                \
     "2     3 3     11     5 5       2"                                                                                \
     "222            222           222"                                                                                \
     "2     3 3            5 5       2"                                                                                \
     "2     333            555       2"                                                                                \
     "2          555  333            2"                                                                                \
     "2    44                11      2"                                                                                \
     "2    44                11      2"                                                                                \
     "2          555  333            2"                                                                                \
     "2     222                 444  2"                                                                                \
     "2              555         4   2"                                                                                \
     "2     222                444   2"                                                                                \
     "2                              2"                                                                                \
     "2       411    555       33    2"                                                                                \
     "2                              2"                                                                                \
     "2       411           222      2"                                                                                \
     "2                              2"                                                                                \
     "2    555      333    222       2"                                                                                \
     "2                              2"                                                                                \
     "2                     111      2"                                                                                \
     "2    555      333              2"                                                                                \
     "2                     111      2"                                                                                \
     "2    44        22              2"                                                                                \
     "2    44        22              2"                                                                                \
     "22222222222222222222222222222222")

#define SCALE (float)64
// #define SCALE 32 // doesn't work
#define WORLD_WIDTH (MAP_WIDTH * SCALE)
#define WORLD_HEIGHT (MAP_HEIGHT * SCALE)

#define MINIMAP_SCALE 1.0f / 4
#define MINIMAP_RANGE (int)(0.5 * WORLD_WIDTH)

#define GRAVITY 30 // for jumping

#define PLAYER_SIZE 50
#define PLAYER_SNAP_ROTATION PI / 18
#define NEAR_CLIPPING_PLANE (PLAYER_SIZE / 2 + 5)
// #define FAR_CLIPPING_PLANE (WORLD_HEIGHT / 2)
#define FAR_CLIPPING_PLANE (WORLD_HEIGHT / 1)
