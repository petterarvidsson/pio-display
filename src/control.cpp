#include "control.hpp"

namespace sdhi {
  void Control::display_items(uint8_t control, std::vector<displays::Item> &items) {
    items.push_back(displays::PrintCenter(displays::cd( control, 1), 63 - 13, displays::size_13, title));
  }
  std::string_view Control::get_group() {
    return group;
  }
  void IntControl::display_items(uint8_t control, std::vector<displays::Item> &items) {
    Control::display_items(control, items);
    items.push_back(displays::PrintCenter(displays::cd(control, 6), 0, displays::size_13, std::to_string(value)));
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
