#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

bool load_texture(const char *texture_file, uint32_t **texture, size_t *text_size, size_t *text_cnt);
