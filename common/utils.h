#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#if defined(__clang__) || defined(__GNUC__)
#define ARRAY_LENGTH(arr)                                             \
    (sizeof(arr) / sizeof((arr)[0]) +                                 \
     0 * sizeof(struct {                                              \
         int _ : !__builtin_types_compatible_p(__typeof__(arr),       \
                                               __typeof__(&(arr)[0])) \
                 ? 1                                                  \
                 : -1;                                                \
     }))
#else
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

char *shift(int *argc, char ***argv);
char *get_uuid();

// ----------------------------------------------------------------------------
//  Log Utils
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  Math Utils
// ----------------------------------------------------------------------------
#define PI 3.14159265359f
#define PI_2 (PI / 2)
#define PI_3_4 (3.0f * PI / 4)

#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_r) ((_r) * (180.0f / PI))

typedef struct Vector2 {
    float x, y;
} Vector2;

typedef struct Vector2i {
    int x, y;
} Vector2i;

static inline Vector2 add2D(Vector2 a, Vector2 b) {
    return (Vector2){.x = a.x + b.x, .y = a.y + b.y};
}
static inline Vector2i add2Di(Vector2i a, Vector2i b) {
    return (Vector2i){.x = a.x + b.x, .y = a.y + b.y};
}

typedef struct Vector3 {
    float x, y, z;
} Vector3;

static inline uint32_t clamp_u32(uint32_t v, uint32_t lo, uint32_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// ----------------------------------------------------------------------------
//  String Utils
// ----------------------------------------------------------------------------
#define UTILS_MAX_TEMP_SIZE 1024
static char utils_static_payload_buffer[UTILS_MAX_TEMP_SIZE];
static char utils_static_temp_buffer[UTILS_MAX_TEMP_SIZE];
// Will malloc combined, free it yourself
void combine_charp(const char *str1, const char *str2, char **combined);
// Will use the utils_static_payload_buffer and reset it everytime
#define COMBINE(separator, ...) \
    combine_strings_with_sep_(separator, __VA_ARGS__, NULL)
// Will use the utils_static_payload_buffer and reset it everytime, last va_arg
// should be NULL
char *combine_strings_with_sep_(const char *separator, ...);
// Will use the utils_static_temp_buffer and reset it everytime
char *temp_sprintf(const char *format, ...);

// ----------------------------------------------------------------------------
//  Vector Utils
// ----------------------------------------------------------------------------
typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Ivector;

Ivector *init_Ivector(size_t init_cap);
void resize_Ivector(Ivector *vector, size_t new_capacity);
void push_back_Ivector(Ivector *vector, int val);
int pop_back_Ivector(Ivector *vector);
int get_Ivector(Ivector *vector, size_t pos);
void reset_Ivector(Ivector *vector);

#ifdef _cplusplus
};
#endif
#endif  // UTILS_H

#ifdef UTILS_IMPLEMENTATION

char *shift(int *argc, char ***argv) {
    if (*argc <= 0) {
        fprintf(stderr, "ERROR: [shift] Requested with non-positive argc\n");
        exit(1);
    }
    (*argc)--;
    return *((*argv)++);
}

char *get_uuid() {
    char v[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    char *buf = malloc(sizeof(char) * (37));
    if (buf == NULL) {
        fprintf(stderr, "ERROR: [get_uuid] Could not allocate buffer\n");
        exit(1);
    }

    for (int i = 0; i < 36; ++i) {
        buf[i] = v[rand() % 16];
    }

    buf[8] = '-';
    buf[13] = '-';
    buf[18] = '-';
    buf[23] = '-';

    buf[36] = '\0';

    return buf;
}

void combine_charp(const char *str1, const char *str2, char **combined) {
    size_t length = strlen(str1) + strlen(str2) + 1;
    *combined = (char *)malloc(length);

    if (*combined == NULL) {
        fprintf(stderr, "ERROR: [combine_charp] Could not allocate buffer\n");
        return;
    }

    strcpy(*combined, str1);
    strcat(*combined, str2);
}

char *combine_strings_with_sep_(const char *separator, ...) {
    va_list args;
    va_start(args, separator);
    const char *str = va_arg(args, const char *);
    int count = 0;
    while (str != NULL) {
        str = va_arg(args, const char *);
        count++;
    }
    va_end(args);

    va_start(args, separator);
    size_t offset = 0;
    str = va_arg(args, const char *);
    for (int i = 0; i < count; i++) {
        size_t len = strlen(str);
        memcpy(utils_static_payload_buffer + offset, str, len);
        offset += len;
        if (i < count - 1) {
            memcpy(utils_static_payload_buffer + offset, separator,
                   strlen(separator));
            offset += strlen(separator);
        }

        str = va_arg(args, const char *);
    }
    va_end(args);
    utils_static_payload_buffer[offset] = '\0';
    return utils_static_payload_buffer;
}

char *temp_sprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int n = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (n < 0) {
        fprintf(stderr, "ERROR: [temp_sprintf] vsnprintf returned neg size\n");
        return NULL;
    }
    if (n >= UTILS_MAX_TEMP_SIZE) {
        fprintf(stderr,
                "ERROR: [temp_sprintf] vsnprintf returned size greater than "
                "UTILS_MAX_TEMP_SIZE\n");
        return NULL;
    }

    va_start(args, format);
    vsnprintf(utils_static_temp_buffer, UTILS_MAX_TEMP_SIZE, format, args);
    va_end(args);

    return utils_static_temp_buffer;
}

Ivector *init_Ivector(size_t init_cap) {
    Ivector *vector = (Ivector *)malloc(sizeof(Ivector));
    if (vector == NULL) {
        fprintf(stderr, "ERROR: [init_Ivector] Could not allocate Ivector\n");
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
                "ERROR: [Ivector_pop_back] cannot pop from empty vector\n");
        exit(1);
    }

    int val = vector->data[vector->size - 1];
    vector->size--;
    return val;
}

int get_Ivector(Ivector *vector, size_t pos) {
    if (pos >= vector->size) {
        fprintf(stderr, "ERROR: [Ivector_get] index %zu out of bounds (%zu)\n",
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

#endif  // UTILS_IMPLEMENTATION
