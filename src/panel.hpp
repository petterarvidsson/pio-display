#pragma once
#include <optional>
#include "control.hpp"
#include "displays.hpp"
#include "input.hpp"

namespace sdhi {
  class Panel {
    std::string_view title;
    std::span<std::optional<std::reference_wrapper<Controllable>>, 9> cs;
  public:
    Panel(std::string_view title, std::span<std::optional<std::reference_wrapper<Controllable>>, 9> controls);
    void display_items( std::vector<displays::DisplayItem> &items);
    void update(int32_t * const change, input::ControllerButtonState * const trigger);
  };

}
