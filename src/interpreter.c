#include <assert.h>

#include "interpreter.h"

double Evaluate(AST_NODE *node);

static double EvaluateConstantNumber(AST_NODE *node)
{
  assert(node->kind == NODE_CONSTANT_NUMBER);
  return node->constant_number;
}

static double EvaluateBinaryOperation(AST_NODE *node)
{
  assert(node->kind == NODE_BINARY_OPERATION);
  double left = Evaluate(node->binary_operation.left);
  double right = Evaluate(node->binary_operation.right);
  switch (node->binary_operation.op)
  {
    case BINOP_ADD:
      return left + right;
    case BINOP_SUB:
      return left - right;
    case BINOP_MUL:
      return left * right;
    case BINOP_DIV:
      return left / right;
    case BINOP_SEQ:
      return right;
  }

  assert(!"EvaluateBinaryOperation: unreachable");
  return 0.0;
}

double Evaluate(AST_NODE *node)
{
  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      return EvaluateConstantNumber(node);
    case NODE_BINARY_OPERATION:
      return EvaluateBinaryOperation(node);
    default:
      assert(!"EvaluateProgram: unreachable");
      return 0.0;
  }
}
