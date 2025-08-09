#include "textures.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "utilities.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool load_texture(const char *texture_file, uint32_t **texture,
                  size_t *text_size, size_t *text_cnt) {
    int nchannels = -1, w, h;
    uint8_t *texture_map = stbi_load(texture_file, &w, &h, &nchannels, 0);
    if (texture_map == NULL) {
        fprintf(stderr, "ERROR: Could not load texture file %s", texture_file);
        return false;
    }

    if (nchannels != 4) {
        fprintf(stderr, "ERROR: Texture file %s is not a 32 bit image",
                texture_file);
        stbi_image_free(texture_map);
        return false;
    }

    if (w % h != 0) {
        fprintf(stderr, "ERROR: Textures are not square (file: %s)",
                texture_file);
        stbi_image_free(texture_map);
        return false;
    }

    *text_cnt = w / h;  // textures are a square
    *text_size = h;

    *texture = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (texture == NULL) {
        fprintf(stderr, "ERROR: Could not allocate texture_map array");
        stbi_image_free(texture_map);
        return false;
    }

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            uint8_t r = texture_map[(i + j * w) * 4 + 0];
            uint8_t g = texture_map[(i + j * w) * 4 + 1];
            uint8_t b = texture_map[(i + j * w) * 4 + 2];
            uint8_t a = texture_map[(i + j * w) * 4 + 3];
            (*texture)[i + j * w] = pack_colour(r, g, b, a);
        }
    }

    stbi_image_free(texture_map);
    return true;
}
