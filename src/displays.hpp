#pragma once
#include <stdint.h>

namespace displays {
  enum FontSize { size_13 = 0, size_18 = 1, size_28 = 2, size_32 = 3 };

  class Display {
    uint8_t * fb;
    int index;
  public:
    constexpr Display(uint8_t * fb, int index) : fb(fb), index(index) {}
    void set_pixel(const uint8_t x, const uint8_t y, const bool on) const;
    void line(const uint8_t x0, const uint8_t y0, const uint8_t x1, const uint8_t y1, const uint8_t size) const;
    void circle(const uint8_t x0, const uint8_t y0, const uint8_t radius) const;
    void sine(const uint8_t from, const uint8_t to, const uint8_t x, const uint8_t y, const uint8_t length, const uint8_t amplitude) const;
    void filled_circle(const uint8_t x0, const uint8_t y0, const uint8_t radius) const;
    void rectangle(const uint8_t startx, const uint8_t starty,
                          const uint8_t endx, const uint8_t endy) const;
    void printc(const uint8_t startx, const uint8_t starty, const FontSize font_size, const bool on, const char c) const;
    void print(const uint8_t startx, const uint8_t starty, const FontSize font_size, const bool on, const char * const str) const;
    void print_center(uint8_t * const fb, const uint8_t y, const FontSize font_size, const bool on, const char * const str) const;
  };

  void init();
  void clear();
  void flip();
  void wait_ready();
  Display get(unsigned int index);
}
