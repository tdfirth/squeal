#ifndef VARINT_H
#define VARINT_H

#include <stdint.h>

typedef struct VarInt {
  uint64_t value;      // The value of the VarInt
  uint8_t bytes_read;  // The number of bytes read to build the VarInt
} VarInt;

VarInt sq_varint_decode(const uint8_t *buf);

#endif  // VARINT_H
