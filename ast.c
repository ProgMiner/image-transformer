#include "ast.h"

#include <stdlib.h>

struct ast_position ast_position_create(uint32_t row, uint32_t col) {
    struct ast_position pos;

    pos.row = row;
    pos.col = col;

    return pos;
}

struct ast_literal ast_literal_create(enum ast_literal_type type, char * value, struct ast_position pos) {
    struct ast_literal literal;

    literal.type = type;
    literal.value = value;
    literal.pos = pos;

    return literal;
}

void ast_literal_discard(struct ast_literal literal) {
    free(literal.value);
}

struct ast_transformation_args *
ast_transformation_args_new(struct ast_literal argument, struct ast_transformation_args * next) {
    struct ast_transformation_args * args = malloc(sizeof(struct ast_transformation_args));

    args->argument = argument;
    args->next = next;

    return args;
}

void ast_transformation_args_delete(struct ast_transformation_args * transformation_args) {
    struct ast_transformation_args * next = transformation_args;
    struct ast_transformation_args * current;

    while (next) {
        current = next;
        next = current->next;

        ast_literal_discard(current->argument);
        free(current);
    }
}

struct ast_transformation_args *
ast_transformation_args_reverse(struct ast_transformation_args * transformation_args) {
    struct ast_transformation_args * next = transformation_args;
    struct ast_transformation_args * result = NULL;
    struct ast_transformation_args * current;

    while (next) {
        current = next;
        next = current->next;

        result = ast_transformation_args_new(current->argument, result);
        free(current);
    }

    return result;
}

struct ast_transformation
ast_transformation_create(char * module, char * name, struct ast_transformation_args * args, struct ast_position pos) {
    struct ast_transformation transformation;

    transformation.module = module;
    transformation.name = name;
    transformation.args = args;
    transformation.pos = pos;

    return transformation;
}

void ast_transformation_discard(struct ast_transformation transformation) {
    free(transformation.module);
    free(transformation.name);

    ast_transformation_args_delete(transformation.args);
}

struct ast_script * ast_script_new(struct ast_transformation transformation, struct ast_script * next) {
    struct ast_script * script = malloc(sizeof(struct ast_script));

    script->transformation = transformation;
    script->next = next;

    return script;
}

void ast_script_delete(struct ast_script * script) {
    struct ast_script * next = script;
    struct ast_script * current;

    while (next) {
        current = next;
        next = current->next;

        ast_transformation_discard(current->transformation);
        free(current);
    }
}

struct ast_script * ast_script_reverse(struct ast_script * script) {
    struct ast_script * next = script;
    struct ast_script * result = NULL;
    struct ast_script * current;

    while (next) {
        current = next;
        next = current->next;
        result = ast_script_new(current->transformation, result);
        free(current);
    }

    return result;
}
