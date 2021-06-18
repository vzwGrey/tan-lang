#pragma once

#include "lexer.h"

typedef enum
{
  BINOP_ADD = TOKEN_PLUS,
  BINOP_SUB = TOKEN_MINUS,
  BINOP_MUL = TOKEN_STAR,
  BINOP_DIV = TOKEN_SLASH,
  BINOP_SEQ = TOKEN_COMMA,
} BINARY_OPERATION_KIND;

typedef enum
{
  NODE_CONSTANT_NUMBER,
  NODE_BINARY_OPERATION,
  NODE_ASSIGNMENT,
  NODE_VARIABLE,
} AST_NODE_KIND;

typedef struct AST_NODE
{
  AST_NODE_KIND kind;
  union
  {
    double constant_number;
    struct
    {
      struct AST_NODE *left;
      struct AST_NODE *right;
      BINARY_OPERATION_KIND op;
    } binary_operation;
    struct
    {
      char *var_name;
      struct AST_NODE *value;
    } assignment;
    char *variable;
  };
} AST_NODE;

AST_NODE *ParseProgram(char const *source);
void FreeAST(AST_NODE *node);
