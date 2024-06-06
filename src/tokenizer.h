#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum TokenType {
  TOK_INVALID,
  TOK_CMD_TABLES,
  TOK_CMD_DBINFO,

  TOK_COMMA,
  TOK_SEMICOLON,
  TOK_STAR,
  TOK_LPAREN,
  TOK_RPAREN,

  TOK_IDENTIFIER,

  TOK_KW_CREATE,
  TOK_KW_TABLE,
  TOK_KW_SELECT,
  TOK_KW_FROM,

  TOK_EOF,
} TokenType;

typedef struct Loc {
  char *start;
  int length;
} Loc;

typedef struct Token {
  TokenType tag;
  Loc loc;
} Token;

typedef struct Tokenizer {
  const char *source;
  char *start;
  char *pos;
} Tokenizer;

void tokenizer_init(Tokenizer *t, const char *source);
Token next_token(Tokenizer *t);

#endif  // TOKENIZER_H
