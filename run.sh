#!/usr/bin/env bash

set -xe

SDL2_INCLUDE_PATH=$(pkg-config --cflags /usr/local/lib/pkgconfig/sdl2.pc)
SDL2_LIB_PATH=$(pkg-config --libs /usr/local/lib/pkgconfig/sdl2.pc)

gcc -Wall -Wextra -Wpedantic ${SDL2_INCLUDE_PATH} ${SDL2_LIB_PATH} textures.c player.c main.c utils.c -lm -o main
# gcc -Wall -Wextra -Wpedantic -O3 ${SDL2_INCLUDE_PATH} ${SDL2_LIB_PATH} textures.c player.c main.c utils.c -lm -o main
