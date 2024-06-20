#ifndef VARINT_H
#define VARINT_H

#include <stdint.h>

int varint_decode(const uint8_t* buf, uint64_t* result);

#endif  // VARINT_H
