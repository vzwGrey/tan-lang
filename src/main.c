#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <history.h>
#include <readline.h>

// ---------------
// Lexer
// ---------------

typedef enum
{
  TOKEN_EOF = 0,
  TOKEN_ERROR,
  TOKEN_NUMBER,
  TOKEN_PLUS,
} TOKEN_KIND;

typedef struct
{
  TOKEN_KIND kind;
  char const *start;
  size_t len;
} TOKEN;

char const *TokenKindName(TOKEN_KIND kind)
{
  switch (kind)
  {
    case TOKEN_EOF:
      return "end of file";
    case TOKEN_ERROR:
      return "<error>";
    case TOKEN_NUMBER:
      return "number";
    case TOKEN_PLUS:
      return "'+'";
    default:
      assert(!"TokenKindName: unreachable");
      return NULL;
  }
}

#define CURRENT_CHAR(source) (**source)
#define ADVANCE(source) ((*source)++)
void NumberToken(char const **source, TOKEN *token)
{
  char const *start = *source;
  while (isdigit(CURRENT_CHAR(source)))
    ADVANCE(source);

  if (CURRENT_CHAR(source) == '.')
  {
    ADVANCE(source);
    while (isdigit(CURRENT_CHAR(source)))
      ADVANCE(source);
  }

  token->kind = TOKEN_NUMBER;
  token->start = start;
  token->len = *source - start;
}

void NextToken(char const **source, TOKEN *token)
{
  if (CURRENT_CHAR(source) == 0)
  {
    token->kind = TOKEN_EOF;
    token->start = *source;
    token->len = 0;
    return;
  }

  while (isspace(CURRENT_CHAR(source)))
    ADVANCE(source);

  if (isdigit(CURRENT_CHAR(source)))
  {
    NumberToken(source, token);
    return;
  }

  switch (CURRENT_CHAR(source))
  {
    case '+':
      token->kind = TOKEN_PLUS;
      token->start = *source;
      token->len = 1;
      ADVANCE(source);
      return;
  }

  fprintf(stderr, "Encountered an unknown character '%c'.\n", CURRENT_CHAR(source));
  token->kind = TOKEN_ERROR;
  token->start = *source;
  token->len = 1;
  ADVANCE(source);
}
#undef ADVANCE
#undef CURRENT_CHAR

// ---------------
// Parser
// ---------------

typedef enum
{
  BINOP_ADDITION = TOKEN_PLUS,
} BINARY_OPERATION_KIND;

typedef enum
{
  NODE_CONSTANT_NUMBER,
  NODE_BINARY_OPERATION,
} AST_NODE_KIND;

typedef struct AST_NODE
{
  AST_NODE_KIND kind;
  union
  {
    double constant_number;
    struct
    {
      struct AST_NODE *left;
      struct AST_NODE *right;
      BINARY_OPERATION_KIND op;
    } binary_operation;
  };
} AST_NODE;

typedef struct
{
  char const *source;
  TOKEN peeked_token;
  bool has_peeked_token;
} PARSER_STATE;

void GetToken(PARSER_STATE *state, TOKEN *token)
{
  if (state->has_peeked_token)
    memcpy(token, &state->peeked_token, sizeof(*token));
  else
    NextToken(&state->source, token);
}

TOKEN ExpectToken(PARSER_STATE *state, TOKEN_KIND expected_kind)
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

bool PeekToken(PARSER_STATE *state, TOKEN_KIND expected_kind)
{
  TOKEN token;
  GetToken(state, &token);
  if (token.kind == expected_kind)
  {
    state->has_peeked_token = false;
    return true;
  }

  state->peeked_token = token;
  state->has_peeked_token = true;
  return false;
}

void ParseEOF(PARSER_STATE *state)
{
  ExpectToken(state, TOKEN_EOF);
}

AST_NODE *ParseConstantNumber(PARSER_STATE *state)
{
  TOKEN token = ExpectToken(state, TOKEN_NUMBER);

  AST_NODE *node = malloc(sizeof(*node));
  node->kind = NODE_CONSTANT_NUMBER;
  node->constant_number = strtod(token.start, NULL);

  return node;
}

AST_NODE *ParseBinaryOperation(PARSER_STATE *state)
{
  AST_NODE *left = ParseConstantNumber(state);

  if (PeekToken(state, TOKEN_PLUS))
  {
    AST_NODE *node = malloc(sizeof(*node));
    node->kind = NODE_BINARY_OPERATION;
    node->binary_operation.left = left;
    node->binary_operation.right = ParseBinaryOperation(state);
    node->binary_operation.op = TOKEN_PLUS;

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

  AST_NODE *node = ParseBinaryOperation(&state);
  ParseEOF(&state);

  return node;
}

void FreeASTNode(AST_NODE *node)
{
  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      break;
    case NODE_BINARY_OPERATION:
      FreeASTNode(node->binary_operation.left);
      FreeASTNode(node->binary_operation.right);
      break;
  }

  free(node);
}

// ---------------
// Interpreter
// ---------------

double Evaluate(AST_NODE *node);

double EvaluateConstantNumber(AST_NODE *node)
{
  assert(node->kind == NODE_CONSTANT_NUMBER);
  return node->constant_number;
}

double EvaluateBinaryOperation(AST_NODE *node)
{
  assert(node->kind == NODE_BINARY_OPERATION);
  double left = Evaluate(node->binary_operation.left);
  double right = Evaluate(node->binary_operation.right);
  switch (node->binary_operation.op)
  {
    case BINOP_ADDITION:
      return left + right;
      break;
  }

  assert(!"EvaluateBinaryOperation: unreachable");
  return 0.0;
}

double Evaluate(AST_NODE *node)
{
  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      return EvaluateConstantNumber(node);
    case NODE_BINARY_OPERATION:
      return EvaluateBinaryOperation(node);
    default:
      assert(!"EvaluateProgram: unreachable");
      return 0.0;
  }
}

// ---------------
// REPL
// ---------------

bool ReadInput(char const *prompt, char **line)
{
  *line = readline(prompt);

  if (!*line)
    return false;

  if (**line)
    add_history(*line);

  return true;
}

int main(void)
{
  char *line;
  while (ReadInput(">> ", &line))
  {
    if (!*line)
      continue;

    char const *source = line;
    AST_NODE *ast = ParseProgram(source);

    double result = Evaluate(ast);
    printf("\t%f\n", result);

    FreeASTNode(ast);

    free(line);
  }

  return 0;
}
