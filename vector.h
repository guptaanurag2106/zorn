#pragma once

#define PI 3.14159265359f
#define PI_2 (PI / 2)

#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_r) ((_r) * (180.0f / PI))

typedef struct Vector2 {
    float x, y;
} Vector2;
