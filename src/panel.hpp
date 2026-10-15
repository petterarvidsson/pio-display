#pragma once
#include <optional>
#include "control.hpp"
#include "displays.hpp"

namespace sdhi {
  class Panel {
    std::string_view title;
    std::span<std::optional<std::reference_wrapper<Controllable>>, 9> cs;
    std::array<displays::Drawable, 49> drawables;
  public:
    Panel(std::string_view title, std::span<std::optional<std::reference_wrapper<Controllable>>, 9> controls);
    std::span<displays::Drawable> display_list();
  };

}
