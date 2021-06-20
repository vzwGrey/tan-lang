#pragma once

#include "parser.h"
#include "value.h"

typedef struct
{
  char *name;
  VALUE value;
} VARIABLE;

typedef struct SCOPE
{
  struct SCOPE *upper_scope;
  VARIABLE *variables;
} SCOPE;

typedef struct
{
  SCOPE *current_scope;
  size_t variables_per_scope;
} INTERPRETER_STATE;

INTERPRETER_STATE NewInterpreterState(size_t variables_per_scope);
void FreeInterpreterState(INTERPRETER_STATE *state);
VALUE Evaluate(INTERPRETER_STATE *state, AST_NODE *node);
