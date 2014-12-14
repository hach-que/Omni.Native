%{

#define YYERROR_VERBOSE
#define YYDEBUG 1

#include <bstring.h>
#include <ast.h>

// Root node for the AST.
ast_node* ast_root;

%}

%union
{
  int number;
  enum yytokentype token;
  bstring string;
  void* node;
}

%token <token> TOKEN WHITESPACE CONTINUING_NEWLINE TERMINATING_NEWLINE PIPE
%token <token> PERCENT DOLLAR BEGIN_PAREN END_PAREN EQUALS BEGIN_SQUARE END_SQUARE
%token <token> BEGIN_BRACE END_BRACE COMMA COMMAND_PIPE SEMICOLON AMPERSAND
%token <number> NUMBER
%token <string> FRAGMENT SINGLE_QUOTED DOUBLE_QUOTED VARIABLE

%type <string> fragment_or_variable
%type <node> root statement pipeline instruction
%type <node> command arguments fragment fragments
%type <node> capture assignment pipe_call
%type <node> key_values expression

%nonassoc PIPE
%nonassoc AMPERSAND
%nonassoc WHITESPACE

%start root

%expect 0

%%

root:
  statement
  {
    ast_root = ast_node_create(&node_type_root);
    $$ = ast_root;
    
    if ($1 != NULL) {
      ast_node_append_child($$, $1);
    }
  } |
  root terminator statement
  {
    if ($3 != NULL) {
      ast_node_append_child($1, $3);
    }
  };
  
terminator:
  TERMINATING_NEWLINE |
  SEMICOLON ;

statement:
  pipeline AMPERSAND
  {
    $$ = $1;
    ast_node_set_string($$, bstrcpy(&pipeline_type_background));
  } |
  pipeline
  {
    $$ = $1;
    ast_node_set_string($$, bstrcpy(&pipeline_type_foreground));
  } |
  assignment
  {
    $$ = $1;
  } |
  {
    $$ = NULL;
  };

assignment:
  DOLLAR VARIABLE EQUALS fragments
  {
    ast_node* variable;
    variable = ast_node_create(&node_type_variable);
    ast_node_set_string(variable, bstrcpy($2));
    
    $$ = ast_node_create(&node_type_assignment);
    ast_node_append_child($$, variable);
    ast_node_append_child($$, $4);
  } |
  DOLLAR VARIABLE EQUALS pipe_call
  {
    ast_node* variable;
    variable = ast_node_create(&node_type_variable);
    ast_node_set_string(variable, bstrcpy($2));
    
    $$ = ast_node_create(&node_type_assignment);
    ast_node_append_child($$, variable);
    ast_node_append_child($$, $4);
  } ;
  
pipeline:
  instruction
  {
    $$ = ast_node_create(&node_type_pipeline);
    ast_node_append_child($$, $1);
  } |
  pipeline PIPE instruction
  {
    ast_node_append_child($1, $3);
  };
  
instruction:
  command
  {
    $$ = ast_node_create(&node_type_command);
    ast_node_append_child($$, $1);
  } |
  capture
  {
    $$ = 1;
  } |
  pipe_call
  {
    $$ = $1;
  };
  
capture:
  PERCENT BEGIN_PAREN command END_PAREN
  {
    $$ = ast_node_create(&node_type_capture);
    ast_node_append_child($$, $3);
  } |
  PERCENT BEGIN_SQUARE key_values END_SQUARE BEGIN_PAREN command END_PAREN
  {
    $$ = ast_node_create(&node_type_capture);
    ast_node_append_child($$, $3);
    ast_node_append_child($$, $6);
  };
  
key_values:
  fragment_or_variable EQUALS fragment
  {
    ast_node* key_value;
    $$ = ast_node_create(&node_type_key_values);
    key_value = ast_node_create(&node_type_key_value);
    ast_node_set_string(key_value, bstrcpy($1));
    ast_node_append_child(key_value, $3);
    ast_node_append_child($$, key_value);
  } |
  key_values COMMA fragment_or_variable EQUALS fragment
  {
    ast_node* key_value;
    key_value = ast_node_create(&node_type_key_value);
    ast_node_set_string(key_value, bstrcpy($3));
    ast_node_append_child(key_value, $5);
    ast_node_append_child($1, key_value);
  };
  
command:
  arguments
  {
    $$ = $1;
  };
  
pipe_call:
  COMMAND_PIPE WHITESPACE arguments
  {
    $$ = ast_node_create(&node_type_pipe_command);
    ast_node_append_child($$, $3);
  } |
  COMMAND_PIPE
  {
    $$ = ast_node_create(&node_type_pipe_command);
  } ;

arguments:
  fragments
  {
    $$ = ast_node_create(&node_type_arguments);
    ast_node_append_child($$, $1);
  } |
  arguments WHITESPACE fragments
  {
    ast_node_append_child($1, $3);
  };

fragments:
  fragment
  {
    $$ = ast_node_create(&node_type_fragments);
    ast_node_append_child($$, $1);
  } |
  fragments fragment
  {
    ast_node_append_child($1, $2);
  };
  
fragment:
  fragment_or_variable
  {
    $$ = ast_node_create(&node_type_fragment);
    ast_node_set_string($$, bstrcpy($1));
  } |
  SINGLE_QUOTED
  {
    $$ = ast_node_create(&node_type_single_quoted);
    ast_node_set_string($$, bstrcpy($1));
  } |
  DOUBLE_QUOTED
  {
    $$ = ast_node_create(&node_type_double_quoted);
    ast_node_set_string($$, bstrcpy($1));
  } |
  DOLLAR VARIABLE
  {
    $$ = ast_node_create(&node_type_variable);
    ast_node_set_string($$, bstrcpy($2));
  } |
  DOLLAR BEGIN_BRACE expression END_BRACE
  {
    $$ = $3;
  };

fragment_or_variable:
  FRAGMENT { $$ = $1; } |
  VARIABLE { $$ = $1; } ;
  
expression:
  fragment
  {
    $$ = $1;
  };
