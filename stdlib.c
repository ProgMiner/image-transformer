#include <stdint.h>
#include <stdio.h>

#include "image.h"
#include "value.h"
#include "util.h"

const char * echo(const struct image * image, uint32_t argc, const struct value * args) {
    puts(argc > 0 && value_is_string(args[0]) ? value_to_string(args[0]) : "");
    return NULL;
}

const char * die(const struct image * image, uint32_t argc, const struct value * args) {
    static char * message;

    return argc > 0 && value_is_string(args[0])
        ? strset(&message, value_to_string(args[0]))
        : "suicide";
}

void do_print_ansi(const struct image image, const char * pixel_string) {
    uint32_t x, y, i;

    for (y = 0, i = 0; y < image.height; ++y) {
        for (x = 0; x < image.width; ++x, ++i) {
            printf("\x1B[48;2;%d;%d;%dm%s\x1B[0m",
                image.pixels[i].red,
                image.pixels[i].green,
                image.pixels[i].blue,
                pixel_string);
        }

        putchar('\n');
    }
}

const char * print_ansi(const struct image * image, uint32_t argc, const struct value * args) {
    do_print_ansi(*image, argc > 0 && value_is_string(args[0])
            ? value_to_string(args[0]) : "  ");

    return NULL;
}
