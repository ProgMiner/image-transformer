#include "value.h"

#include <stdlib.h>
#include <stdio.h>

#include "util.h"

struct value value_from_integer(int64_t value) {
    struct value ret = { V_INTEGER };

    ret.value.integer = value;
    return ret;
}

struct value value_from_floating(double value) {
    struct value ret = { V_FLOATING };

    ret.value.floating = value;
    return ret;
}

struct value value_from_string(const char * value) {
    struct value ret = { V_STRING };

    ret.value.string = strdup(value);
    return ret;
}

struct value value_from_identifier(void * value) {
    struct value ret = { V_IDENTIFIER };

    ret.value.identifier = value;
    return ret;
}

void value_discard(struct value value) {
    if (value.type == V_STRING) {
        free(value.value.string);
    }
}

int64_t value_parse_integer(const char * string) {
    int64_t value;

    if (sscanf(string, "%ld", &value) != 1) {
        return 0;
    }

    return value;
}

double value_parse_floating(const char * string) {
    char * end;

    double value = strtod(string, &end);

    return end == string ? .0 : value;
}

const char * value_integer_to_string(int64_t integer) {
    static char value[24];

    sprintf(value, "%ld", integer);
    return value;
}

const char * value_floating_to_string(double floating) {
    static char value[256];

    sprintf(value, "%.127f", floating);
    return value;
}

bool value_is_integer(struct value value) {
    int64_t buffer;

    switch (value.type) {
    case V_INTEGER:
        return true;

    case V_FLOATING:
        return (double) ((int64_t) value.value.floating) == value.value.floating;

    case V_STRING:
        return sscanf(value.value.string, "%ld", &buffer) == 1;

    default:
        return false;
    }
}

bool value_is_floating(struct value value) {
    char * end;

    switch (value.type) {
    case V_INTEGER:
    case V_FLOATING:
        return true;

    case V_STRING:
        strtod(value.value.string, &end);
        return end != value.value.string;

    default:
        return false;
    }
}

bool value_is_string(struct value value) {
    switch (value.type) {
    case V_INTEGER:
    case V_FLOATING:
    case V_STRING:
        return true;

    default:
        return false;
    }
}

bool value_is_identifier(struct value value) {
    return value.type == V_IDENTIFIER;
}

int64_t value_to_integer(struct value value) {
    switch (value.type) {
    case V_INTEGER:
        return value.value.integer;

    case V_FLOATING:
        return (int64_t) value.value.floating;

    case V_STRING:
        return value_parse_integer(value.value.string);

    default:
        return 0;
    }
}

double value_to_floating(struct value value) {
    switch (value.type) {
    case V_INTEGER:
        return (double) value.value.integer;

    case V_FLOATING:
        return value.value.floating;

    case V_STRING:
        return value_parse_floating(value.value.string);

    default:
        return .0;
    }
}

const char * value_to_string(struct value value) {
    switch (value.type) {
    case V_INTEGER:
        return value_integer_to_string(value.value.integer);

    case V_FLOATING:
        return value_floating_to_string(value.value.floating);

    case V_STRING:
        return value.value.string;

    default:
        return "";
    }
}

void * value_to_identifier(struct value value) {
    return value.type == V_IDENTIFIER
        ? value.value.identifier
        : NULL;
}
