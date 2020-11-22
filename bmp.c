#include "bmp.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct bmp_pixel {
    uint8_t b, g, r;
};

void bmp_image_discard(struct bmp_image image) {
    free(image.bitmap);
}

struct image bmp_image_to_image(const struct bmp_image bmp_image) {
    uint32_t size = bmp_image.header.biWidth * bmp_image.header.biHeight;
    struct image image = image_create(
        bmp_image.header.biWidth,
        bmp_image.header.biHeight
    );

    uint32_t i;
    for (i = 0; i < size; ++i) {
        image.pixels[i].red = bmp_image.bitmap[i].r;
        image.pixels[i].green = bmp_image.bitmap[i].g;
        image.pixels[i].blue = bmp_image.bitmap[i].b;
    }

    return image;
}

void bmp_image_repair(struct bmp_image * image) {

    /* Repair signature */
    image->header.bfType[0] = 'B';
    image->header.bfType[1] = 'M';

    /* Repair offset */
    image->header.bfOffBits = sizeof(struct bmp_header);

    /* Repair common parameters */
    image->header.biSize = 40;
    image->header.biPlanes = 1;
    image->header.biBitCount = 24;
    image->header.biCompression = 0;

    /* Calc size image */
    image->header.biSizeImage = image->header.biHeight *
        (image->header.biWidth * sizeof(struct bmp_pixel)
            + image->header.biWidth % 4);

    /* Calc file size */
    image->header.bfSize = image->header.bfOffBits + image->header.biSizeImage;
}

void bmp_image_replace(struct bmp_image * bmp_image, const struct image image) {
    uint32_t size = image.width * image.height;
    uint32_t i;

    bmp_image->header.biWidth = image.width;
    bmp_image->header.biHeight = image.height;

    free(bmp_image->bitmap);
    bmp_image->bitmap = malloc(sizeof(struct bmp_pixel) * size);

    for (i = 0; i < size; ++i) {
        bmp_image->bitmap[i].r = image.pixels[i].red;
        bmp_image->bitmap[i].g = image.pixels[i].green;
        bmp_image->bitmap[i].b = image.pixels[i].blue;
    }

    bmp_image_repair(bmp_image);
}

const char * bmp_image_read(struct bmp_image * image, FILE * file) {
    int32_t row, rowOffset;

    size_t read_count = fread(&(image->header), sizeof(struct bmp_header), 1, file);
    if (read_count < 1) {
        return "cannot read BMP file";
    }

    /* Check file type signature */
    if (image->header.bfType[0] != 'B' || image->header.bfType[1] != 'M') {
        return "invalid BMP file";
    }

    if ((image->header.biSizeImage         /* Check size if biSizeImage != 0 */
     && (image->header.bfSize != image->header.bfOffBits + image->header.biSizeImage))
     || (image->header.biPlanes != 1)      /* Check biPlanes */
     || (image->header.biBitCount != 24)   /* Check pixel bits count, only 24 is supported */
     || (image->header.biCompression != 0) /* Check biCompression, only 0 is supported */
    ) {
        return "invalid BMP file";
    }

    /* Check file size */
    if (fseek(file, 0L, SEEK_END)) {
        return strerror(errno);
    }

    if (ftell(file) != image->header.bfSize) {
        return strerror(errno);
    }

    /* Go to bitmap */
    if (fseek(file, image->header.bfOffBits, SEEK_SET)) {
        return strerror(errno);
    }

    image->bitmap = malloc(sizeof(struct bmp_pixel) * image->header.biWidth * image->header.biHeight);

    rowOffset = image->header.biWidth % 4;
    for (row = image->header.biHeight - 1; row >= 0; --row) {
        read_count = fread(image->bitmap + row * image->header.biWidth, sizeof(struct bmp_pixel), image->header.biWidth, file);

        if (read_count < image->header.biWidth) {
            free(image->bitmap);
            return "cannot read BMP file";
        }

        if (fseek(file, rowOffset, SEEK_CUR)) {
            free(image->bitmap);
            return strerror(errno);
        }
    }

    return NULL;
}

const char * bmp_image_write(const struct bmp_image image, FILE * file) {
    static uint8_t offsetBuffer[] = { 0, 0, 0 };
    int32_t row, rowOffset;

    if (fwrite(&(image.header), sizeof(struct bmp_header), 1, file) < 1) {
        return "cannot write file";
    }

    rowOffset = image.header.biWidth % 4;
    for (row = image.header.biHeight - 1; row >= 0; --row) {
        if (fwrite(image.bitmap + row * image.header.biWidth,
            sizeof(struct bmp_pixel), image.header.biWidth, file) < image.header.biWidth) {
            return strerror(errno);
        }

        if (fwrite(offsetBuffer, 1, rowOffset, file) < rowOffset) {
            return strerror(errno);
        }
    }

    return NULL;
}
