#pragma once
#include <cstdint>
#include <array>
#include <variant>
#include <string>
#include <vector>

namespace protocol {
  enum FontSize { size_13 = 0, size_18 = 1, size_28 = 2, size_32 = 3 };
  struct Point final {
    uint8_t x;
    uint8_t y;
    bool operator==(const Point&) const = default;
    constexpr Point(const uint8_t x, const uint8_t y) : x(x), y(y) {}
  };

  struct Line final {
    uint8_t display;
    Point start;
    Point end;
    uint8_t size;
    bool operator==(const Line&) const = default;
    constexpr Line(const uint8_t display, const Point start, const Point end, const uint8_t size) : display(display), start(start), end(end), size(size) {}
  };

  struct Circle final {
    uint8_t display;
    Point center;
    uint8_t radius;
    bool operator==(const Circle&) const = default;
    constexpr Circle(const uint8_t display, const Point center, const uint8_t radius) : display(display), center(center), radius(radius) {}
  };

  struct FilledCircle final {
    uint8_t display;
    Point center;
    uint8_t radius;
    bool operator==(const FilledCircle&) const = default;
    constexpr FilledCircle(const uint8_t display, const Point center, const uint8_t radius) : display(display), center(center), radius(radius) {}
  };

  struct SineSegment final {
    uint8_t display;
    Point start;
    uint8_t length;
    uint8_t amplitude;
    /* To and from are expressed in PI / 8 */
    uint8_t from;
    uint8_t until;
    bool operator==(const SineSegment&) const = default;
    constexpr SineSegment(const uint8_t display, const Point start, const uint8_t length, const uint8_t amplitude, const uint8_t from, const uint8_t until) : display(display), start(start), length(length), amplitude(amplitude), from(from), until(until) {}
  };

  struct Print final {
    uint8_t display;
    Point start;
    FontSize font_size;
    std::array<char, 17> str;
    bool operator==(const Print&) const = default;
    constexpr Print(const uint8_t display, Point start, FontSize font_size, std::string_view str_view) : display(display), start(start), font_size(font_size), str({}) {
      str_view.copy(str.data(), str.size() - 1);
    }
  };

  struct PrintCenter final {
    uint8_t display;
    uint8_t y;
    FontSize font_size;
    std::array<char, 17> str;
    bool operator==(const PrintCenter&) const = default;
    constexpr PrintCenter(const uint8_t display, uint8_t y, FontSize font_size, std::string_view str_view) : display(display), y(y), font_size(font_size), str({}) {
      str_view.copy(str.data(), str.size() - 2);
    }
  };

  using Item = std::variant<Line, Circle, FilledCircle, SineSegment, Print, PrintCenter>;

  Item decode_item(const uint8_t * const buf, size_t &pos);
  void decode_items(const uint8_t * const buf, size_t size, std::vector<Item> &items);
  void encode_item(uint8_t *buf, size_t &pos, const Item item);
  size_t encode_items(uint8_t *buf, const std::vector<Item> items);
}
