#include <assert.h>
#include <string.h>

#include "interpreter.h"

INTERPRETER_STATE NewInterpreterState(size_t num_variables)
{
  return (INTERPRETER_STATE){
      .variables = malloc(sizeof(VARIABLE) * num_variables),
      .num_variables = num_variables,
  };
}
void FreeInterpreterState(INTERPRETER_STATE *state)
{
  for (size_t i = 0; i < state->num_variables; i++)
  {
    char *name = state->variables[i].name;
    if (name != NULL)
      free(name);
  }
  free(state->variables);
}

static void SetVariable(INTERPRETER_STATE *state, char const *name, double value)
{
  for (size_t i = 0; i < state->num_variables; i++)
  {
    VARIABLE *var = &state->variables[i];
    if (var->name != NULL && strcmp(var->name, name) == 0)
    {
      var->value = value;
      return;
    }
  }

  for (size_t i = 0; i < state->num_variables; i++)
  {
    VARIABLE *var = &state->variables[i];
    if (var->name == NULL)
    {
      var->name = strdup(name);
      var->value = value;
      return;
    }
  }

  assert(!"SetVariable: out of free variables.");
}

static double GetVariable(INTERPRETER_STATE *state, char const *name)
{
  for (size_t i = 0; i < state->num_variables; i++)
  {
    VARIABLE *var = &state->variables[i];
    if (var->name != NULL && strcmp(var->name, name) == 0)
    {
      return var->value;
    }
  }

  assert(!"GetVariable: unknown variable.");
  return 0.0;
}

static double EvaluateConstantNumber(INTERPRETER_STATE *state, AST_NODE *node)
{
  (void)state;

  assert(node->kind == NODE_CONSTANT_NUMBER);
  return node->constant_number;
}

static double EvaluateBinaryOperation(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_BINARY_OPERATION);
  double left = Evaluate(state, node->binary_operation.left);
  double right = Evaluate(state, node->binary_operation.right);
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

static double EvaluateAssignment(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_ASSIGNMENT);
  double value = Evaluate(state, node->assignment.value);
  SetVariable(state, node->assignment.var_name, value);
  return value;
}

static double EvaluateVariable(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_VARIABLE);
  return GetVariable(state, node->variable);
}

double Evaluate(INTERPRETER_STATE *state, AST_NODE *node)
{
  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      return EvaluateConstantNumber(state, node);
    case NODE_BINARY_OPERATION:
      return EvaluateBinaryOperation(state, node);
    case NODE_ASSIGNMENT:
      return EvaluateAssignment(state, node);
    case NODE_VARIABLE:
      return EvaluateVariable(state, node);
  }

  assert(!"EvaluateProgram: unreachable");
  return 0.0;
}
