#include "utilities.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t pack_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (a << 24) | (b << 16) | (g << 8) | (r << 0);
}

void unpack_colour(uint32_t colour, uint8_t *r, uint8_t *g, uint8_t *b,
                   uint8_t *a) {
    *r = (colour >> 0) & 0xFF;
    *g = (colour >> 8) & 0xFF;
    *b = (colour >> 16) & 0xFF;
    *a = (colour >> 24) & 0xFF;
}

uint32_t darken_color(uint32_t colour, int d, int limit) {
    if (d < 1) d = 1;
    if (d > limit) d = limit;

    float factor = 1 / (1 + powf((float)d * 10 / limit, 5) * 0.0002);
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
              const size_t height) {
    FILE *output = fopen(output_file, "wb");

    fprintf(output, "P3\n");
    fprintf(output, "%zu %zu\n", width, height);
    fprintf(output, "%u\n", 255);
    uint8_t r = 0, g = 0, b = 0, a = 0;

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            unpack_colour(image[j + i * width], &r, &g, &b, &a);
            fprintf(output, "%u %u %u", r, g, b);
            if (j != (width - 1)) fprintf(output, " ");
        }
        fprintf(output, "\n");
    }

    fclose(output);
}
