#pragma once

#include <stdint.h>
#include <stdio.h>

uint32_t pack_colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void unpack_colour(uint32_t colour, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

void dump_ppm(const char *output_file, uint32_t *image, const size_t width, const size_t height);
