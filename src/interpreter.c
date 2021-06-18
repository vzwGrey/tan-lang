#include <assert.h>
#include <string.h>

#include "interpreter.h"
#include "unreachable.h"

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
    VARIABLE *var = &state->variables[i];
    if (var->name != NULL)
    {
      free(var->name);
      FreeValue(&var->value);
    }
  }
  free(state->variables);
}

static void SetVariable(INTERPRETER_STATE *state, char const *name, VALUE value)
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

static VALUE GetVariable(INTERPRETER_STATE *state, char const *name)
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
  unreachable();
}

static VALUE EvaluateConstantNumber(INTERPRETER_STATE *state, AST_NODE *node)
{
  (void)state;

  assert(node->kind == NODE_CONSTANT_NUMBER);
  return ValueNumber(node->constant_number);
}

static VALUE EvaluateBinaryOperation(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_BINARY_OPERATION);

  VALUE left = Evaluate(state, node->binary_operation.left);
  VALUE right = Evaluate(state, node->binary_operation.right);

  switch (node->binary_operation.op)
  {
    case BINOP_ADD:
      return ValueAdd(left, right);
    case BINOP_SUB:
      return ValueSub(left, right);
    case BINOP_MUL:
      return ValueMul(left, right);
    case BINOP_DIV:
      return ValueDiv(left, right);
    case BINOP_SEQ:
      return right;
  }
}

static VALUE EvaluateAssignment(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_ASSIGNMENT);
  VALUE value = Evaluate(state, node->assignment.value);
  SetVariable(state, node->assignment.var_name, value);
  return value;
}

static VALUE EvaluateVariable(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_VARIABLE);
  return GetVariable(state, node->variable);
}

static VALUE EvaluateLambda(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_LAMBDA);
  return ValueLambda(node->lambda.body);
}

static VALUE EvaluateCall(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_CALL);
  VALUE fn = Evaluate(state, node->call.fn);
  assert(fn.kind == VALUE_LAMBDA && "EvaluateCall: only functions can be called");
  return Evaluate(state, fn.lambda.body);
}

VALUE Evaluate(INTERPRETER_STATE *state, AST_NODE *node)
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
    case NODE_LAMBDA:
      return EvaluateLambda(state, node);
    case NODE_CALL:
      return EvaluateCall(state, node);
  }

  assert(!"EvaluateProgram: unreachable");
  unreachable();
}
