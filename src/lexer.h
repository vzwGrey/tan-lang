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
  TOKEN_EQUAL,
  TOKEN_IDENT,
  TOKEN_OPAREN,
  TOKEN_CPAREN,
  TOKEN_OBRACE,
  TOKEN_CBRACE,
  TOKEN_FN,
} TOKEN_KIND;

typedef struct
{
  TOKEN_KIND kind;
  char const *start;
  size_t len;
} TOKEN;

char const *TokenKindName(TOKEN_KIND kind);
void NextToken(char const **source, TOKEN *token);
