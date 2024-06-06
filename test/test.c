#include "test.h"

int main(int argc, char *argv[]) {
  MunitSuite suites[] = {
      experiment_suite,
      varint_suite,
      tokenizer_suite,
      NULL_SUITE,
  };

  MunitSuite test_suite = {
      "",                      // name
      NULL,                    // tests
      suites,                  // suites
      1,                       // iterations
      MUNIT_SUITE_OPTION_NONE  // options
  };

  return munit_suite_main(&test_suite, NULL, argc, argv);
}
