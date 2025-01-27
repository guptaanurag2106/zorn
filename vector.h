#pragma once

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

static inline Vector2 add2D(Vector2 a, Vector2 b) { return (Vector2){.x = a.x + b.x, .y = a.y + b.y}; }

static inline Vector2i add2Di(Vector2i a, Vector2i b) { return (Vector2i){.x = a.x + b.x, .y = a.y + b.y}; }

typedef struct Vector3 {
    float x, y, z;
} Vector3;
