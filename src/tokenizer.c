#include "tokenizer.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

void tokenizer_init(Tokenizer *t, const char *source) {
  t->source = source;
  t->start = (char *)source;
  t->pos = (char *)source;
}

inline static void advance(Tokenizer *t) { t->pos++; }

inline static char peek(Tokenizer *t) { return *t->pos; }

inline static char consume(Tokenizer *t) {
  char c = peek(t);
  advance(t);
  return c;
}

inline static bool finished(Tokenizer *t) { return *t->pos == '\0'; }

inline static bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

inline static bool is_digit(char c) { return c >= '0' && c <= '9'; }

inline static bool is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void skip_whitespace(Tokenizer *t) {
  while (is_whitespace(peek(t))) {
    advance(t);
  }
}

static bool check(Tokenizer *t, int len, const char *s) {
  int matched = t->pos - t->start;
  int cmp = strncasecmp(t->start, s, len);
  return (matched == len && cmp == 0);
}

static TokenType check_command(Tokenizer *t, int len, const char *s,
                               TokenType tag) {
  return check(t, len, s) ? tag : TOK_INVALID;
}

static TokenType check_keyword(Tokenizer *t, int len, const char *s,
                               TokenType tag) {
  return check(t, len, s) ? tag : TOK_IDENTIFIER;
}

static Token emit(Tokenizer *t, TokenType tag) {
  return (Token){
      .tag = tag,
      .loc =
          {
              .start = t->start,
              .length = t->pos - t->start,
          },
  };
}

static TokenType match_command(Tokenizer *t) {
  // 1 because we've already consumed the '.'
  switch (t->start[1]) {
    case 't':
      return check_command(t, 7, ".tables", TOK_CMD_TABLES);
    case 'd':
      return check_command(t, 7, ".dbinfo", TOK_CMD_DBINFO);
    default:
      return TOK_INVALID;
  }
}

static TokenType match_keyword(Tokenizer *t) {
  switch (tolower(t->start[0])) {
    case 's':
      return check_keyword(t, 6, "select", TOK_KW_SELECT);
    case 't':
      return check_keyword(t, 5, "table", TOK_KW_TABLE);
    case 'c':
      return check_keyword(t, 6, "create", TOK_KW_CREATE);
    case 'f':
      return check_keyword(t, 4, "from", TOK_KW_FROM);
    default:
      return TOK_IDENTIFIER;
  }
}

static Token command(Tokenizer *t) {
  while (is_alpha(peek(t))) {
    advance(t);
  }
  TokenType tag = match_command(t);
  return emit(t, tag);
}

static Token identifer(Tokenizer *t) {
  while (is_alpha(peek(t)) || is_digit(peek(t)) || peek(t) == '_') {
    advance(t);
  }
  TokenType tag = match_keyword(t);
  return emit(t, tag);
}

Token next_token(Tokenizer *t) {
  skip_whitespace(t);
  t->start = t->pos;
  if (finished(t)) {
    return emit(t, TOK_EOF);
  }
  char c = consume(t);

  // Commands start with a '.'
  if (c == '.') {
    return command(t);
  }

  // Identifiers start with a letter or an underscore
  if (is_alpha(c) || c == '_') {
    return identifer(t);
  }

  // Single character tokens
  switch (c) {
    case ',':
      return emit(t, TOK_COMMA);
    case ';':
      return emit(t, TOK_SEMICOLON);
    case '*':
      return emit(t, TOK_STAR);
    case '(':
      return emit(t, TOK_LPAREN);
    case ')':
      return emit(t, TOK_RPAREN);
  }

  return emit(t, TOK_INVALID);
}
