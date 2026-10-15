#pragma once
#include <cstdint>

namespace input {
  enum ControllerButtonState {
    no_change = -1, released = 0, pressed = 1
  };

  void init();
  bool read_change(int32_t * const change_update, ControllerButtonState * const button_update);
}
