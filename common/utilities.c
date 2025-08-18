#include "utilities.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

void call_if_due(Timer *timer, void (*func)(void *), void *arg) {
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    int diff = (current_time.tv_sec - timer->last_time.tv_sec) * 1000000 +
               (current_time.tv_nsec - timer->last_time.tv_nsec) / 1000 -
               1000000 / timer->frequency;

    if (diff >= 0) {
        func(arg);
        clock_gettime(CLOCK_MONOTONIC_RAW, &timer->last_time);
    } else
        usleep(-1 * diff);
}

int sendm_(int fd, const char *message) {
    printf("INFO: sending %lu: %s\n", strlen(message), message);
    return send(fd, message, strlen(message), 0);
    // printf("%s", packet);                                             \
        // size_t packet_len = strlen(packet);                               \
        // sendm_(fd, COMBINE("|", PROTOCOL_V, get_packet_type_string(type), \
        //                    packet_len, packet));                          \
        //
    // int i = 0;
    // for (i = 0; i < MAX_CONNECT_RETRIES; i++) {
    // if (send(fd, message, strlen(message), 0) < 0) {
    // } else
    // return 0;
    // }
    // return -1;
}
