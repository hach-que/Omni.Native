#ifndef __OMNI_LANG_AST_H
#define __OMNI_LANG_AST_H

#include <simclist.h>
#include <parser.h>
#include <bstring.h>

#define DATA_TYPE_NONE 0
#define DATA_TYPE_TOKEN 1
#define DATA_TYPE_NUMBER 2
#define DATA_TYPE_STRING 3

extern struct tagbstring node_type_root;
extern struct tagbstring node_type_statement;
extern struct tagbstring node_type_pipeline;
extern struct tagbstring node_type_instruction;
extern struct tagbstring node_type_command;
extern struct tagbstring node_type_capture;
extern struct tagbstring node_type_subshell;
extern struct tagbstring node_type_arguments;
extern struct tagbstring node_type_fragments;
extern struct tagbstring node_type_fragment;
extern struct tagbstring node_type_number;
extern struct tagbstring node_type_single_quoted;
extern struct tagbstring node_type_double_quoted;
extern struct tagbstring node_type_variable;
extern struct tagbstring node_type_assignment;
extern struct tagbstring node_type_key_values;
extern struct tagbstring node_type_key_value;
extern struct tagbstring node_type_expression;
extern struct tagbstring node_type_access;
extern struct tagbstring node_type_invocation;
extern struct tagbstring node_type_divide;
extern struct tagbstring node_type_multiply;
extern struct tagbstring node_type_add;
extern struct tagbstring node_type_minus;
extern struct tagbstring node_type_if;
extern struct tagbstring node_type_else;
extern struct tagbstring node_type_while;
extern struct tagbstring node_type_for;
extern struct tagbstring node_type_foreach;
extern struct tagbstring node_type_do;
extern struct tagbstring node_type_break;
extern struct tagbstring node_type_continue;
extern struct tagbstring node_type_php;

extern struct tagbstring pipeline_type_foreground;
extern struct tagbstring pipeline_type_background;
extern struct tagbstring pipeline_type_expression;

typedef struct 
{
    const_bstring node_type;
    int data_type;
    YYSTYPE data;
    list_t* children;
    bstring original;
} ast_node;

ast_node* ast_node_create(const_bstring node_type);
void ast_node_set_token(ast_node* node, int token);
void ast_node_set_number(ast_node* node, int number);
void ast_node_set_string(ast_node* node, bstring string);
void ast_node_append_child(ast_node* node, ast_node* child);

#endif