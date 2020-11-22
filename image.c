#include "image.h"

#include <stdlib.h>
#include <string.h>

struct image image_create(uint32_t width, uint32_t height) {
    struct image image;

    image.width = width;
    image.height = height;
    image.pixels = malloc(sizeof(struct pixel) * width * height);

    return image;
}

void image_discard(struct image image) {
    free(image.pixels);
}

struct image image_clone(const struct image image) {
    struct image new_image = image_create(
        image.width,
        image.height
    );

    memcpy(new_image.pixels, image.pixels, sizeof(struct pixel) * image.width * image.height);
    return new_image;
}
