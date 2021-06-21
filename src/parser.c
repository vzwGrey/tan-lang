#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"

typedef struct
{
  char const *source;
  TOKEN peeked_token;
  bool has_peeked_token;
} PARSER_STATE;

static void GetToken(PARSER_STATE *state, TOKEN *token)
{
  if (state->has_peeked_token)
    *token = state->peeked_token;
  else
    NextToken(&state->source, token);
}

static void ConsumePeekedToken(PARSER_STATE *state)
{
  assert(state->has_peeked_token && "ConsumePeekedToken: no peeked token available");
  state->has_peeked_token = false;
}

static TOKEN ExpectToken(PARSER_STATE *state, TOKEN_KIND expected_kind)
{
  TOKEN token;
  GetToken(state, &token);
  state->has_peeked_token = false;

  if (token.kind != expected_kind)
  {
    fprintf(stderr, "Expected token %s but found token %s.\n", TokenKindName(expected_kind),
            TokenKindName(token.kind));
    // If the expected token was not found make one up to try to resume parsing.
    return (TOKEN){.kind = expected_kind, .start = state->source, .len = 0};
  }

  return token;
}

static TOKEN PeekToken(PARSER_STATE *state)
{
  TOKEN token;
  GetToken(state, &token);
  state->peeked_token = token;
  state->has_peeked_token = true;
  return token;
}

static AST_NODE *ParseSequence(PARSER_STATE *state);
static AST_NODE *ParseAssignment(PARSER_STATE *state);

static void ParseEOF(PARSER_STATE *state)
{
  ExpectToken(state, TOKEN_EOF);
}

static AST_NODE *ParseConstantNumber(PARSER_STATE *state)
{
  TOKEN token = ExpectToken(state, TOKEN_NUMBER);

  AST_NODE *node = malloc(sizeof(*node));
  node->kind = NODE_CONSTANT_NUMBER;
  node->constant_number = strtod(token.start, NULL);

  return node;
}

static AST_NODE *ParseVariable(PARSER_STATE *state)
{
  TOKEN token = ExpectToken(state, TOKEN_IDENT);

  AST_NODE *node = malloc(sizeof(*node));
  node->kind = NODE_VARIABLE;
  node->variable = strndup(token.start, token.len);

  return node;
}

static FN_PARAM *ParseParam(PARSER_STATE *state)
{
  TOKEN name = ExpectToken(state, TOKEN_IDENT);
  FN_PARAM *param = malloc(sizeof(*param));
  param->name = strndup(name.start, name.len);
  param->next = NULL;
  return param;
}

static FN_PARAM *ParseParams(PARSER_STATE *state)
{
  if (PeekToken(state).kind != TOKEN_IDENT)
    return NULL;

  FN_PARAM *param = ParseParam(state);

  while (PeekToken(state).kind == TOKEN_COMMA)
  {
    ConsumePeekedToken(state);
    FN_PARAM *next_param = ParseParam(state);
    AppendFnParam(param, next_param);
  }

  return param;
}

static AST_NODE *ParseLambda(PARSER_STATE *state)
{
  ExpectToken(state, TOKEN_FN);
  ExpectToken(state, TOKEN_OPAREN);
  FN_PARAM *params = ParseParams(state);
  ExpectToken(state, TOKEN_CPAREN);
  ExpectToken(state, TOKEN_OBRACE);
  AST_NODE *lambda = malloc(sizeof(*lambda));
  lambda->kind = NODE_LAMBDA;
  lambda->lambda.params = params;
  lambda->lambda.body = ParseSequence(state);
  ExpectToken(state, TOKEN_CBRACE);

  return lambda;
}

static FN_ARG *ParseArg(PARSER_STATE *state)
{
  FN_ARG *arg = malloc(sizeof(*arg));
  arg->value = ParseAssignment(state);
  arg->next = NULL;
  return arg;
}

static FN_ARG *ParseArgs(PARSER_STATE *state)
{
  if (PeekToken(state).kind == TOKEN_CPAREN)
    return NULL;

  FN_ARG *arg = ParseArg(state);

  while (PeekToken(state).kind == TOKEN_COMMA)
  {
    ConsumePeekedToken(state);
    FN_ARG *next_arg = ParseArg(state);
    AppendFnArg(arg, next_arg);
  }

  return arg;
}

