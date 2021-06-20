#include <assert.h>
#include <stdio.h>

#include "ast.h"
#include "value.h"

VALUE ValueNumber(double number)
{
  return (VALUE){.kind = VALUE_NUMBER, .number = number};
}

VALUE ValueLambda(AST_NODE *body)
{
  LAMBDA lambda = (LAMBDA){.body = CopyAST(body)};
  return (VALUE){.kind = VALUE_LAMBDA, .lambda = lambda};
}

void FreeValue(VALUE *value)
{
  switch (value->kind)
  {
    case VALUE_NUMBER:
      break;
    case VALUE_LAMBDA:
      FreeAST(value->lambda.body);
      break;
  }
}

VALUE
ValueAdd(VALUE left, VALUE right)
{
  assert(left.kind == VALUE_NUMBER && right.kind == VALUE_NUMBER &&
         "ValueAdd: only numbers can be added");
  return ValueNumber(left.number + right.number);
}

VALUE ValueSub(VALUE left, VALUE right)
{
  assert(left.kind == VALUE_NUMBER && right.kind == VALUE_NUMBER &&
         "ValueSub: only numbers can be subtracted");
  return ValueNumber(left.number - right.number);
}

VALUE ValueMul(VALUE left, VALUE right)
{
  assert(left.kind == VALUE_NUMBER && right.kind == VALUE_NUMBER &&
         "ValueMul: only numbers can be multiplied");
  return ValueNumber(left.number * right.number);
}

VALUE ValueDiv(VALUE left, VALUE right)
{
  assert(left.kind == VALUE_NUMBER && right.kind == VALUE_NUMBER &&
         "ValueDiv: only numbers can be divided");
  return ValueNumber(left.number / right.number);
}

void PrintValue(VALUE *value)
{
  switch (value->kind)
  {
    case VALUE_NUMBER:
      printf("%f", value->number);
      break;
    case VALUE_LAMBDA:
      printf("<lambda>");
      break;
  }
}
