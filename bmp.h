#pragma once

#include <stdio.h>

#include "image.h"

struct __attribute__((packed)) bmp_header {
    char     bfType[2];
    uint32_t bfSize;
    uint32_t bfReserved;
    uint32_t bfOffBits;

    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

struct bmp_pixel;

struct bmp_image {
    struct bmp_header header;
    struct bmp_pixel * bitmap;
};

void bmp_image_discard(struct bmp_image bmp_image);

struct image bmp_image_to_image(const struct bmp_image);
void bmp_image_replace(struct bmp_image * bmp_image, const struct image image);

const char * bmp_image_read(struct bmp_image * bmp_image, FILE * file);
const char * bmp_image_write(const struct bmp_image bmp_image, FILE * file);
