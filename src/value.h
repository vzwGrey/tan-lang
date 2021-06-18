#pragma once

#include "parser.h"

typedef struct
{
  AST_NODE *body;
} LAMBDA;

typedef enum
{
  VALUE_NUMBER,
  VALUE_LAMBDA,
} VALUE_KIND;

typedef struct
{
  VALUE_KIND kind;
  union
  {
    double number;
    LAMBDA lambda;
  };
} VALUE;

VALUE ValueNumber(double number);
VALUE ValueLambda(AST_NODE *body);
void FreeValue(VALUE *value);
VALUE ValueAdd(VALUE left, VALUE right);
VALUE ValueSub(VALUE left, VALUE right);
VALUE ValueMul(VALUE left, VALUE right);
VALUE ValueDiv(VALUE left, VALUE right);
void PrintValue(VALUE *value);