static AST_NODE *ParseTerm(PARSER_STATE *state)
{
  AST_NODE *term;

  TOKEN next_token = PeekToken(state);
  switch (next_token.kind)
  {
    case TOKEN_IDENT:
      term = ParseVariable(state);
      break;
    case TOKEN_FN:
      term = ParseLambda(state);
      break;
    default:
      term = ParseConstantNumber(state);
      break;
  }

  TOKEN oparen = PeekToken(state);
  while (oparen.kind == TOKEN_OPAREN)
  {
    ConsumePeekedToken(state);

    AST_NODE *call = malloc(sizeof(*call));
    call->kind = NODE_CALL;
    call->call.args = ParseArgs(state);
    call->call.fn = term;

    ExpectToken(state, TOKEN_CPAREN);

    term = call;

    oparen = PeekToken(state);
  }

  return term;
}

static AST_NODE *ParseFactor(PARSER_STATE *state)
{
  AST_NODE *left = ParseTerm(state);

  TOKEN next_token = PeekToken(state);
  if (next_token.kind == TOKEN_STAR || next_token.kind == TOKEN_SLASH)
  {
    ConsumePeekedToken(state);

    AST_NODE *node = malloc(sizeof(*node));
    node->kind = NODE_BINARY_OPERATION;
    node->binary_operation.left = left;
    node->binary_operation.right = ParseFactor(state);
    node->binary_operation.op = next_token.kind;

    left = node;
  }

  return left;
}

static AST_NODE *ParseSum(PARSER_STATE *state)
{
  AST_NODE *left = ParseFactor(state);

  TOKEN next_token = PeekToken(state);
  if (next_token.kind == TOKEN_PLUS || next_token.kind == TOKEN_MINUS)
  {
    ConsumePeekedToken(state);

    AST_NODE *node = malloc(sizeof(*node));
    node->kind = NODE_BINARY_OPERATION;
    node->binary_operation.left = left;
    node->binary_operation.right = ParseSum(state);
    node->binary_operation.op = next_token.kind;

    left = node;
  }

  return left;
}

static AST_NODE *ParseAssignment(PARSER_STATE *state)
{
  PARSER_STATE saved_state = *state;

  TOKEN token = PeekToken(state);
  if (token.kind == TOKEN_IDENT)
  {
    ConsumePeekedToken(state);

    TOKEN equal = PeekToken(state);
    if (equal.kind != TOKEN_EQUAL)
    {
      *state = saved_state;
      return ParseSum(state);
    }
    ConsumePeekedToken(state);

    AST_NODE *assignment = malloc(sizeof(*assignment));
    assignment->kind = NODE_ASSIGNMENT;
    assignment->assignment.var_name = strndup(token.start, token.len);
    assignment->assignment.value = ParseSum(state);

    return assignment;
  }
  else
    return ParseSum(state);
}

static AST_NODE *ParseIfElse(PARSER_STATE *state)
{
  TOKEN if_tok = PeekToken(state);
  if (if_tok.kind == TOKEN_IF)
  {
    ConsumePeekedToken(state);

    ExpectToken(state, TOKEN_OPAREN);
    AST_NODE *condition = ParseAssignment(state);
    ExpectToken(state, TOKEN_CPAREN);

    ExpectToken(state, TOKEN_OBRACE);
    AST_NODE *if_true = ParseIfElse(state);
    ExpectToken(state, TOKEN_CBRACE);

    ExpectToken(state, TOKEN_ELSE);
    ExpectToken(state, TOKEN_OBRACE);
    AST_NODE *if_false = ParseIfElse(state);
    ExpectToken(state, TOKEN_CBRACE);

    AST_NODE *if_else = malloc(sizeof(*if_else));
    if_else->kind = NODE_IF_ELSE;
    if_else->if_else.condition = condition;
    if_else->if_else.if_true = if_true;
    if_else->if_else.if_false = if_false;

    return if_else;
  }
  else
    return ParseAssignment(state);
}

static AST_NODE *ParseSequence(PARSER_STATE *state)
{
  AST_NODE *left = ParseIfElse(state);

  TOKEN next_token = PeekToken(state);
  if (next_token.kind == TOKEN_COMMA)
  {
    ConsumePeekedToken(state);

    AST_NODE *node = malloc(sizeof(*node));
    node->kind = NODE_BINARY_OPERATION;
    node->binary_operation.left = left;
    node->binary_operation.right = ParseSequence(state);
    node->binary_operation.op = BINOP_SEQ;

    left = node;
  }

  return left;
}

AST_NODE *ParseProgram(char const *source)
{
  PARSER_STATE state = {
      .source = source,
      .has_peeked_token = false,
  };

  AST_NODE *node = ParseSequence(&state);
  ParseEOF(&state);

  return node;
}
