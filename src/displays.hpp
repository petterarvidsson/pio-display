#pragma once
#include <stdint.h>

namespace displays {
  class Display {
    uint8_t * fb;
    int index;
  public:
    constexpr Display(uint8_t * fb, int index) : fb(fb), index(index) {}
    void set_pixel(const uint8_t x, const uint8_t y, const bool on) const;
  };

  void init();
  void clear();
  void update();
  Display get(unsigned int index);
}
