#include "varint.h"

#include <stdint.h>

// TODO disater if buf is not at least 9 bytes long
VarInt sq_varint_decode(const uint8_t *buf) {
  uint64_t value = 0;
  int i = 0;

  for (; i < 9; ++i) {
    uint8_t const byte = buf[i];

    // The first 8 bytes use 7 bits, the last uses all 8.
    if (i < 8)
      value = (value << 7) | (byte & 0x7F);
    else
      value = (value << 8) | byte;

    // Check the continuation bit
    if ((byte & 0x80) == 0) break;
  }

  return (VarInt){.value = value, .bytes_read = i + 1};
}
