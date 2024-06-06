#ifndef TEST_H
#define TEST_H

#include "munit.h"

#define NULL_TEST \
  (MunitTest) { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

#define NULL_SUITE \
  { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }

#define TEST(description, name) \
  { description, name, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

extern const MunitSuite experiment_suite;
extern const MunitSuite varint_suite;
extern const MunitSuite tokenizer_suite;

#endif  // TEST_H
