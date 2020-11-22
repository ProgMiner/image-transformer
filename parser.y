%parse-param {struct ast_script ** result} {char ** error}

%{
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

int yylex(void);

void yyerror(struct ast_script ** result, char ** error, const char * str);
%}

%code requires {
#include "ast.h"
}

%union {
    struct ast_script * script;
    struct ast_transformation transformation;
    struct ast_transformation_args * transformation_args;
    struct ast_literal literal;
    char * token;
}

%token T_IDENTIFIER T_INTEGER T_FLOATING T_STRING

%type<script> script
%type<transformation> transformation
%type<transformation_args> transformation_args transformation_args_req
%type<literal> literal
%type<token> T_IDENTIFIER T_INTEGER T_FLOATING T_STRING

%%

file
    : script YYEOF  { *result = ast_script_reverse($1); }
    ;

script
    : /* empty */               { $$ = NULL; }
    | script transformation ';' { $$ = ast_script_new($2, $1); }
    ;

transformation
    : T_IDENTIFIER '(' transformation_args ')'                  {
        $$ = ast_transformation_create(NULL, $1, ast_transformation_args_reverse($3),
                ast_position_create(@$.first_line, @$.first_column));
    }
    | T_IDENTIFIER '.' T_IDENTIFIER '(' transformation_args ')' {
        $$ = ast_transformation_create($1, $3, ast_transformation_args_reverse($5),
                ast_position_create(@$.first_line, @$.first_column));
    }
    ;

transformation_args
    : /* empty */               { $$ = NULL; }
    | transformation_args_req   { $$ = $1; }
    ;

transformation_args_req
    : literal                               { $$ = ast_transformation_args_new($1, NULL); }
    | transformation_args_req ',' literal   { $$ = ast_transformation_args_new($3, $1); }
    ;

literal
    : T_INTEGER     { $$ = ast_literal_create(L_INTEGER, $1, ast_position_create(@$.first_line, @$.first_column)); }
    | T_FLOATING    { $$ = ast_literal_create(L_FLOATING, $1, ast_position_create(@$.first_line, @$.first_column)); }
    | T_STRING      { $$ = ast_literal_create(L_STRING, $1, ast_position_create(@$.first_line, @$.first_column)); }
    | T_IDENTIFIER  { $$ = ast_literal_create(L_IDENTIFIER, $1, ast_position_create(@$.first_line, @$.first_column)); }
    ;

%%

void yyerror(struct ast_script ** result, char ** error, const char * str) {
    free(*error);

    *error = malloc(strlen(str) + 56);
    sprintf(*error, "at %d:%d: %s", yylloc.first_line, yylloc.first_column, str);
}
