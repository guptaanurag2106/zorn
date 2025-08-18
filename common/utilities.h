#pragma once

#include <stdint.h>
#include <stdio.h>
#include <time.h>

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

typedef struct {
    struct timespec last_time;
    float frequency;
} Timer;
void call_if_due(Timer *timer, void (*func)(void *), void *arg);

#define SENDM(fd, type, ...)                                         \
    ({                                                               \
        char *packet = COMBINE("", __VA_ARGS__);                     \
        sendm_(fd, temp_sprintf("%s|%02d|%04d|%s", PROTOCOL_V, type, \
                                strlen(packet), packet));            \
    })

int sendm_(int fd, const char *message);
