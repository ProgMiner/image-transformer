#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "../image.h"
#include "../value.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

double min(double a, double b) {
    return a < b ? a : b;
}

void do_rotate(struct image * image, double angle) {
    double center_x, center_y, alpha,
           min_x = DBL_MAX, min_y = DBL_MAX,
           max_x = -DBL_MAX, max_y = -DBL_MAX;

    uint32_t x, y, i, j, k, base_x, base_y, pixels_count;

    struct {
        double x, y;
        struct pixel pixel;
    } * pixels;

    pixels_count = image->width * image->height;
    pixels = malloc(sizeof(*pixels) * pixels_count);

    center_x = ((double) image->width) / 2;
    center_y = ((double) image->height) / 2;

    for (y = 0, i = 0; y < image->height; ++y) {
        for (x = 0; x < image->width; ++x, ++i) {
            pixels[i].x = center_x + (x - center_x) * cos(angle) - (y - center_y) * sin(angle);
            pixels[i].y = center_y + (x - center_x) * sin(angle) + (y - center_y) * cos(angle);
            pixels[i].pixel = image->pixels[i];

            if (min_x > pixels[i].x) {
                min_x = pixels[i].x;
            }

            if (min_y > pixels[i].y) {
                min_y = pixels[i].y;
            }

            if (max_x < pixels[i].x) {
                max_x = pixels[i].x;
            }

            if (max_y < pixels[i].y) {
                max_y = pixels[i].y;
            }
        }
    }

    image->width = ceil(max_x - min_x + 1);
    image->height = ceil(max_y - min_y + 1);
    free(image->pixels);

    image->pixels = calloc(image->width * image->height, sizeof(struct pixel));

    for (i = 0; i < pixels_count; ++i) {
        pixels[i].x -= min_x;
        pixels[i].y -= min_y;

        base_x = ceil(pixels[i].x);
        base_y = ceil(pixels[i].y);

        /* 0 - center
         * 1 - top
         * 2 - right
         * 3 - bottom
         * 4 - left
         */
        for (k = 0; k < 5; ++k) {
            switch (k) {
            case 0:
                x = base_x;
                y = base_y;
                alpha = (1 - abs(pixels[i].x - x)) * (1 - abs(pixels[i].y - y));
                break;

            case 1:
                x = base_x;
                y = base_y - 1;
                alpha = (1 - abs(x - pixels[i].x)) * (1 - min(pixels[i].y - y, 1));
                break;

            case 2:
                x = base_x + 1;
                y = base_y;
                alpha = (1 - min(x - pixels[i].x, 1)) * (1 - abs(y - pixels[i].y));
                break;

            case 3:
                x = base_x;
                y = base_y + 1;
                alpha = (1 - abs(x - pixels[i].x)) * (1 - min(y - pixels[i].y, 1));
                break;

            case 4:
                x = base_x - 1;
                y = base_y;
                alpha = (1 - min(pixels[i].x - x, 1)) * (1 - abs(y - pixels[i].y));
                break;
            }

            if (x >= 0 && x < image->width
             && y >= 0 && y < image->height) {
                j = y * image->width + x;

                image->pixels[j].red += (pixels[i].pixel.red - image->pixels[j].red) * alpha;
                image->pixels[j].green += (pixels[i].pixel.green - image->pixels[j].green) * alpha;
                image->pixels[j].blue += (pixels[i].pixel.blue - image->pixels[j].blue) * alpha;
            }
        }
    }

    free(pixels);
}

const char * rotate(struct image * image, uint32_t argc, const struct value * argv) {
    double angle = argc > 0 && value_is_floating(argv[0])
        ? value_to_floating(argv[0]) * M_PI / 180
        : M_PI / 2;

    do_rotate(image, angle);
    return NULL;
}
