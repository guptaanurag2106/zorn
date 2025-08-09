#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

uint32_t pack_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void unpack_colour(uint32_t colour, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

uint32_t darken_color(uint32_t colour, int d, int limit);

void dump_ppm(const char *output_file, uint32_t *image, const size_t width, const size_t height);
