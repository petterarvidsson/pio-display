#pragma once
#include "displays.hpp"
#include <string>
#include <vector>

namespace sdhi {

  class Controllable {
  public:
    virtual std::string_view get_group() = 0;
    virtual void update(int steps) = 0;
    virtual void display_items(const uint8_t control, std::vector<displays::Item> &items) = 0;
  };

  class Control : public Controllable {
    std::string_view group;
    std::string title;
  public:
    constexpr Control(std::string title, std::string_view group) : title(title), group(group) {}
    void display_items(const uint8_t control, std::vector<displays::Item> &items);
    std::string_view get_group();
  };

  class IntControl : public Control {
    int min;
    int max;
    int step;
  public:
    int value;
    constexpr IntControl(std::string title, std::string_view group, int min, int max, int step = 1, int initial = 0) : Control(title, group), min(min), max(max), step(step), value(initial) {}
    void display_items(const uint8_t control, std::vector<displays::Item> &items);
    void update(int steps);
  };
}
