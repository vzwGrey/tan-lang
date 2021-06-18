#pragma once

#include <stdlib.h>

typedef enum
{
  TOKEN_EOF = 0,
  TOKEN_ERROR,
  TOKEN_NUMBER,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_COMMA,
} TOKEN_KIND;

typedef struct
{
  TOKEN_KIND kind;
  char const *start;
  size_t len;
} TOKEN;

char const *TokenKindName(TOKEN_KIND kind);
void NextToken(char const **source, TOKEN *token);
