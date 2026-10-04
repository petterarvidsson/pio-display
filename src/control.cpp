#include "control.hpp"

constexpr Drawable Control::title_drawable(uint8_t control, Drawable drawable) {
  drawables[0] = DisplayList(title_items, control, 1);
  drawables[1] = drawable;
  return drawables;
}
std::string_view Control::get_group() {
  return group;
}
constexpr Drawable IntControl::drawable(uint8_t control) {
  value_string = std::to_string(value);
  return title_drawable(control, DisplayList(items, control, 6));
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

