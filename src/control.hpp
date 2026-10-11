#pragma once
#include "displays.hpp"
#include <string>
#include <array>

namespace sdhi {

  class Controllable {
  public:
    virtual std::string_view get_group() = 0;
    virtual void update(int steps) = 0;
    virtual displays::Drawable drawable(uint8_t control) = 0;
  };

  class Control : public Controllable {
    std::array<displays::Item, 1> title_items;
    std::string_view group;
    std::array<displays::Drawable, 2> drawables;
  public:
    constexpr Control(std::string_view title, std::string_view group) : group(group), title_items({displays::PrintCenter(63 - 13, displays::size_13, title)}), drawables() {}
    constexpr displays::Drawable title_drawable(uint8_t control, displays::Drawable drawable);
    std::string_view get_group();
  };

  class IntControl : public Control {
    int min;
    int max;
    int step;
    std::string value_string;
    std::array<displays::Item, 1> items;
    static constexpr std::string longest_string(int min, int max) {
      auto min_str = std::to_string(min);
      auto max_str = std::to_string(min);
      if(min_str.length() > max_str.length()) {
        return min_str;
      } else {
        return max_str;
      }
    }
  public:
    int value;
    constexpr IntControl(std::string_view title, std::string_view group, int min, int max, int step = 1, int initial = 0) : Control(title, group), min(min), max(max), step(step), value(initial), value_string(longest_string(min, max)), items({displays::PrintCenter(0, displays::size_13, value_string)}) {}
    constexpr displays::Drawable drawable(uint8_t control);
    void update(int steps);
  };
}
