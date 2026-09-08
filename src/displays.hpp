#pragma once
#include <stdint.h>
#include <variant>
#include <span>

namespace displays {
  enum FontSize { size_13 = 0, size_18 = 1, size_28 = 2, size_32 = 3 };
  struct Point {
    uint8_t x;
    uint8_t y;
    Point(const uint8_t x, const uint8_t y) : x(x), y(y) {}
  };
  struct Line {
    Point start;
    Point end;
    uint8_t size;
    Line(const Point start, const Point end, const uint8_t size) : start(start), end(end), size(size) {}
  };

  struct Circle {
    Point center;
    uint8_t radius;
    Circle(const Point center, const uint8_t radius) : center(center), radius(radius) {}
  };

  struct FilledCircle {
    Point center;
    uint8_t radius;
    FilledCircle(const Point center, const uint8_t radius) : center(center), radius(radius) {}
  };

  struct SineSegment {
    Point start;
    uint8_t length;
    uint8_t amplitude;
    /* To and from are expressed in PI / 8 */
    uint8_t from;
    uint8_t until;
    SineSegment(const Point start, const uint8_t length, const uint8_t amplitude, const uint8_t from, const uint8_t until) : start(start), length(length), amplitude(amplitude), from(from), until(until) {}
  };

  struct Print {
    uint8_t startx;
    uint8_t starty;
    FontSize font_size;
    char * const str;
    Print(uint8_t startx, uint8_t starty, FontSize font_size, char * const str) : startx(startx), starty(starty), font_size(font_size), str(str) {}
  };

  struct PrintCenter {
    uint8_t y;
    FontSize font_size;
    char * const str;
    PrintCenter(uint8_t y, FontSize font_size, char * const str);
  };

  using Item = std::variant<Line, Circle, FilledCircle, SineSegment>; //t, Print, PrintCenter

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
    void draw(std::span<Item> display_tems) const;
  };

  void init();
  void clear();
  void flip();
  void wait_ready();
  Display get(unsigned int index);
}
