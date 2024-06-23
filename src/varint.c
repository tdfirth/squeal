#include "varint.h"

#include <stdint.h>

int varint_decode(const uint8_t* buf, uint64_t* result) {
  uint8_t byte;
  uint64_t value = 0;
  int bytes_read = 0;

  do {
    byte = *buf++;
    value = (value << 7) | (byte & 0x7f);
    bytes_read++;
  } while ((byte & 0x80) && bytes_read < 8);

  // If we saw the continuatoion bit then there's a ninth byte.
  if ((byte & 0x80)) {
    bytes_read++;
    value = (value << 8) | *buf;
  }

  *result = value;
  return bytes_read;
}
