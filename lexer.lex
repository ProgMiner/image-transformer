%option noyywrap noinput nounput

%{
#include <string.h>

#include "parser.h"
#include "util.h"

extern YYLTYPE yylloc;

int fileno(FILE *stream);

void update_yylloc() {
    size_t i;

    yylloc.first_line = yylloc.last_line;
    yylloc.first_column = yylloc.last_column;

    for (i = 0; i < yyleng; ++i) {
        switch (yytext[i]) {
        case '\n':
            ++yylloc.last_line;
            yylloc.last_column = 1;
            break;

        case '\t':
            yylloc.last_column += 8;
            break;

        default:
            ++yylloc.last_column;
        }
    }
}
%}

S [ \b\n\t\f\r]
W [a-zA-Z_]
D [0-9]

I {W}({W}|{D})*

%%

{S}         update_yylloc();
#.*$        update_yylloc();

{I}         update_yylloc(); yylval.token = strdup(yytext); return T_IDENTIFIER;

{D}+                update_yylloc(); yylval.token = strdup(yytext); return T_INTEGER;
{D}*\.{D}+          update_yylloc(); yylval.token = strdup(yytext); return T_FLOATING;
\"(\\.|[^"\\])*\"   update_yylloc(); yylval.token = strdup(yytext); return T_STRING;

.           update_yylloc(); return yytext[0];
