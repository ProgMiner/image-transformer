#pragma once

#include "ast.h"
#include "image.h"

struct interpreter_ids;

struct interpreter {
    const char * modules_prefix;

    const struct ast_script * script;
    struct interpreter_ids * identifiers;
};

struct interpreter interpreter_create(const struct ast_script * script);
void interpreter_discard(struct interpreter interpreter);

const char * interpreter_process_script(struct interpreter * interpreter);
const char * interpreter_run(const struct interpreter interpreter, struct image * image);
