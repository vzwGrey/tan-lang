#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "lexer.h"

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
    case TOKEN_MINUS:
      return "'-'";
    case TOKEN_STAR:
      return "'*'";
    case TOKEN_SLASH:
      return "'/'";
    case TOKEN_COMMA:
      return "','";
    case TOKEN_EQUAL:
      return "'='";
    case TOKEN_IDENT:
      return "identifier";
  }

  assert(!"TokenKindName: unreachable");
  return NULL;
}

#define CURRENT_CHAR(source) (**source)
#define ADVANCE(source) ((*source)++)
static void NumberToken(char const **source, TOKEN *token)
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

static void IdentToken(char const **source, TOKEN *token)
{
  char const *start = *source;

  while (CURRENT_CHAR(source) == '_' || isalnum(CURRENT_CHAR(source)))
    ADVANCE(source);

  token->kind = TOKEN_IDENT;
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

  if (CURRENT_CHAR(source) == '_' || isalpha(CURRENT_CHAR(source)))
  {
    IdentToken(source, token);
    return;
  }

#define SINGLE_CHAR_TOK(k)                                                                         \
  do                                                                                               \
  {                                                                                                \
    token->kind = (k);                                                                             \
    token->start = *source;                                                                        \
    token->len = 1;                                                                                \
    ADVANCE(source);                                                                               \
  }                                                                                                \
  while (false)

  switch (CURRENT_CHAR(source))
  {
    case '+':
      SINGLE_CHAR_TOK(TOKEN_PLUS);
      return;
    case '-':
      SINGLE_CHAR_TOK(TOKEN_MINUS);
      return;
    case '*':
      SINGLE_CHAR_TOK(TOKEN_STAR);
      return;
    case '/':
      SINGLE_CHAR_TOK(TOKEN_SLASH);
      return;
    case ',':
      SINGLE_CHAR_TOK(TOKEN_COMMA);
      return;
    case '=':
      SINGLE_CHAR_TOK(TOKEN_EQUAL);
      return;
  }
#undef SINGLE_CHAR_TOK

  fprintf(stderr, "Encountered an unknown character '%c'.\n", CURRENT_CHAR(source));
  token->kind = TOKEN_ERROR;
  token->start = *source;
  token->len = 1;
  ADVANCE(source);
}
#undef ADVANCE
#undef CURRENT_CHAR
