#include "php.h"
#include "ext/standard/info.h"
#include <bstring.h>
#include <parser.h>
#include <lexer.h>
#include <stdio.h>
#include <ast.h>
#include <phpmodule.h>

PHP_MODULE_BEGIN()

extern ast_node* ast_root;
static int has_error;
static bstring* error_string;

void process_node(zval* output, ast_node* node) {
  zval* arr_children;
  zval* temp;
  
  array_init(output);
  
  add_assoc_stringl(output, "type", node->node_type->data, node->node_type->slen, 1);
  switch (node->data_type) {
    case DATA_TYPE_NONE:
      add_assoc_null(output, "data");
      break;
    case DATA_TYPE_TOKEN:
      add_assoc_null(output, "data");
      break;
    case DATA_TYPE_NUMBER:
      add_assoc_long(output, "data", node->data.number);
      break;
    case DATA_TYPE_STRING:
      add_assoc_string(output, "data", node->data.string->data, node->data.string->slen);
      break;
  }
  
  ALLOC_INIT_ZVAL(arr_children);
  array_init(arr_children);
  list_iterator_start(node->children);
  while (list_iterator_hasnext(node->children)) {
    ALLOC_INIT_ZVAL(temp);
    process_node(temp, (ast_node*)list_iterator_next(node->children));
    add_next_index_zval(arr_children, temp);
  }
  add_assoc_zval(output, "children", arr_children);
}

void yyerror(char const *s)
{
  has_error = 1;
  fprintf(stderr, "%s\n", s);
  
  // TODO error_string
}

int yywrap()
{
  return 1;
}

PHP_FUNCTION(omnilang_parse) {
  char* code;
  int code_len;
  YY_BUFFER_STATE buffer;
  
  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &code, &code_len) == FAILURE) {
    RETURN_FALSE;
  }
  
  yyin = NULL;
  yyout = NULL;
  has_error = 0;
  buffer = yy_scan_bytes(code, code_len);
  yyparse();
  yy_delete_buffer(buffer);
  
  if (has_error) {
    RETURN_FALSE;
  }
  
  process_node(return_value, ast_root);
}

PHP_MODULE(omnilang, 
  PHP_FE(omnilang_parse, NULL)
)