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

static AST_NODE *ParseLambda(PARSER_STATE *state)
{
  ExpectToken(state, TOKEN_FN);
  ExpectToken(state, TOKEN_OPAREN);
  ExpectToken(state, TOKEN_CPAREN);
  ExpectToken(state, TOKEN_OBRACE);
  AST_NODE *lambda = malloc(sizeof(*lambda));
  lambda->kind = NODE_LAMBDA;
  lambda->lambda.body = ParseSequence(state);
  ExpectToken(state, TOKEN_CBRACE);

  return lambda;
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
  if (oparen.kind == TOKEN_OPAREN)
  {
    ConsumePeekedToken(state);

    AST_NODE *call = malloc(sizeof(*call));
    call->kind = NODE_CALL;
    call->call.fn = term;

    ExpectToken(state, TOKEN_CPAREN);

    return call;
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

static AST_NODE *ParseSequence(PARSER_STATE *state)
{
  AST_NODE *left = ParseAssignment(state);

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
