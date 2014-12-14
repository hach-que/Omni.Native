%{

#include <bstring.h>
#include "parser.h"

#define YYDEBUG 1

#define OD(s) { fprintf(stderr, s "\n"); }

%}

%option yylineno
%option case-insensitive

%x IN_SINGLE_STRING
%x IN_DOUBLE_STRING

%%

    /* Comments */

\#[^\n]*\n                              /* comment */;

    /* String handling */

\'                                      { yylval.string = bfromcstr(""); BEGIN(IN_SINGLE_STRING); }
\"                                      { yylval.string = bfromcstr(""); BEGIN(IN_DOUBLE_STRING); }
<IN_SINGLE_STRING>[^'\r\n]*             { bcatcstr(yylval.string, yytext); }
<IN_SINGLE_STRING>\'                    { BEGIN(INITIAL); return SINGLE_QUOTED; }
<IN_DOUBLE_STRING>[^"\r\n]*             { bcatcstr(yylval.string, yytext); }
<IN_DOUBLE_STRING>\"                    { BEGIN(INITIAL); return DOUBLE_QUOTED; }

pipe                                    { return COMMAND_PIPE; }

[a-zA-Z_][a-zA-Z_0-9]*                  { yylval.string = bfromcstr(yytext); return VARIABLE; }
[a-zA-Z\/_\.\-][a-zA-Z\/_0-9\.\-]*      { yylval.string = bfromcstr(yytext); return FRAGMENT; }

%                                       { return PERCENT; }
\$                                      { return DOLLAR; }
[ \t]*\([ \t]*                          { return BEGIN_PAREN; }
[ \t]*\)[ \t]*                          { return END_PAREN; }
[ \t]*\{[ \t]*                          { return BEGIN_BRACE; }
[ \t]*\}[ \t]*                          { return END_BRACE; }
[ \t]*\[[ \t]*                          { return BEGIN_SQUARE; }
[ \t]*\][ \t]*                          { return END_SQUARE; }

[ \t]*&[ \t]*                           { return AMPERSAND; }
[ \t]*\|[ \t]*                          { return PIPE; }
[ \t]*\=[ \t]*                          { return EQUALS; }
[ \t]*\,[ \t]*                          { return COMMA; }
[ \t]*\;[ \t]*                          { return SEMICOLON; }

[ \t]+                                  { return WHITESPACE; }
\\[ \t]*(\r\n|\r|\n)[ \t]*              { return CONTINUING_NEWLINE; }
[ \t]*(\r\n|\r|\n)[ \t]*                { return TERMINATING_NEWLINE; }
