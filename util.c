#include "util.h"

#include <string.h>
#include <stdlib.h>

char * strdup(const char * src) {
    size_t length;
    char * dst;

    if (src == NULL) {
        return NULL;
    }

    length = strlen(src);
    dst = malloc(sizeof(char) * (length + 1));
    dst[length] = 0;

    return strcpy(dst, src);
}

char * strset(char ** dst, const char * src) {
    free(*dst);

    if (!src) {
        return (*dst = NULL);
    }

    return strcpy(*dst = malloc(sizeof(char) * strlen(src)), src);
}
