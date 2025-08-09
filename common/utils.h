#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stdio.h"

#ifdef _cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
//  General Utils
// ----------------------------------------------------------------------------
#define MAX(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })

#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _b : _a;      \
    })

char *shift(int *argc, char ***argv) {
    if (*argc <= 0) {
        fprintf(stderr, "ERROR: `shift` requested with non-positive argc\n");
        exit(1);
    }
    (*argc)--;
    return *((*argv)++);
}

char *get_uuid(uint8_t l) {
    char v[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    // 3fb17ebc-bc38-4939-bc8b-74f2443281d4
    // 8 dash 4 dash 4 dash 4 dash 12
    char *buf = malloc(sizeof(char) * (l + 1));
    if (buf == NULL) {
        fprintf(stderr, "ERROR: Could not allocate buffer get_uuid\n");
        exit(1);
    }

    for (int i = 0; i < l; ++i) {
        buf[i] = v[rand() % 16];
    }

    buf[8] = '-';
    buf[13] = '-';
    buf[18] = '-';
    buf[23] = '-';

    buf[36] = '\0';

    return buf;
}

// ----------------------------------------------------------------------------
//  Log Utils
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  String Utils
// ----------------------------------------------------------------------------
void combine_charp(const char *str1, const char *str2, char **combined) {
    size_t length = strlen(str1) + strlen(str2) + 1;
    *combined = (char *)malloc(length);

    if (*combined == NULL) {
        fprintf(stderr, "Failed to allocate memory");
    }

    strcpy(*combined, str1);
    strcat(*combined, str2);
}

// ----------------------------------------------------------------------------
//  Vector Utils
// ----------------------------------------------------------------------------
typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Ivector;

Ivector *init_Ivector(size_t init_cap) {
    Ivector *vector = (Ivector *)malloc(sizeof(Ivector));
    if (vector == NULL) {
        fprintf(stderr, "ERROR: Ivector_init malloc failed\n");
        exit(1);
    }

    vector->data = (int *)malloc(sizeof(int) * init_cap);
    vector->size = 0;
    vector->capacity = init_cap;

    return vector;
}

void resize_Ivector(Ivector *vector, size_t new_capacity) {
    vector->data = (int *)realloc(vector->data, sizeof(int) * new_capacity);

    vector->capacity = new_capacity;
}

void push_back_Ivector(Ivector *vector, int val) {
    if (vector->size >= vector->capacity) {
        resize_Ivector(vector, vector->size * 2);
    }
    vector->data[vector->size++] = val;
}

int pop_back_Ivector(Ivector *vector) {
    if (vector->size == 0) {
        fprintf(stderr,
                "ERROR: Ivector_pop_back cannot pop from empty vector\n");
        exit(1);
    }

    int val = vector->data[vector->size - 1];
    vector->size--;
    return val;
}

int get_Ivector(Ivector *vector, size_t pos) {
    if (pos >= vector->size) {
        fprintf(stderr, "ERROR: Ivector_get index %zu out of bounds (%zu)\n",
                pos, vector->size);
        exit(1);
    }
    return vector->data[pos];
}

void reset_Ivector(Ivector *vector) {
    free(vector->data);
    vector->size = 0;
    vector->capacity = 0;
}

#ifdef _cplusplus
};
#endif
#endif
