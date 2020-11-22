#pragma once

#include <stdbool.h>
#include <stdint.h>

struct value {
    enum {
        V_INTEGER,
        V_FLOATING,
        V_STRING,
        V_IDENTIFIER
    } type;

    union {
        int64_t integer;
        double floating;
        char * string;
        void * identifier;
    } value;
};

struct value value_from_integer(int64_t value);
struct value value_from_floating(double value);
struct value value_from_string(const char * value);
struct value value_from_identifier(void * value);
void value_discard(struct value value);

bool value_is_integer(struct value value);
bool value_is_floating(struct value value);
bool value_is_string(struct value value);
bool value_is_identifier(struct value value);

int64_t value_to_integer(struct value value);
double value_to_floating(struct value value);
const char * value_to_string(struct value value);
void * value_to_identifier(struct value value);
