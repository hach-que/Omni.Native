%{

#define YYERROR_VERBOSE
#define YYDEBUG 1

#include <bstring.h>
#include <ast.h>

#define VALUE(a) (a).value
#define VALUE_NODE(a) (a).node
#define ORIGINAL_INIT(a) { (a).original = bfromcstr(""); }
#define ORIGINAL_APPEND(a, b) { if ((a).original == NULL) { (a).original = bfromcstr(""); } bconcat((a).original, (b).original); }
#define ORIGINAL_NODE_APPEND(a, b) { bconcat(((ast_node*)((a).node))->original, (b).original); }
#define ORIGINAL_NODE_NODE_APPEND(a, b) { bconcat(((ast_node*)((a).node))->original, ((ast_node*)((b).node))->original); }

// Root node for the AST.
ast_node* ast_root;

%}

%union
{
  struct bstring_with_original {
    bstring value;
    bstring original;
  } string;
  struct number_with_original {
    int value;
    bstring original;
  } number;
  struct token_with_original {
    enum yytokentype value;
    bstring original;
  } token;
  struct node_with_original {
    void* node;
  } node;
}

%token <token> TOKEN WHITESPACE CONTINUING_NEWLINE TERMINATING_NEWLINE PIPE
%token <token> PERCENT DOLLAR BEGIN_PAREN END_PAREN EQUALS BEGIN_SQUARE END_SQUARE
%token <token> BEGIN_BRACE END_BRACE COMMA COMMAND_PIPE SEMICOLON AMPERSAND ERROR
%token <token> ACCESS MAP MINUS ADD MULTIPLY DIVIDE 
%token <string> KEYWORD_WHILE KEYWORD_DO KEYWORD_FOR KEYWORD_FOREACH KEYWORD_BREAK
%token <string> KEYWORD_CONTINUE KEYWORD_IF KEYWORD_ELSE
%token <number> NUMBER
%token <string> FRAGMENT SINGLE_QUOTED DOUBLE_QUOTED VARIABLE

%type <string> fragment_or_variable keyword_as_string
%type <node> root statement pipeline instruction
%type <node> command arguments fragment fragments
%type <node> capture assignment comma_arguments
%type <node> key_values expression number statements
%type <token> terminator optional_whitespace

%nonassoc PIPE
%nonassoc AMPERSAND
%nonassoc WHITESPACE
%nonassoc COMMA
%left ADD MINUS
%left MULTIPLY DIVIDE
%left BEGIN_PAREN
%nonassoc ACCESS
//%right KEYWORD_IF 
//%right KEYWORD_ELSE
//%nonassoc TERMINATING_NEWLINE

%start root

%expect 0

%%

root:
  statements
  {
    ast_root = VALUE_NODE($1);
  } ;

statements:
  statement
  {
    VALUE_NODE($$) = ast_node_create(&node_type_root);
    
    if (VALUE_NODE($1) != NULL) {
      ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
      ORIGINAL_NODE_NODE_APPEND($$, $1);
    }
  } |
  statements terminator statement
  {
    ORIGINAL_NODE_APPEND($1, $2);
      
    if (VALUE_NODE($3) != NULL) {
      ast_node_append_child(VALUE_NODE($1), VALUE_NODE($3));
      ORIGINAL_NODE_NODE_APPEND($1, $3);
    }
  } ;
  
terminator:
  TERMINATING_NEWLINE |
  SEMICOLON ;

optional_whitespace:
  /* empty */ |
  WHITESPACE |
  TERMINATING_NEWLINE ;
  
statement:
  pipeline AMPERSAND
  {
    VALUE_NODE($$) = VALUE_NODE($1);
    ast_node_set_string(VALUE_NODE($$), bstrcpy(&pipeline_type_background));
    
    ORIGINAL_NODE_APPEND($$, $2);
  } |
  pipeline
  {
    VALUE_NODE($$) = VALUE_NODE($1);
    ast_node_set_string(VALUE_NODE($$), bstrcpy(&pipeline_type_foreground));
  } |
  assignment
  {
    $$ = $1;
  } |
  KEYWORD_IF optional_whitespace fragment optional_whitespace BEGIN_BRACE statements END_BRACE KEYWORD_ELSE BEGIN_BRACE statements END_BRACE
  {
    VALUE_NODE($$) = ast_node_create(&node_type_if);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($6));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($10));
  } |
  KEYWORD_IF optional_whitespace fragment optional_whitespace BEGIN_BRACE statements END_BRACE
  {
    VALUE_NODE($$) = ast_node_create(&node_type_if);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($6));
  }
  /* empty */ |
  {
    VALUE_NODE($$) = NULL;
  };

