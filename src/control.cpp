#include "control.hpp"

namespace sdhi {
  void Control::display_items(uint8_t control, std::vector<displays::DisplayItem> &items) {
    items.push_back(displays::DisplayItem(displays::PrintCenter(63 - 13, displays::size_13, title), control, 1));
  }
  std::string_view Control::get_group() {
    return group;
  }
  void IntControl::display_items(uint8_t control, std::vector<displays::DisplayItem> &items) {
    Control::display_items(control, items);
    items.push_back(displays::DisplayItem(displays::PrintCenter(0, displays::size_13, std::to_string(value)), control, 6));
  }
  void IntControl::update(int steps) {
    int new_value = value + steps * step;
    if(new_value > max)
      value = max;
    else if(new_value < min)
      value = min;
    else
      value = new_value;
  }
}
