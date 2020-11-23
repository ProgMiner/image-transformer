#include "interpreter.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "value.h"
#include "util.h"

typedef const char * (* transformation_function)(struct image * image, uint32_t argc, const struct value * argv);

struct interpreter_ids {
    const char * module;
    const char * name;

    void * handle;
    void * symbol;

    struct interpreter_ids * next;
};

struct interpreter_ids *
interpreter_ids_new(const char * module, const char * name, void * handle, void * symbol, struct interpreter_ids * next);
void interpreter_ids_delete(struct interpreter_ids * interpreter_ids);

const struct interpreter_ids *
interpreter_ids_lookup(const struct interpreter_ids * interpreter_ids, const char * module, const char * name);

struct interpreter interpreter_create(const struct ast_script * script) {
    struct interpreter interpreter;

    interpreter.modules_prefix = "";
    interpreter.script = script;
    interpreter.identifiers = NULL;

    return interpreter;
}

void interpreter_discard(struct interpreter interpreter) {
    interpreter_ids_delete(interpreter.identifiers);
}

const char * interpreter_do_load_module(void ** handle, const char * filename) {
    if (!(*handle = dlopen(filename, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND))) {
        return dlerror();
    }

    return NULL;
}

const char * interpreter_load_module(const struct interpreter interpreter, void ** handle, const char * module) {
    size_t prefix_length, module_length;
    static char * error = NULL;
    char * filename;

    if (!module) {
        return strset(&error, interpreter_do_load_module(handle, NULL));
    }

    prefix_length = strlen(interpreter.modules_prefix);
    module_length = strlen(module);

    filename = malloc(sizeof(char) * (prefix_length + module_length + 4));

    sprintf(filename, "%s%s.so", interpreter.modules_prefix, module);
    if (!strset(&error, interpreter_do_load_module(handle, filename))) {
        free(filename);
        return NULL;
    }

    sprintf(filename, "%s%s", interpreter.modules_prefix, module);
    if (!interpreter_do_load_module(handle, filename)) {
        free(filename);
        return NULL;
    }

    free(filename);
    return error;
}

const char * interpreter_do_load_symbol(void ** symbol, void * handle, const char * name) {
    dlerror();

    *symbol = dlsym(handle, name);
    return dlerror();
}

const char * interpreter_load_symbol(struct interpreter * interpreter, const char * module, const char * name) {
    static char * error = NULL;
    void * handle;
    void * symbol;

    if (interpreter_ids_lookup(interpreter->identifiers, module, name)) {
        return NULL;
    }

    if (strset(&error, interpreter_load_module(*interpreter, &handle, module))) {
        return error;
    }

    if (strset(&error, interpreter_do_load_symbol(&symbol, handle, name))) {
        dlclose(handle);
        return error;
    }

    interpreter->identifiers = interpreter_ids_new(module, name, handle, symbol, interpreter->identifiers);
    return NULL;
}

const char * interpreter_print_pos(struct ast_position pos) {
    static char * result = NULL;

    free(result);
    result = malloc(sizeof(char) * 53);

    sprintf(result, "at %u:%u", pos.row, pos.col);
    return result;
}

const char *
interpreter_print_positional_error(struct ast_position pos, const char * message, const char * source) {
    static char * error = NULL;

    const char * pos_str = interpreter_print_pos(pos);

    free(error);
    error = malloc(sizeof(char) * (strlen(pos_str) + strlen(message) + strlen(source) + 5));

    sprintf(error, "%s: %s: %s", pos_str, message, source);
    return error;
}

const char * interpreter_process_script(struct interpreter * interpreter) {
    const struct ast_transformation_args * next_transformation_args;
    struct ast_transformation transformation;
    const struct ast_script * next_script;
    struct ast_literal literal;
    const char * error;

    for (next_script = interpreter->script; next_script; next_script = next_script->next) {
        transformation = next_script->transformation;

        if ((error = interpreter_load_symbol(interpreter, transformation.module, transformation.name))) {
            return interpreter_print_positional_error(transformation.pos, "cannot load transformation", error);
        }

        for (
            next_transformation_args = next_script->transformation.args;
            next_transformation_args;
            next_transformation_args = next_transformation_args->next
        ) {
            literal = next_transformation_args->argument;

            if (literal.type == L_IDENTIFIER) {
                if ((error = interpreter_load_symbol(interpreter, transformation.module, literal.value))) {
                    return interpreter_print_positional_error(literal.pos, "cannot load identifier", error);
                }
            }
        }
    }

    return NULL;
}