assignment:
  DOLLAR VARIABLE EQUALS fragments
  {
    ast_node* variable;
    variable = ast_node_create(&node_type_variable);
    ast_node_set_string(variable, bstrcpy(VALUE($2)));
    
    VALUE_NODE($$) = ast_node_create(&node_type_assignment);
    ast_node_append_child(VALUE_NODE($$), variable);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($4));
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_APPEND($$, $3);
    ORIGINAL_NODE_NODE_APPEND($$, $4);
  } ;
  
pipeline:
  instruction
  {
    VALUE_NODE($$) = ast_node_create(&node_type_pipeline);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
  } |
  pipeline PIPE instruction
  {
    ast_node_append_child(VALUE_NODE($1), VALUE_NODE($3));
    
    ORIGINAL_NODE_APPEND($1, $2);
    ORIGINAL_NODE_NODE_APPEND($1, $3);
  };
  
instruction:
  command
  {
    VALUE_NODE($$) = ast_node_create(&node_type_command);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
  } |
  capture
  {
    $$ = $1;
  } ;
  
capture:
  PERCENT BEGIN_PAREN command END_PAREN
  {
    VALUE_NODE($$) = ast_node_create(&node_type_capture);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
    ORIGINAL_NODE_APPEND($$, $4);
  } |
  PERCENT BEGIN_SQUARE key_values END_SQUARE BEGIN_PAREN command END_PAREN
  {
    VALUE_NODE($$) = ast_node_create(&node_type_capture);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($6));
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
    ORIGINAL_NODE_APPEND($$, $4);
    ORIGINAL_NODE_APPEND($$, $5);
    ORIGINAL_NODE_NODE_APPEND($$, $6);
    ORIGINAL_NODE_APPEND($$, $7);
  };
  
key_values:
  fragment_or_variable EQUALS fragment
  {
    ast_node* key_value;
    VALUE_NODE($$) = ast_node_create(&node_type_key_values);
    key_value = ast_node_create(&node_type_key_value);
    ast_node_set_string(key_value, bstrcpy(VALUE($1)));
    ast_node_append_child(key_value, VALUE_NODE($3));
    ast_node_append_child(VALUE_NODE($$), key_value);
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
  } |
  key_values COMMA fragment_or_variable EQUALS fragment
  {
    ast_node* key_value;
    key_value = ast_node_create(&node_type_key_value);
    ast_node_set_string(key_value, bstrcpy(VALUE($3)));
    ast_node_append_child(key_value, VALUE_NODE($5));
    ast_node_append_child(VALUE_NODE($1), key_value);
    
    ORIGINAL_NODE_APPEND($1, $2);
    ORIGINAL_NODE_APPEND($1, $3);
    ORIGINAL_NODE_APPEND($1, $4);
    ORIGINAL_NODE_NODE_APPEND($1, $5);
  };
  
command:
  arguments
  {
    $$ = $1;
  };

arguments:
  fragments
  {
    VALUE_NODE($$) = ast_node_create(&node_type_arguments);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
  } |
  arguments WHITESPACE fragments
  {
    ast_node_append_child(VALUE_NODE($1), VALUE_NODE($3));
    
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
  };

fragments:
  fragment
  {
    VALUE_NODE($$) = ast_node_create(&node_type_fragments);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
  } |
  fragments fragment
  {
    ast_node_append_child(VALUE_NODE($1), VALUE_NODE($2));
    
    ORIGINAL_NODE_NODE_APPEND($1, $2);
  } |
  fragments keyword_as_string
  {
    ast_node* fragment = ast_node_create(&node_type_fragment);
    ast_node_set_string(fragment, bstrcpy(VALUE($2)));
    
    fragment->original = bstrcpy(VALUE($2));
    
    ast_node_append_child(VALUE_NODE($1), fragment);
    
    ORIGINAL_NODE_APPEND($1, $2);
  };
  
