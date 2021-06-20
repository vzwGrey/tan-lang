#pragma once

#include "lexer.h"

struct AST_NODE;

typedef enum
{
  BINOP_ADD = TOKEN_PLUS,
  BINOP_SUB = TOKEN_MINUS,
  BINOP_MUL = TOKEN_STAR,
  BINOP_DIV = TOKEN_SLASH,
  BINOP_SEQ = TOKEN_COMMA,
} BINARY_OPERATION_KIND;

typedef struct FN_PARAM
{
  struct FN_PARAM *next;
  char *name;
} FN_PARAM;

typedef struct FN_ARG
{
  struct FN_ARG *next;
  struct AST_NODE *value;
} FN_ARG;

typedef enum
{
  NODE_CONSTANT_NUMBER,
  NODE_BINARY_OPERATION,
  NODE_ASSIGNMENT,
  NODE_VARIABLE,
  NODE_LAMBDA,
  NODE_CALL,
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
    struct
    {
      FN_PARAM *params;
      struct AST_NODE *body;
    } lambda;
    struct
    {
      FN_ARG *args;
      struct AST_NODE *fn;
    } call;
  };
} AST_NODE;

AST_NODE *CopyAST(AST_NODE *node);
void FreeAST(AST_NODE *node);

FN_PARAM *CopyFnParams(FN_PARAM *param);
void AppendFnParam(FN_PARAM *params, FN_PARAM *new_param);
FN_ARG *CopyFnArgs(FN_ARG *arg);
void AppendFnArg(FN_ARG *args, FN_ARG *new_arg);
