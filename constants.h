#pragma once

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
#define MINIMAP_RANGE (int)(0.5 * WORLD_WIDTH)
