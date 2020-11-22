#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>

#include "util.h"
#include "parser.h"
#include "interpreter.h"
#include "bmp.h"

typedef struct yy_buffer_state * YY_BUFFER_STATE;

YY_BUFFER_STATE yy_create_buffer(FILE * file, int size);
void yy_switch_to_buffer(YY_BUFFER_STATE new_buffer);
YY_BUFFER_STATE yy_scan_string(const char * str);
void yy_delete_buffer(YY_BUFFER_STATE buffer);

struct args {
    const char * script; /* script filename */
    const char * input; /* input BMP filename */
    const char * output; /* output BMP filename */

    bool code; /* assume that script is code instead of filename */
    char * modules_prefix; /* optional modules prefix */
    bool help; /* print help and exit */
};

struct args args_create() {
    struct args args = { NULL, "-", "-", false, NULL, false };
    return args;
}

void args_discard(struct args args) {
    free(args.modules_prefix);
}

bool parse_args(struct args * args, int argc, char ** argv) {
    static const char * const usage = ""
        "Usage: %s [-c] [-p <modules_prefix>] <script> [<input>] [<output>]\n"
        "Arguments:\n"
        "  - script - script filename\n"
        "  - input - input BMP filename or stdin if is - (default is -)\n"
        "  - output - output BMP filename or stdout if is - (default is -)\n"
        "Options:\n"
        "  - -c - assume that script is code instead of filename\n"
        "  - -p <modules_prefix> - set prefix for module files lookup "
        "(for example: if is ./, then all modules will be searching only in the working directory)\n";

    uint32_t i;
    int opt;

    while ((opt = getopt(argc, argv, "cp:h")) != -1) {
        switch (opt) {
        case 'c':
            args->code = true;
            break;

        case 'p':
            args->modules_prefix = strdup(optarg);
            break;

        case 'h':
            args->help = true;
            break;

        default:
            fprintf(stderr, usage, argv[0]);
            return false;
        }
    }

    if (args->help) {
        printf(usage, argv[0]);
        return true;
    }

    for (i = optind; i < argc; ++i) {
        switch (i - optind) {
        case 0:
            args->script = argv[i];
            continue;

        case 1:
            args->input = argv[i];
            continue;

        case 2:
            args->output = argv[i];
            continue;
        }

        fputs("There are some extra arguments on the tail, skipping.\n", stderr);
        break;
    }

    if (i - optind < 1) {
        fputs("Script is not specified.\n", stderr);
        fprintf(stderr, usage, argv[0]);
        return false;
    }

    return true;
}

bool parse_script(struct ast_script ** result, const char * script, bool code) {
    YY_BUFFER_STATE buffer_state;
    FILE * script_file = NULL;
    char * error = NULL;

    if (code) {
        buffer_state = yy_scan_string(script);
    } else {
        if (!(script_file = fopen(script, "r"))) {
            perror("Script file opening failed");
            return false;
        }

        buffer_state = yy_create_buffer(script_file, 16384);
        yy_switch_to_buffer(buffer_state);
    }

    if (yyparse(result, &error)) {
        fprintf(stderr, "Parsing failed: %s.\n", error);
        return false;
    }

    yy_delete_buffer(buffer_state);

    if (script_file) {
        if (fclose(script_file)) {
            perror("Script file closing failed");
            return false;
        }
    }

    return true;
}

bool init_interpreter(struct interpreter * interpreter, const struct ast_script * script, const char * modules_prefix) {
    const char * error;

    *interpreter = interpreter_create(script);

    if (modules_prefix) {
        interpreter->modules_prefix = modules_prefix;
    }

    if ((error = interpreter_process_script(interpreter))) {
        fprintf(stderr, "Script preprocessing failed: %s.\n", error);
        interpreter_discard(*interpreter);
        return false;
    }

    return true;
}

bool load_image(struct bmp_image * image, const char * filename) {
    bool stdinFilename = filename[0] == '-' && filename[1] == '\0';
    const char * error;
    FILE * file;

    if (stdinFilename) {
        file = stdin;
    } else {
        if (!(file = fopen(filename, "rb"))) {
            perror("Input file opening failed");
            return false;
        }
    }

    if ((error = bmp_image_read(image, file))) {
        fprintf(stderr, "Input file reading failed: %s.\n", error);
        return false;
    }

    if (stdinFilename) {
        if (fclose(file)) {
            perror("Input file closing failed");
            return false;
        }
    }

    return true;
}

bool run_interpreter(struct interpreter interpreter, struct image * image) {
    const char * error;

    if ((error = interpreter_run(interpreter, image))) {
        fprintf(stderr, "Interpretation failed: %s.\n", error);
        return false;
    }

    return true;
}

bool save_image(struct bmp_image image, const char * filename) {
    bool stdoutFilename = filename[0] == '-' && filename[1] == '\0';
    const char * error;
    FILE * file;

    if (stdoutFilename) {
        file = stdout;
    } else {
        if (!(file = fopen(filename, "wb"))) {
            perror("Output file opening failed");
            return false;
        }
    }

    if ((error = bmp_image_write(image, file))) {
        fprintf(stderr, "Output file writing failed: %s.\n", error);
        return false;
    }

    if (stdoutFilename) {
        if (fclose(file)) {
            perror("Output file closing failed");
            return false;
        }
    }

    return true;
}

int main(int argc, char ** argv) {
    struct args args = args_create();
    struct ast_script * script;
    struct interpreter interpreter;
    struct bmp_image bmp_image;
    struct image image;

    if (!parse_args(&args, argc, argv)) {
        return 1;
    }

    if (args.help) {
        return 0;
    }

    if (!parse_script(&script, args.script, args.code)) {
        args_discard(args);
        return 2;
    }

    if (!init_interpreter(&interpreter, script, args.modules_prefix)) {
        ast_script_delete(script);
        args_discard(args);
        return 3;
    }

    if (!load_image(&bmp_image, args.input)) {
        interpreter_discard(interpreter);
        ast_script_delete(script);
        args_discard(args);
        return 4;
    }

    image = bmp_image_to_image(bmp_image);

    if (!run_interpreter(interpreter, &image)) {
        image_discard(image);
        bmp_image_discard(bmp_image);
        interpreter_discard(interpreter);
        ast_script_delete(script);
        args_discard(args);
        return 5;
    }

    interpreter_discard(interpreter);
    ast_script_delete(script);

    bmp_image_replace(&bmp_image, image);
    image_discard(image);

    if (!save_image(bmp_image, args.output)) {
        bmp_image_discard(bmp_image);
        args_discard(args);
        return 6;
    }

    bmp_image_discard(bmp_image);
    args_discard(args);
    return 0;
}
