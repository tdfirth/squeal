#include "test.h"

static char* foo_params[] = {"one", "two", "three", NULL};

static char* bar_params[] = {"red", "green", "blue", NULL};

static MunitParameterEnum params[] = {
    {"foo", foo_params},
    {"bar", bar_params},
    {NULL, NULL},
};

static MunitResult test_params(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;

  munit_assert_int(1, ==, 1);

  return MUNIT_OK;
}

static MunitTest experiment_tests[] = {
    {
        "/parameters",
        test_params,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        params,
    },
    NULL_TEST,
};

const MunitSuite experiment_suite = {
    "/experiment",           // name
    experiment_tests,        // tests
    NULL,                    // suites
    1,                       // iterations
    MUNIT_SUITE_OPTION_NONE  // options
};
