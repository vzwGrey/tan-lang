#include <stdlib.h>
#include <string.h>

#include "ast.h"

AST_NODE *CopyAST(AST_NODE *node)
{
  AST_NODE *copy = malloc(sizeof(*copy));
  memcpy(copy, node, sizeof(*copy));

  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      break;
    case NODE_BINARY_OPERATION:
      copy->binary_operation.left = CopyAST(node->binary_operation.left);
      copy->binary_operation.right = CopyAST(node->binary_operation.right);
      break;
    case NODE_ASSIGNMENT:
      copy->assignment.var_name = strdup(node->assignment.var_name);
      copy->assignment.value = CopyAST(node->assignment.value);
      break;
    case NODE_VARIABLE:
      copy->variable = strdup(node->variable);
      break;
    case NODE_LAMBDA:
      copy->lambda.params = CopyFnParams(node->lambda.params);
      copy->lambda.body = CopyAST(node->lambda.body);
      break;
    case NODE_CALL:
      copy->call.args = CopyFnArgs(node->call.args);
      copy->call.fn = CopyAST(node->call.fn);
      break;
    case NODE_IF_ELSE:
      copy->if_else.condition = CopyAST(node->if_else.condition);
      copy->if_else.if_true = CopyAST(node->if_else.if_true);
      copy->if_else.if_false = CopyAST(node->if_else.if_false);
      break;
  }

  return copy;
}

void FreeAST(AST_NODE *node)
{
  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      break;
    case NODE_BINARY_OPERATION:
      FreeAST(node->binary_operation.left);
      FreeAST(node->binary_operation.right);
      break;
    case NODE_ASSIGNMENT:
      free(node->assignment.var_name);
      free(node->assignment.value);
      break;
    case NODE_VARIABLE:
      free(node->variable);
      break;
    case NODE_LAMBDA: {
      FN_PARAM *param = node->lambda.params;
      while (param != NULL)
      {
        free(param->name);

        FN_PARAM *tmp = param;
        param = param->next;
        free(tmp);
      }
      if (node->lambda.body != NULL)
        FreeAST(node->lambda.body);
      break;
    }
    case NODE_CALL: {
      FN_ARG *arg = node->call.args;
      while (arg != NULL)
      {
        FreeAST(arg->value);

        FN_ARG *tmp = arg;
        arg = arg->next;
        free(tmp);
      }
      FreeAST(node->call.fn);
      break;
    }
    case NODE_IF_ELSE:
      FreeAST(node->if_else.condition);
      FreeAST(node->if_else.if_true);
      FreeAST(node->if_else.if_false);
      break;
  }

  free(node);
}

FN_PARAM *CopyFnParams(FN_PARAM *params)
{
  FN_PARAM *copy = NULL;
  for (FN_PARAM *param = params; param != NULL; param = param->next)
  {
    if (copy == NULL)
    {
      copy = malloc(sizeof(*copy));
      copy->name = strdup(param->name);
      copy->next = NULL;
    }
    else
    {
      FN_PARAM *next_copy = malloc(sizeof(*next_copy));
      next_copy->name = strdup(param->name);
      next_copy->next = NULL;
      AppendFnParam(copy, next_copy);
    }
  }
  return copy;
}

void AppendFnParam(FN_PARAM *params, FN_PARAM *new_param)
{
  FN_PARAM *last = params;
  while (last->next != NULL)
    last = last->next;
  last->next = new_param;
}

FN_ARG *CopyFnArgs(FN_ARG *args)
{
  FN_ARG *copy = NULL;
  for (FN_ARG *arg = args; arg != NULL; arg = arg->next)
  {
    if (copy == NULL)
    {
      copy = malloc(sizeof(*copy));
      copy->value = CopyAST(arg->value);
      copy->next = NULL;
    }
    else
    {
      FN_ARG *next_copy = malloc(sizeof(*next_copy));
      next_copy->value = CopyAST(arg->value);
      next_copy->next = NULL;
      AppendFnArg(copy, next_copy);
    }
  }
  return copy;
}

void AppendFnArg(FN_ARG *args, FN_ARG *new_arg)
{
  FN_ARG *last = args;
  while (last->next != NULL)
    last = last->next;
  last->next = new_arg;
}
