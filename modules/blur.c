#include <stddef.h>

#include "../image.h"
#include "../value.h"

typedef struct pixel (* blur_t)(uint32_t x, uint32_t y, const struct image image);

struct image expand_image(struct image image) {
    static const struct pixel black_pixel = { 0, 0, 0 };
    uint32_t width, height, x, y, i;
    struct image new_image;

    width = image.width + 2;
    height = image.height + 2;
    new_image = image_create(width, height);

    i = (height - 1) * width;
    for (x = 0; x < width; ++x) {
        new_image.pixels[x] = black_pixel;
        new_image.pixels[i + x] = black_pixel;
    }

    for (y = 1; y < height; ++y) {
        new_image.pixels[y * width - 1] = black_pixel;
        new_image.pixels[y * width] = black_pixel;
    }

    for (y = 0, i = 0; y < image.height; ++y) {
        for (x = 0; x < image.width; ++x, ++i) {
            new_image.pixels[width * (y + 1) + x + 1] = image.pixels[i];
        }
    }

    return new_image;
}

void do_blur(struct image image, blur_t map) {
    struct image expanded_image = expand_image(image);
    uint32_t x, y;

    for (y = 0; y < image.height; ++y) {
        for (x = 0; x < image.width; ++x) {
            image.pixels[image.width * y + x] = map(x + 1, y + 1, expanded_image);
        }
    }

    image_discard(expanded_image);
}

struct pixel blur(uint32_t x, uint32_t y, const struct image image) {
    uint32_t sum_r = 0, sum_g = 0, sum_b = 0, i;
    int8_t kern_x, kern_y;
    struct pixel pixel;

    for (kern_y = -1; kern_y < 2; ++kern_y) {
        for (kern_x = -1; kern_x < 2; ++kern_x) {
            i = image.width * (y + kern_y) + x + kern_x;

            sum_r += image.pixels[i].red;
            sum_g += image.pixels[i].green;
            sum_b += image.pixels[i].blue;
        }
    }

    pixel.red   = sum_r / 9;
    pixel.green = sum_g / 9;
    pixel.blue  = sum_b / 9;
    return pixel;
}

struct pixel dilate(uint32_t x, uint32_t y, const struct image image) {
    uint8_t max_r = 0, max_g = 0, max_b = 0;
    int8_t kern_x, kern_y;
    struct pixel pixel;
    uint32_t i;

    for (kern_y = -1; kern_y < 2; ++kern_y) {
        for (kern_x = -1; kern_x < 2; ++kern_x) {
            i = image.width * (y + kern_y) + x + kern_x;

            if (max_r <= image.pixels[i].red
             && max_g <= image.pixels[i].green
             && max_b <= image.pixels[i].blue) {
                max_r = image.pixels[i].red;
                max_g = image.pixels[i].green;
                max_b = image.pixels[i].blue;
            }
        }
    }

    pixel.red   = max_r;
    pixel.green = max_g;
    pixel.blue  = max_b;
    return pixel;
}

struct pixel erode(uint32_t x, uint32_t y, const struct image image) {
    uint8_t min_r = 255, min_g = 255, min_b = 255;
    int8_t kern_x, kern_y;
    struct pixel pixel;
    uint32_t i;

    for (kern_y = -1; kern_y < 2; ++kern_y) {
        for (kern_x = -1; kern_x < 2; ++kern_x) {
            i = image.width * (y + kern_y) + x + kern_x;

            if (min_r >= image.pixels[i].red
             && min_g >= image.pixels[i].green
             && min_b >= image.pixels[i].blue) {
                min_r = image.pixels[i].red;
                min_g = image.pixels[i].green;
                min_b = image.pixels[i].blue;
            }
        }
    }

    pixel.red   = min_r;
    pixel.green = min_g;
    pixel.blue  = min_b;
    return pixel;
}

const char * do_(struct image * image, uint32_t argc, struct value * args) {
    blur_t map_function;

    if (argc < 1 || !value_is_identifier(args[0])) {
        return "blur type (blur, dilate or erode) is required as first argument";
    }

    *((void **) &map_function) = value_to_identifier(args[0]);

    if (map_function != blur && map_function != dilate && map_function != erode) {
        return "wrong blur type, only blur, dilate or erode are allowed";
    }

    do_blur(*image, map_function);
    return NULL;
}
