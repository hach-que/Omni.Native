#include <bstring.h>
#include <ast.h>
#include <stdlib.h>
#include <assert.h>

struct tagbstring node_type_root = bsStatic("root");
struct tagbstring node_type_statement = bsStatic("statement");
struct tagbstring node_type_pipeline = bsStatic("pipeline");
struct tagbstring node_type_instruction = bsStatic("instruction");
struct tagbstring node_type_command = bsStatic("command");
struct tagbstring node_type_capture = bsStatic("capture");
struct tagbstring node_type_subshell = bsStatic("subshell");
struct tagbstring node_type_arguments = bsStatic("arguments");
struct tagbstring node_type_fragments = bsStatic("fragments");
struct tagbstring node_type_fragment = bsStatic("fragment");
struct tagbstring node_type_number = bsStatic("number");
struct tagbstring node_type_single_quoted = bsStatic("single_quoted");
struct tagbstring node_type_double_quoted = bsStatic("double_quoted");
struct tagbstring node_type_variable = bsStatic("variable");
struct tagbstring node_type_assignment = bsStatic("assignment");
struct tagbstring node_type_key_values = bsStatic("key_values");
struct tagbstring node_type_key_value = bsStatic("key_value");
struct tagbstring node_type_expression = bsStatic("expression");
struct tagbstring node_type_access = bsStatic("access");
struct tagbstring node_type_invocation = bsStatic("invocation");
struct tagbstring node_type_divide = bsStatic("divide");
struct tagbstring node_type_multiply = bsStatic("multiply");
struct tagbstring node_type_add = bsStatic("add");
struct tagbstring node_type_minus = bsStatic("minus");

struct tagbstring pipeline_type_foreground = bsStatic("foreground");
struct tagbstring pipeline_type_background = bsStatic("background");
struct tagbstring pipeline_type_expression = bsStatic("expression");

ast_node* ast_node_create(const_bstring node_type) {
  ast_node* node;
  node = malloc(sizeof(ast_node));
  node->node_type = node_type;
  node->data_type = DATA_TYPE_NONE;
  node->data.number.value = 0;
  node->children = malloc(sizeof(list_t));
  node->original = bfromcstr("");
  list_init(node->children);
  return node;
}

void ast_node_set_token(ast_node* node, int token) {
  assert(node != NULL);
  node->data_type = DATA_TYPE_TOKEN;
  node->data.token.value = token;
}

void ast_node_set_number(ast_node* node, int number) {
  assert(node != NULL);
  node->data_type = DATA_TYPE_NUMBER;
  node->data.number.value = number;
}

void ast_node_set_string(ast_node* node, bstring string) {
  assert(node != NULL);
  node->data_type = DATA_TYPE_STRING;
  node->data.string.value = bstrcpy(string);
}

void ast_node_append_child(ast_node* node, ast_node* child) {
  assert(node != NULL);
  assert(child != NULL);
  list_append(node->children, child);
}