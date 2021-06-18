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
      copy->lambda.body = CopyAST(node->lambda.body);
      break;
    case NODE_CALL:
      copy->call.fn = CopyAST(node->call.fn);
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
    case NODE_LAMBDA:
      if (node->lambda.body != NULL)
        FreeAST(node->lambda.body);
      break;
    case NODE_CALL:
      FreeAST(node->call.fn);
      break;
  }

  free(node);
}
