#pragma once

#include "parser.h"

typedef struct
{
  char *name;
  double value;
} VARIABLE;

typedef struct
{
  VARIABLE *variables;
  size_t num_variables;
} INTERPRETER_STATE;

INTERPRETER_STATE NewInterpreterState(size_t num_variables);
void FreeInterpreterState(INTERPRETER_STATE *state);
double Evaluate(INTERPRETER_STATE *state, AST_NODE *node);
