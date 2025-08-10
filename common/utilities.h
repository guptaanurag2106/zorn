#pragma once

#include <stdint.h>
#include <stdio.h>

uint32_t pack_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void unpack_colour(uint32_t colour, uint8_t *r, uint8_t *g, uint8_t *b,
                   uint8_t *a);

static inline uint32_t darken_colour(uint32_t colour, float factor) {
    uint32_t darkened_color = colour;

    darkened_color &= 0xFF00FFFF;
    darkened_color |= (uint32_t)((((colour >> 16) & 0xFF) * factor)) << 16;

    darkened_color &= 0xFFFF00FF;
    darkened_color |= (uint32_t)((((colour >> 8) & 0xFF) * factor)) << 8;
    darkened_color &= 0xFFFFFF00;
    darkened_color |= (uint32_t)((((colour >> 0) & 0xFF) * factor)) << 0;

    return darkened_color;
}

void dump_ppm(const char *output_file, uint32_t *image, const size_t width,
              const size_t height);