uint32_t interpreter_count_args(const struct ast_transformation_args * transformation_args) {
    uint32_t count = 0;

    for (; transformation_args; transformation_args = transformation_args->next) {
        ++count;
    }

    return count;
}

struct value interpreter_parse_string_value(const char * value) {
    size_t value_length = strlen(value);
    size_t i, j;

    char * result = malloc(sizeof(char) * (value_length - 1));
    struct value result_value;

    for (i = 1, j = 0; i < value_length - 1; ++i, ++j) {
        if (value[i] == '\\') {
            ++i;
        }

        result[j] = value[i];
    }

    result[j++] = '\0';
    result_value = value_from_string(result);

    free(result);
    return result_value;
}

struct value *
interpreter_collect_args(const struct interpreter interpreter, uint32_t * argc, const struct ast_transformation transformation) {
    struct value * args = malloc(sizeof(struct value) * (*argc = interpreter_count_args(transformation.args)));
    const struct ast_transformation_args * next = transformation.args;
    uint32_t i;

    for (i = 0; next; next = next->next, ++i) {
        switch (next->argument.type) {
        case L_INTEGER:
            args[i].type = V_INTEGER;
            sscanf(next->argument.value, "%ld", &(args[i].value.integer));
            break;

        case L_FLOATING:
            args[i].type = V_FLOATING;
            sscanf(next->argument.value, "%lf", &(args[i].value.floating));
            break;

        case L_STRING:
            args[i] = interpreter_parse_string_value(next->argument.value);
            break;

        case L_IDENTIFIER:
            args[i] = value_from_identifier(interpreter_ids_lookup(
                interpreter.identifiers,
                transformation.module,
                next->argument.value
            )->symbol);
            break;

        default: /* fallback */
            args[i].type = V_IDENTIFIER;
            args[i].value.identifier = NULL;
        }
    }

    return args;
}

void interpreter_delete_args(uint32_t argc, struct value * args) {
    uint32_t i;

    for (i = 0; i < argc; ++i) {
        value_discard(args[i]);
    }

    free(args);
}

const char * interpreter_run(const struct interpreter interpreter, struct image * image) {
    static char * transformation_name = NULL;
    const char * transformation_error;

    transformation_function transformation_function;
    struct ast_transformation transformation;
    const struct ast_script * next;
    struct value * args;
    uint32_t argc;

    for (next = interpreter.script; next; next = next->next) {
        transformation = next->transformation;

        args = interpreter_collect_args(interpreter, &argc, transformation);
        *((void **) (&transformation_function)) = interpreter_ids_lookup(
            interpreter.identifiers,
            transformation.module,
            transformation.name
        )->symbol;

        transformation_error = transformation_function(image, argc, args);
        interpreter_delete_args(argc, args);

        if (transformation_error) {
            free(transformation_name);

            transformation_name = malloc(sizeof(char) * ((transformation.module
                ? strlen(transformation.module) + 1 : 0) + strlen(transformation.name) + 1));

            if (transformation.module) {
                sprintf(transformation_name, "%s.%s", transformation.module, transformation.name);
            } else {
                sprintf(transformation_name, "%s", transformation.name);
            }

            return interpreter_print_positional_error(transformation.pos, transformation_name, transformation_error);
        }
    }

    return NULL;
}

struct interpreter_ids *
interpreter_ids_new(const char * module, const char * name, void * handle, void * symbol, struct interpreter_ids * next) {
    struct interpreter_ids * interpreter_ids = malloc(sizeof(struct interpreter_ids));

    interpreter_ids->module = module;
    interpreter_ids->name = name;
    interpreter_ids->handle = handle;
    interpreter_ids->symbol = symbol;
    interpreter_ids->next = next;

    return interpreter_ids;
}

void interpreter_ids_delete(struct interpreter_ids * interpreter_ids) {
    struct interpreter_ids * next = interpreter_ids;
    struct interpreter_ids * current;

    while (next) {
        current = next;
        next = current->next;

        dlclose(current->handle);
        free(current);
    }
}

const struct interpreter_ids *
interpreter_ids_lookup(const struct interpreter_ids * interpreter_ids, const char * module, const char * name) {
    const struct interpreter_ids * next;
    bool module_equals;

    for (next = interpreter_ids; next; next = next->next) {
        module_equals = next->module == NULL || module == NULL
            ? next->module == module : strcmp(next->module, module) == 0;

        if (module_equals && strcmp(next->name, name) == 0) {
            return next;
        }
    }

    return NULL;
}
