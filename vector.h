#pragma once

#define PI 3.14159265359f
#define PI_2 (PI / 2)
#define PI_3_4 (3.0f * PI / 4)

#define DEG2RAD(_d) ((_d) * (PI / 180.0f))
#define RAD2DEG(_r) ((_r) * (180.0f / PI))

typedef struct Vector2 {
    float x, y;
} Vector2;

#define dot2D(v0, v1)                                                                                                  \
    ({                                                                                                                 \
        const Vector2 _v0 = (v0), _v1 = (v1);                                                                          \
        (_v0.x * _v1.x) + (_v0.y * _v1.y);                                                                             \
    })
#define length2D(v)                                                                                                    \
    ({                                                                                                                 \
        const Vector2 _v = (v);                                                                                        \
        sqrtf(dot(_v, _v));                                                                                            \
    })
#define normalize2D(u)                                                                                                 \
    ({                                                                                                                 \
        const Vector2 _u = (u);                                                                                        \
        const float l = length(_u);                                                                                    \
        (Vector2){_u.x / l, _u.y / l};                                                                                 \
    })

// #define scale2D(v, x) ((Vector2){(v).x * (x), (v).y * (x)})
#define scale2D(v, x) ((Vector2){(v).x * (x), (v).y * (x)})



// #define scale2D(v, x)                                                                                                  \
//     ({                                                                                                                  \
//         (Vector2){(v).x * x, (v).y * x};                                                                               \
//     })

#define add2D(v0, v1) ({ (Vector2){(v0).x + (v1).x, (v0).y + (v1).y}; })

typedef struct Vector3 {
    float x, y, z;
} Vector3;
