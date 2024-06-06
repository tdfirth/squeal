#include <stdint.h>

#include "test.h"
#include "varint.h"

static MunitResult test_decode(const uint8_t buf[], uint64_t want) {
  VarInt got = sq_varint_decode(buf);
  munit_assert_uint64(got.value, ==, want);
  return MUNIT_OK;
}

static MunitResult test_decode_1(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x01};
  return test_decode(buf, 0x1);
}

static MunitResult test_decode_2(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x81, 0x01};
  return test_decode(buf, 0x81);
}

static MunitResult test_decode_3(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x81, 0x80, 0x01};
  return test_decode(buf, 0x4001);
}

static MunitResult test_decode_4(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x81, 0x80, 0x80, 0x01};
  return test_decode(buf, 0x200001);
}

static MunitResult test_decode_5(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x81, 0x80, 0x80, 0x80, 0x01};
  return test_decode(buf, 0x10000001);
}

static MunitResult test_decode_6(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x81, 0x80, 0x80, 0x80, 0x80, 0x01};
  return test_decode(buf, 0x800000001);
}

static MunitResult test_decode_7(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01};
  return test_decode(buf, 0x40000000001);
}

static MunitResult test_decode_8(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01};
  return test_decode(buf, 0x2000000000001);
}

static MunitResult test_decode_9(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  // Ensure we use the top bit on the ninth byte
  const uint8_t buf[] = {0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x81};
  return test_decode(buf, 0x200000000000081);
}

static MunitResult test_decode_random_short(const MunitParameter params[],
                                            void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0x84, 0xcc, 0xd8, 0x85, 0x52};
  return test_decode(buf, 1234567890);
}

static MunitResult test_decode_random_long(const MunitParameter params[],
                                           void* data) {
  (void)params;
  (void)data;
  // Ensure we use the top bit on the ninth byte
  const uint8_t buf[] = {0xd5, 0xd5, 0x95, 0x98, 0xe7, 0xac, 0xbe, 0x8a, 0xd2};
  return test_decode(buf, 12345678901234567890U);
}

static MunitResult test_varint_x(const MunitParameter params[], void* data) {
  (void)params;
  (void)data;
  const uint8_t buf[] = {0xac, 0x9d, 0x91, 0xba, 0xe3, 0xbf, 0xc4, 0xb6, 0x77};
  return test_decode(buf, 6373872945363826295);
}

static MunitTest varint_tests[] = {
    TEST("/test_decode_1", test_decode_1),
    TEST("/test_decode_2", test_decode_2),
    TEST("/test_decode_3", test_decode_3),
    TEST("/test_decode_4", test_decode_4),
    TEST("/test_decode_5", test_decode_5),
    TEST("/test_decode_6", test_decode_6),
    TEST("/test_decode_7", test_decode_7),
    TEST("/test_decode_8", test_decode_8),
    TEST("/test_decode_9", test_decode_9),
    TEST("/test_decode_random_short", test_decode_random_short),
    TEST("/test_decode_random_long", test_decode_random_long),
    TEST("/test_varint_x", test_varint_x),
    NULL_TEST,
};

const MunitSuite varint_suite = {
    "/varint",               // name
    varint_tests,            // tests
    NULL,                    // suites
    1,                       // iterations
    MUNIT_SUITE_OPTION_NONE  // options
};
