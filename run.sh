#!/usr/bin/env bash

set -xe

gcc -Wall -Wextra -Wpedantic -g textures.c player.c main.c utils.c -lm -o main
# gcc -Wall -Wextra -O3 textures.c player.c main.c utils.c -lm -o main
