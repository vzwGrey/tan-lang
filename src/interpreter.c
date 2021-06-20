#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "interpreter.h"
#include "unreachable.h"

static void PushNewScope(INTERPRETER_STATE *state);
static void PopScope(INTERPRETER_STATE *state);

INTERPRETER_STATE NewInterpreterState(size_t variables_per_scope)
{
  INTERPRETER_STATE state = (INTERPRETER_STATE){
      .current_scope = NULL,
      .variables_per_scope = variables_per_scope,
  };
  PushNewScope(&state);
  return state;
}

void FreeInterpreterState(INTERPRETER_STATE *state)
{
  while (state->current_scope != NULL)
    PopScope(state);
}

static void PushNewScope(INTERPRETER_STATE *state)
{
  SCOPE *new_scope = malloc(sizeof(*new_scope));
  new_scope->upper_scope = state->current_scope;
  new_scope->variables = malloc(sizeof(VARIABLE) * state->variables_per_scope);
  state->current_scope = new_scope;
}

static void PopScope(INTERPRETER_STATE *state)
{
  SCOPE *current_scope = state->current_scope;
  SCOPE *upper_scope = current_scope->upper_scope;

  for (size_t i = 0; i < state->variables_per_scope; i++)
  {
    VARIABLE *var = &current_scope->variables[i];
    if (var->name != NULL)
    {
      free(var->name);
      FreeValue(&var->value);
    }
  }
  free(current_scope->variables);
  free(current_scope);

  state->current_scope = upper_scope;
}

static void SetVariable(INTERPRETER_STATE *state, char const *name, VALUE value)
{
  for (size_t i = 0; i < state->variables_per_scope; i++)
  {
    VARIABLE *var = &state->current_scope->variables[i];
    if (var->name != NULL && strcmp(var->name, name) == 0)
    {
      var->value = value;
      return;
    }
  }

  for (size_t i = 0; i < state->variables_per_scope; i++)
  {
    VARIABLE *var = &state->current_scope->variables[i];
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
  for (SCOPE *scope = state->current_scope; scope != NULL; scope = scope->upper_scope)
    for (size_t i = 0; i < state->variables_per_scope; i++)
    {
      VARIABLE *var = &scope->variables[i];
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

  unreachable();
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
  (void)state;
  assert(node->kind == NODE_LAMBDA);
  return ValueLambda(node->lambda.params, node->lambda.body);
}

static VALUE EvaluateCall(INTERPRETER_STATE *state, AST_NODE *node)
{
  assert(node->kind == NODE_CALL);
  VALUE fn = Evaluate(state, node->call.fn);
  assert(fn.kind == VALUE_LAMBDA && "EvaluateCall: only functions can be called");

  PushNewScope(state);

  FN_PARAM *current_param = fn.lambda.params;
  FN_ARG *current_arg = node->call.args;

  while (true)
  {
    if (current_param == NULL && current_arg == NULL)
      break;

    if ((current_param == NULL && current_arg != NULL) ||
        (current_param != NULL && current_arg == NULL))
    {
      assert(!"EvaluateCall: number of arguments does not match number of function parameters");
    }

    SetVariable(state, current_param->name, Evaluate(state, current_arg->value));

    current_param = current_param->next;
    current_arg = current_arg->next;
  }

  VALUE fn_ret = Evaluate(state, fn.lambda.body);
  PopScope(state);
  return fn_ret;
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
