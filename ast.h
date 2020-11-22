#pragma once

#include <stdint.h>

struct ast_position {
    uint32_t row;
    uint32_t col;
};

enum ast_literal_type {
    L_INTEGER,
    L_FLOATING,
    L_STRING,
    L_IDENTIFIER
};

struct ast_literal {
    enum ast_literal_type type;
    char * value;

    struct ast_position pos;
};

struct ast_transformation_args {
    struct ast_literal argument;
    struct ast_transformation_args * next;
};

struct ast_transformation {
    char * module;
    char * name;

    struct ast_transformation_args * args;

    struct ast_position pos;
};

struct ast_script {
    struct ast_transformation transformation;
    struct ast_script * next;
};

/* ast_position */

struct ast_position ast_position_create(uint32_t row, uint32_t col);

/* ast_literal */

struct ast_literal ast_literal_create(enum ast_literal_type type, char * value, struct ast_position pos);
void ast_literal_discard(struct ast_literal literal);

/* ast_transformation_args */

struct ast_transformation_args *
ast_transformation_args_new(struct ast_literal argument, struct ast_transformation_args * next);
void ast_transformation_args_delete(struct ast_transformation_args * transformation_args);

struct ast_transformation_args * ast_transformation_args_reverse(struct ast_transformation_args * transformation_args);

/* transformation */

struct ast_transformation
ast_transformation_create(char * module, char * name, struct ast_transformation_args * args, struct ast_position pos);
void ast_transformation_discard(struct ast_transformation transformation);

/* script */

struct ast_script * ast_script_new(struct ast_transformation transformation, struct ast_script * next);
void ast_script_delete(struct ast_script * script);

struct ast_script * ast_script_reverse(struct ast_script * script);
