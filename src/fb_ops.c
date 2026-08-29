#include "fb_ops.h"

void split_byte_into(const uint8_t byte, uint8_t *target, const uint32_t displays) {
  for(uint32_t b = 0; b < 8; b++) {
    uint32_t v = (byte >> b) & 0x1;
    for(uint32_t d = 0; d < displays; d++) {
      uint32_t bit = b * displays + d;
      uint32_t i_off = (displays - 1) - bit / 8;
      uint32_t bit_i = bit % 8;
      target[i_off] |= (v << bit_i);
    }
  }
}

void split_bytes_into(const uint8_t * const bytes, size_t size, uint8_t *target, const uint32_t displays) {
  for(size_t i = 0; i < size; i++) {
    split_byte_into(bytes[i], target + i * displays, displays);
  }
}