fragment:
  fragment_or_variable
  {
    VALUE_NODE($$) = ast_node_create(&node_type_fragment);
    ast_node_set_string(VALUE_NODE($$), bstrcpy(VALUE($1)));
    
    ORIGINAL_NODE_APPEND($$, $1);
  } |
  number
  {
    $$ = $1;
  } |
  MINUS
  {
    VALUE_NODE($$) = ast_node_create(&node_type_fragment);
    ast_node_set_string(VALUE_NODE($$), bfromcstr("-"));
    
    ORIGINAL_NODE_APPEND($$, $1);
  } |
  SINGLE_QUOTED
  {
    VALUE_NODE($$) = ast_node_create(&node_type_single_quoted);
    ast_node_set_string(VALUE_NODE($$), bstrcpy(VALUE($1)));
    
    ORIGINAL_NODE_APPEND($$, $1);
  } |
  DOUBLE_QUOTED
  {
    VALUE_NODE($$) = ast_node_create(&node_type_double_quoted);
    ast_node_set_string(VALUE_NODE($$), bstrcpy(VALUE($1)));
    
    ORIGINAL_NODE_APPEND($$, $1);
  } |
  DOLLAR VARIABLE
  {
    VALUE_NODE($$) = ast_node_create(&node_type_variable);
    ast_node_set_string(VALUE_NODE($$), bstrcpy(VALUE($2)));
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
  } |
  DOLLAR NUMBER
  {
    VALUE_NODE($$) = ast_node_create(&node_type_variable);
    ast_node_set_number(VALUE_NODE($$), VALUE($2));
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
  } |
  DOLLAR BEGIN_PAREN pipeline END_PAREN
  {
    VALUE_NODE($$) = ast_node_create(&node_type_expression);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    
    ast_node_set_string(VALUE_NODE($3), bstrcpy(&pipeline_type_expression));
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
    ORIGINAL_NODE_APPEND($$, $4);
  } |
  BEGIN_PAREN expression END_PAREN
  {
    VALUE_NODE($$) = ast_node_create(&node_type_expression);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($2));
    
    ORIGINAL_NODE_APPEND($$, $1);
    ORIGINAL_NODE_NODE_APPEND($$, $2);
    ORIGINAL_NODE_APPEND($$, $3);
  } ;
  
number:
  NUMBER
  {
    VALUE_NODE($$) = ast_node_create(&node_type_number);
    ast_node_set_number(VALUE_NODE($$), VALUE($1));
    
    ORIGINAL_NODE_APPEND($$, $1);
  } ;
  
keyword_as_string:
  KEYWORD_IF       { $$ = $1; } |
  KEYWORD_ELSE     { $$ = $1; } |
  KEYWORD_WHILE    { $$ = $1; } |
  KEYWORD_FOR      { $$ = $1; } |
  KEYWORD_FOREACH  { $$ = $1; } |
  KEYWORD_DO       { $$ = $1; } |
  KEYWORD_BREAK    { $$ = $1; } |
  KEYWORD_CONTINUE { $$ = $1; } ;

fragment_or_variable:
  FRAGMENT { $$ = $1; } |
  VARIABLE { $$ = $1; } ;
  
expression:
  fragment
  {
    VALUE_NODE($$) = VALUE_NODE($1);
  } |
  expression ACCESS expression
  {
    VALUE_NODE($$) = ast_node_create(&node_type_access);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
  } |
  expression BEGIN_PAREN comma_arguments END_PAREN
  {
    VALUE_NODE($$) = ast_node_create(&node_type_invocation);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
    ORIGINAL_NODE_APPEND($$, $4);
  } |
  expression DIVIDE expression
  {
    VALUE_NODE($$) = ast_node_create(&node_type_divide);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
  } |
  expression MULTIPLY expression
  {
    VALUE_NODE($$) = ast_node_create(&node_type_multiply);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
  } |
  expression ADD expression
  {
    VALUE_NODE($$) = ast_node_create(&node_type_add);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($3));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
  } |
  expression WHITESPACE MINUS expression
  {
    VALUE_NODE($$) = ast_node_create(&node_type_minus);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($4));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_APPEND($$, $3);
    ORIGINAL_NODE_NODE_APPEND($$, $4);
  } ;

comma_arguments:
  fragments
  {
    VALUE_NODE($$) = ast_node_create(&node_type_arguments);
    ast_node_append_child(VALUE_NODE($$), VALUE_NODE($1));
    
    ORIGINAL_NODE_NODE_APPEND($$, $1);
  } |
  comma_arguments COMMA fragments
  {
    ast_node_append_child(VALUE_NODE($1), VALUE_NODE($3));
    
    ORIGINAL_NODE_APPEND($$, $2);
    ORIGINAL_NODE_NODE_APPEND($$, $3);
  } ;