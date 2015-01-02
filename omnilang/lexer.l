%{

#include <bstring.h>
#include "parser.h"

#define YYDEBUG 1

#define OD(s) { fprintf(stderr, s "\n"); }
#define OCOPY(s) { yylval.s.original = bfromcstr(yytext); }
#define OCAT(s) { bcatcstr(yylval.s.original, yytext); }

void reset_to_initial();

%}

%option yylineno
%option case-insensitive

%x IN_SINGLE_STRING
%x IN_DOUBLE_STRING
%x IN_PHP

%%

    /* Comments */

\#[^\n]*\n                              /* comment */;

    /* String handling */

\'                                      { OCOPY(string); yylval.string.value = bfromcstr(""); BEGIN(IN_SINGLE_STRING); }
\"                                      { OCOPY(string); yylval.string.value = bfromcstr(""); BEGIN(IN_DOUBLE_STRING); }
<IN_SINGLE_STRING>[^'\r\n]*             { OCAT(string); bcatcstr(yylval.string.value, yytext); }
<IN_SINGLE_STRING>\'                    { OCAT(string); BEGIN(INITIAL); return SINGLE_QUOTED; }
<IN_SINGLE_STRING><<EOF>>               { return UNTERMINATED_LEXING_BLOCK; }
<IN_DOUBLE_STRING>[^"\r\n]*             { OCAT(string); bcatcstr(yylval.string.value, yytext); }
<IN_DOUBLE_STRING>\"                    { OCAT(string); BEGIN(INITIAL); return DOUBLE_QUOTED; }
<IN_DOUBLE_STRING><<EOF>>               { return UNTERMINATED_LEXING_BLOCK; }

\<\?php                                 { OCOPY(string); BEGIN(IN_PHP); yylval.string.value = bfromcstr(yytext); }
<IN_PHP>\?\>                            { OCAT(string); BEGIN(INITIAL); bcatcstr(yylval.string.value, yytext); return PHP; }
<IN_PHP>.|\r|\n                         { OCAT(string); bcatcstr(yylval.string.value, yytext); }
<IN_PHP><<EOF>>                         { return UNTERMINATED_LEXING_BLOCK; }

[0-9]+                                  { OCOPY(number); yylval.number.value = strtoul(yytext, NULL, 10); return NUMBER; }

if                                      { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_IF; }
else                                    { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_ELSE; }
while                                   { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_WHILE; }
for                                     { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_FOR; }
foreach                                 { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_FOREACH; }
do                                      { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_DO; }
break                                   { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_BREAK; }
continue                                { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_CONTINUE; }
as                                      { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_AS; }
return                                  { OCOPY(string); yylval.string.value = bfromcstr(yytext); return KEYWORD_RETURN; }

[a-zA-Z0-9_][a-zA-Z_0-9]*               { OCOPY(string); yylval.string.value = bfromcstr(yytext); return VARIABLE; }
[a-zA-Z\/_\.\~]                         { OCOPY(string); yylval.string.value = bfromcstr(yytext); return FRAGMENT; }
[a-zA-Z\/_\.\-\~][a-zA-Z\/_0-9\.\-\~]*[a-zA-Z\/_0-9\.\~]  { OCOPY(string); yylval.string.value = bfromcstr(yytext); return FRAGMENT; }

%                                       { OCOPY(token); return PERCENT; }
\$                                      { OCOPY(token); return DOLLAR; }
\?                                      { OCOPY(string); yylval.string.value = bfromcstr(yytext); return QUESTION_MARK; }
@                                       { OCOPY(token); return AT; }
[ \t]*\-\>[ \t]*                        { OCOPY(token); return ACCESS; }
[ \t]*\=\>[ \t]*                        { OCOPY(token); return MAP; }
\([ \t]*                                { OCOPY(token); return BEGIN_PAREN; }
[ \t]*\)                                { OCOPY(token); return END_PAREN; }
[ \t]*\{[ \t]*                          { OCOPY(token); return BEGIN_BRACE; }
[ \t]*\}[ \t]*                          { OCOPY(token); return END_BRACE; }
[ \t]*\[[ \t]*                          { OCOPY(token); return BEGIN_SQUARE; }
[ \t]*\][ \t]*                          { OCOPY(token); return END_SQUARE; }

[ \t]*&[ \t]*                           { OCOPY(token); return AMPERSAND; }
[ \t]*\|[ \t]*                          { OCOPY(token); return PIPE; }
[ \t]*\=[ \t]*                          { OCOPY(token); return EQUALS; }
[ \t]*\,[ \t]*                          { OCOPY(token); return COMMA; }
[ \t]*\;[ \t]*                          { OCOPY(token); return SEMICOLON; }
[ \t]*:[ \t]*                           { OCOPY(token); return COLON; }

[ \t]*\+[ \t]*                          { OCOPY(token); return ADD; }
\-[ \t]*                                { OCOPY(token); return MINUS; }
[ \t]*\*[ \t]*                          { OCOPY(token); return MULTIPLY; }
[ \t]*\/[ \t]*                          { OCOPY(token); return DIVIDE; }

[ \t]+                                  { OCOPY(token); return WHITESPACE; }
\\[ \t]*(\r\n|\r|\n)[ \t]*              { OCOPY(token); return CONTINUING_NEWLINE; }
[ \t]*(\r\n|\r|\n)[ \t]*                { OCOPY(token); return TERMINATING_NEWLINE; }

    /* Default rule for unknown characters */

<*>.|\r|\n                              { printf("unable to handle text %s in lexer!\n", yytext); return ERROR; }

%%

void reset_to_initial() {
  BEGIN(INITIAL);
}