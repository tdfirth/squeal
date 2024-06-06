#include <stdio.h>

#include "test.h"
#include "tokenizer.h"

MunitResult test_capture_identifier(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  Tokenizer t;
  tokenizer_init(&t, "count(*)");
  Token token = next_token(&t);
  munit_assert_true(token.tag == TOK_IDENTIFIER);
  munit_assert_memory_equal(token.loc.length, token.loc.start, "count");
  return MUNIT_OK;
}

MunitResult test_tokenize(const char* stmt, const Token* tokens) {
  Tokenizer t;
  tokenizer_init(&t, stmt);
  for (int i = 0; tokens[i].tag != TOK_EOF; i++) {
    Token token = next_token(&t);
    munit_assert_true(token.tag == tokens[i].tag);
  }
  return MUNIT_OK;
}

MunitResult test_cmd_tables(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  Token tokens[] = {
      {.tag = TOK_CMD_TABLES},
      {.tag = TOK_EOF},
  };
  return test_tokenize(".tables", tokens);
}

MunitResult test_cmd_dbinfo(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  Token tokens[] = {
      {.tag = TOK_CMD_DBINFO},
      {.tag = TOK_EOF},
  };
  return test_tokenize(".dbinfo", tokens);
}

MunitResult test_sqlite_schema(const MunitParameter params[], void* data) {
  char* stmt =
      "CREATE TABLE sqlite_schema( \
         type text, \
         name text, \
         tbl_name text, \
         rootpage integer, \
         sql text \
       );";
  Token tokens[] = {
      {.tag = TOK_KW_CREATE}, {.tag = TOK_KW_TABLE},   {.tag = TOK_IDENTIFIER},
      {.tag = TOK_LPAREN},    {.tag = TOK_IDENTIFIER}, {.tag = TOK_IDENTIFIER},
      {.tag = TOK_COMMA},     {.tag = TOK_IDENTIFIER}, {.tag = TOK_IDENTIFIER},
      {.tag = TOK_COMMA},     {.tag = TOK_IDENTIFIER}, {.tag = TOK_IDENTIFIER},
      {.tag = TOK_COMMA},     {.tag = TOK_IDENTIFIER}, {.tag = TOK_IDENTIFIER},
      {.tag = TOK_COMMA},     {.tag = TOK_IDENTIFIER}, {.tag = TOK_IDENTIFIER},
      {.tag = TOK_RPAREN},    {.tag = TOK_SEMICOLON},  {.tag = TOK_EOF},
  };
  return test_tokenize(stmt, tokens);
}

MunitResult test_case_insensitive_keywords(const MunitParameter params[],
                                           void* data) {
  char* stmt = "SeLeCt FROM table CREAte";
  Token tokens[] = {
      {.tag = TOK_KW_SELECT}, {.tag = TOK_KW_FROM}, {.tag = TOK_KW_TABLE},
      {.tag = TOK_KW_CREATE}, {.tag = TOK_EOF},
  };
  return test_tokenize(stmt, tokens);
}

MunitTest tokenizer_tests[] = {
    TEST("/cmd/.tables", test_cmd_tables),
    TEST("/cmd/.dbinfo", test_cmd_dbinfo),
    TEST("/sqlite-schema", test_sqlite_schema),
    TEST("/keywords-are-case-insensitive", test_case_insensitive_keywords),
    TEST("/capture-identifier", test_capture_identifier),
    NULL_TEST,
};

const MunitSuite tokenizer_suite = {
    "/tokenizer",            // name
    tokenizer_tests,         // tests
    NULL,                    // suites
    1,                       // iterations
    MUNIT_SUITE_OPTION_NONE  // options
};
