#pragma once

#include <stdint.h>

struct pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct image {
    uint32_t width;
    uint32_t height;

    struct pixel * pixels;
};

struct image image_create(uint32_t width, uint32_t height);
void image_discard(struct image image);

struct image image_clone(const struct image image);
