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
  TOKEN_NUMBER,
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
    case TOKEN_NUMBER:
      return "number";
    case TOKEN_EOF:
      return "end of file";
    default:
      assert(!"TokenKindName: unreachable");
      return NULL;
  }
}

#define CURRENT_CHAR(source) (**source)
void NumberToken(char const **source, TOKEN *token)
{
  char const *start = *source;
  while (isdigit(CURRENT_CHAR(source)))
    (*source)++;

  token->kind = TOKEN_NUMBER;
  token->start = start;
  token->len = *source - start;
}

bool NextToken(char const **source, TOKEN *token)
{
  if (CURRENT_CHAR(source) == 0)
  {
    token->kind = TOKEN_EOF;
    token->start = *source;
    token->len = 0;
    return true;
  }

  while (isspace(CURRENT_CHAR(source)))
    (*source)++;

  if (isdigit(CURRENT_CHAR(source)))
  {
    NumberToken(source, token);
    return true;
  }

  return false;
}
#undef CURRENT_CHAR

// ---------------
// Parser
// ---------------

typedef enum
{
  NODE_CONSTANT_NUMBER,
} AST_NODE_KIND;

typedef struct AST_NODE
{
  AST_NODE_KIND kind;
  union
  {
    double constant_number;
  };
} AST_NODE;

void ParseEOF(char const **source)
{
  TOKEN token;
  // TODO: Make/Use `ExpectToken` function.
  NextToken(source, &token);
  assert(token.kind == TOKEN_EOF && "Expected EOF token");
}

AST_NODE *ParseConstantNumber(char const **source)
{
  TOKEN token;
  // TODO: Make/Use `ExpectToken` function.
  NextToken(source, &token);
  assert(token.kind == TOKEN_NUMBER && "Expected number token");

  AST_NODE *node = malloc(sizeof(*node));
  node->kind = NODE_CONSTANT_NUMBER;
  node->constant_number = strtod(token.start, NULL);

  return node;
}

AST_NODE *ParseProgram(char const **source)
{
  AST_NODE *node = ParseConstantNumber(source);
  ParseEOF(source);
  return node;
}

void FreeAST(AST_NODE *node)
{
  free(node);
}

// ---------------
// Interpreter
// ---------------

double EvaluateConstantNumber(AST_NODE *node)
{
  assert(node->kind == NODE_CONSTANT_NUMBER);
  return node->constant_number;
}

double EvaluateProgram(AST_NODE *node)
{
  switch (node->kind)
  {
    case NODE_CONSTANT_NUMBER:
      return EvaluateConstantNumber(node);
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
    AST_NODE *ast = ParseProgram(&source);

    double result = EvaluateProgram(ast);
    printf("\t%f\n", result);

    FreeAST(ast);

    free(line);
  }

  return 0;
}
