#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

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

static AST_NODE *ParseFactor(PARSER_STATE *state)
{
  AST_NODE *left = ParseConstantNumber(state);

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

static AST_NODE *ParseSequence(PARSER_STATE *state)
{
  AST_NODE *left = ParseSum(state);

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

void FreeAST(AST_NODE *node)
{
  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      break;
    case NODE_BINARY_OPERATION:
      FreeAST(node->binary_operation.left);
      FreeAST(node->binary_operation.right);
      break;
  }

  free(node);
}
