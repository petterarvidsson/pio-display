#include "overloaded.hpp"
#include "panel.hpp"
#include <ranges>
namespace sdhi {
  using namespace displays;
  static Point start_point = Point(0, 31);
  static Point end_point = Point(127, 31);
  static Point top_point = Point(63, 0);
  static Point bottom_point = Point(63, 63);
  static Point middle_point = Point(63, 31);
  static uint8_t width;
  void separator_items(bool start, bool end, bool top, bool bottom, uint8_t display, std::vector<displays::Item> &items) {
    if(start && end && top && bottom) {
      items.push_back(Line(display, start_point, end_point, width));
      items.push_back(Line(display, top_point, bottom_point, width));
    } else if(start && end && top) {
      items.push_back(Line(display, start_point, end_point, width));
      items.push_back(Line(display, top_point, middle_point, width));
    } else if(start && end && bottom){
      items.push_back(Line(display, start_point, end_point, width));
      items.push_back(Line(display, middle_point, bottom_point, width));
    } else if(start && top && bottom) {
      items.push_back(Line(display, start_point, middle_point, width));
      items.push_back(Line(display, top_point, bottom_point, width));
    } else if(end && top && bottom) {
      items.push_back(Line(display, middle_point, end_point, width));
      items.push_back(Line(display, top_point, bottom_point, width));
    } else if(start && end) {
      items.push_back(Line(display, start_point, end_point, width));
    } else if(top && bottom) {
      items.push_back(Line(display, top_point, bottom_point, width));
    } else if(start && top) {
      items.push_back(Line(display, start_point, middle_point, width));
      items.push_back(Line(display, top_point, middle_point, width));
    } else if(start && bottom) {
      items.push_back(Line(display, start_point, middle_point, width));
      items.push_back(Line(display, middle_point, bottom_point, width));
    } else if(end && top) {
      items.push_back(Line(display, middle_point, end_point, width));
      items.push_back(Line(display, top_point, middle_point, width));
    } else if(end && bottom) {
      items.push_back(Line(display, middle_point, end_point, width));
      items.push_back(Line(display, middle_point, bottom_point, width));
    }
  }

  struct Cross {
    int start_top;
    int end_top;
    int start_bottom;
    int end_bottom;
    Cross(int start_top, int end_top, int start_bottom, int end_bottom) : start_top(start_top), end_top(end_top), start_bottom(start_bottom), end_bottom(end_bottom) {}
  };

  struct Bar {
    int top;
    int bottom;
    Bar(int top, int bottom) : top(top), bottom(bottom) {}
  };

  using DisplayControl = std::variant<Cross, Bar>;

  static std::array<const DisplayControl, 40> ctrl_index = {
    Cross(-1,-1,-1, 0), Bar(-1, 0), Cross(-1,-1, 0, 1), Bar(-1, 1), Cross(-1,-1, 1, 2), Bar(-1, 2), Cross(-1,-1, 2,-1),
    Bar(-1,  0),        /*0*/       Bar( 0, 1),         /*1*/       Bar( 1, 2),         /*2*/       Bar( 2,-1),
    Cross(-1, 0,-1, 3), Bar( 0, 3), Cross( 0, 1, 3, 4), Bar( 1, 4), Cross( 1, 2, 4, 5), Bar(2, 5),  Cross( 2,-1, 5,-1),
    Bar(-1, 3),         /*3*/       Bar( 3, 4),         /*4*/       Bar( 4, 5),         /*5*/       Bar( 5,-1),
    Cross(-1, 3,-1, 6), Bar( 3, 6), Cross( 3, 4, 6, 7), Bar( 4, 7), Cross( 4, 5, 7, 8), Bar(5, 8),  Cross( 5,-1, 8,-1),
    Bar(-1, 6),         /*6*/       Bar( 6, 7),         /*7*/       Bar( 7, 8),         /*8*/       Bar( 8,-1),
    Cross(-1, 6,-1,-1), Bar( 6,-1), Cross( 6, 7,-1,-1), Bar( 7,-1), Cross( 7, 8,-1,-1), Bar(8,-1),  Cross( 8,-1,-1,-1)
  };

  static std::string_view group_of(Controllable &c) {
    return c.get_group();
  }
  static void sp(std::span<std::optional<std::reference_wrapper<Controllable>>, 9> cs, uint8_t display, std::vector<displays::Item> &items) {
    return std::visit(util::overloaded{
        [cs, display, &items](const Cross cross) {
          auto start_top = cross.start_top == -1 ? std::nullopt : cs[cross.start_top].transform(group_of);
          auto end_top = cross.end_top == -1 ? std::nullopt : cs[cross.end_top].transform(group_of);
          auto start_bottom = cross.start_bottom == -1 ? std::nullopt : cs[cross.start_bottom].transform(group_of);
          auto end_bottom = cross.end_bottom == -1 ? std::nullopt : cs[cross.end_bottom].transform(group_of);
          separator_items(start_top != start_bottom, end_top != end_bottom, start_top != end_top, start_bottom != end_bottom, display, items);
        },
          [cs, display, &items](const Bar bar) {
            auto top = bar.top == -1 ? std::nullopt : cs[bar.top].transform(group_of);
            auto bottom = bar.bottom == -1 ? std::nullopt : cs[bar.bottom].transform(group_of);
            separator_items(top != bottom, top != bottom, false, false, display, items);
          }
          }, ctrl_index[display]);
  }

  Panel::Panel(std::string_view title, std::span<std::optional<std::reference_wrapper<Controllable>>, 9> controls) :
    title(title),
    cs(controls) {
  }

  void Panel::display_items(std::vector<Item> &items) {
    for(int d = 0; d < 40; d++)
      sp(cs, d, items);
    for(int c = 0; c < 9; c++) {
      if(cs[c].has_value()) {
        cs[c].value().get().display_items(c, items);
      }
    }
  }

  void Panel::update(int32_t * const change, input::ControllerButtonState * const trigger) {
    auto active_controllers = cs
      | std::views::filter([](auto opt) {return bool(opt);})
      | std::views::transform([](auto opt) {return opt.value();})
      | std::views::enumerate;

    for(auto [i, c]: active_controllers) {
      c.get().update(change[i]);
    }
  }
}
