#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "stdio.h"
#include "string.h"
#include "displays.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <initializer_list>
using namespace displays;

static constexpr Line start = Line(Point(0, 31), Point(63, 31), 2);
static constexpr Line end = Line(Point(63, 31), Point(127, 31), 2);
static constexpr Line top = Line(Point(63, 0), Point(63, 31), 2);
static constexpr Line bottom = Line(Point(63, 31), Point(63, 63), 2);

static constexpr std::array<Item, 2> start_end = {
  start,
  end
};
static constexpr std::array<Item, 2> top_bottom = {
  top,
  bottom
};
static constexpr std::array<Item, 2> start_top = {
  start,
  top
};
static constexpr std::array<Item, 2> start_bottom = {
  start,
  bottom
};
static constexpr std::array<Item, 2> end_top = {
  end,
  top
};
static constexpr std::array<Item, 2> end_bottom = {
  end,
  bottom
};
static constexpr std::array<Item, 4> start_end_top_bottom = {
  start,
  end,
  top,
  bottom
};
static constexpr std::array<Item, 3> start_end_top = {
  start,
  end,
  top
};
static constexpr std::array<Item, 3> start_end_bottom = {
  start,
  end,
  bottom
};
static constexpr std::array<Item, 3> start_top_bottom = {
  start,
  top,
  bottom
};
static constexpr std::array<Item, 3> end_top_bottom = {
  end,
  top,
  bottom
};
static constexpr std::array<Item, 0> empty = {
};

constexpr Drawable Separator(bool start, bool end, bool top, bool bottom, uint8_t display) {
  if(start && end && top && bottom) {
    return Drawable(start_end_top_bottom, display);
  } else if(start && end && top) {
    return Drawable(start_end_top, display);
  } else if(start && end && bottom){
    return Drawable(start_end_bottom, display);
  } else if(start && top && bottom) {
    return Drawable(start_top_bottom, display);
  } else if(end && top && bottom) {
    return Drawable(end_top_bottom, display);
  } else if(start && end) {
    return Drawable(start_end, display);
  } else if(top && bottom) {
    return Drawable(top_bottom, display);
  } else if(start && top) {
    return Drawable(start_top, display);
  } else if(start && bottom) {
    return Drawable(start_bottom, display);
  } else if(end && top) {
    return Drawable(end_top, display);
  } else if(end && bottom) {
    return Drawable(end_bottom, display);
  } else {
    return Drawable(empty, display);
  }
}
class Controllable {
public:
  virtual std::string_view get_title();
  virtual std::string_view get_group();
  virtual void update(int steps);
  virtual Drawable drawable(uint8_t display);
};
class Control final : public Controllable {
  PrintCenter print;
  std::array<Item, 1> items;
  std::string_view title;
  std::string_view group;
public:
  constexpr Control(std::string_view title, std::string_view group) : title(title), group(group), print(63 - 13, size_13, title), items({print}) {}
  constexpr Drawable drawable(uint8_t display) {
    items[0] = print;
    return Drawable(items, display);
  }
  void update(int step) {
    print.y++;
    if(print.y > 63 - 13) {
      print.y = 13;
    }
  }
  std::string_view get_title() {
    return title;
  }
  std::string_view get_group() {
    return group;
  }
};

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

static const std::array<const DisplayControl, 40> ctrl_index = {
  Cross(-1,-1,-1, 0), Bar(-1, 0), Cross(-1,-1, 0, 1), Bar(-1, 1), Cross(-1,-1, 1, 2), Bar(-1, 2), Cross(-1,-1, 2,-1),
  Bar(-1,  0),        /*0*/       Bar( 0, 1),         /*1*/       Bar( 1, 2),         /*2*/       Bar( 2,-1),
  Cross(-1, 0,-1, 3), Bar( 0, 3), Cross( 0, 1, 3, 4), Bar( 1, 4), Cross( 1, 2, 4, 5), Bar(2, 5),  Cross( 2,-1, 5,-1),
  Bar(-1, 3),         /*3*/       Bar( 3, 4),         /*4*/       Bar( 4, 5),         /*5*/       Bar( 5,-1),
  Cross(-1, 3,-1, 6), Bar( 3, 6), Cross( 3, 4, 6, 7), Bar( 4, 7), Cross( 4, 5, 7, 8), Bar(5, 8),  Cross( 5,-1, 8,-1),
  Bar(-1, 6),         /*6*/       Bar( 6, 7),         /*7*/       Bar( 7, 8),         /*8*/       Bar( 8,-1),
  Cross(-1, 6,-1,-1), Bar( 6,-1), Cross( 6, 7,-1,-1), Bar( 7,-1), Cross( 7, 8,-1,-1), Bar(8,-1),  Cross( 8,-1,-1,-1)
};
template<class... Ts>
  struct overloaded : Ts... { using Ts::operator()...; };


class Panel {
  std::string_view title;
  std::span<std::optional<std::reference_wrapper<Controllable>>, 9> cs;
  static constexpr std::string_view group_of(Controllable &c) {
    return c.get_group();
  }
  static constexpr Drawable sp(std::span<std::optional<std::reference_wrapper<Controllable>>, 9> cs, uint8_t display) {
    return std::visit(overloaded{
        [cs, display](const Cross cross) {
          auto start_top = cross.start_top == -1 ? std::nullopt : cs[cross.start_top].transform(group_of);
          auto end_top = cross.end_top == -1 ? std::nullopt : cs[cross.end_top].transform(group_of);
          auto start_bottom = cross.start_bottom == -1 ? std::nullopt : cs[cross.start_bottom].transform(group_of);
          auto end_bottom = cross.end_bottom == -1 ? std::nullopt : cs[cross.end_bottom].transform(group_of);
          return Separator(start_top != start_bottom, end_top != end_bottom, start_top != end_top, start_bottom != end_bottom, display);
        },
          [cs, display](const Bar bar) {
            auto top = bar.top == -1 ? std::nullopt : cs[bar.top].transform(group_of);
            auto bottom = bar.bottom == -1 ? std::nullopt : cs[bar.bottom].transform(group_of);
            return Separator(top != bottom, top != bottom, false, false, display);
          }
          }, ctrl_index[display]);
  }
  static constexpr Drawable cd(std::optional<std::reference_wrapper<Controllable>> c, uint8_t display) {
    return c.transform([display] (Controllable& c) {
      return c.drawable(display);
    }).value_or(Drawable(empty, display));
  }
  std::array<Drawable, 49> drawables;
public:
  constexpr Panel(std::string_view title, std::span<std::optional<std::reference_wrapper<Controllable>>, 9> controls) :
    title(title),
    cs(controls),
    drawables({
      sp(cs,  0), sp(cs,  1), sp(cs,  2), sp(cs,  3), sp(cs,  4), sp(cs,  5), sp(cs,  6),
      sp(cs,  7),             sp(cs,  8),             sp(cs,  9),             sp(cs, 10),
      sp(cs, 11), sp(cs, 12), sp(cs, 13), sp(cs, 14), sp(cs, 15), sp(cs, 16), sp(cs, 17),
      sp(cs, 18),             sp(cs, 19),             sp(cs, 20),             sp(cs, 21),
      sp(cs, 22), sp(cs, 23), sp(cs, 24), sp(cs, 25), sp(cs, 26), sp(cs, 27), sp(cs, 28),
      sp(cs, 29),             sp(cs, 30),             sp(cs, 31),             sp(cs, 32),
      sp(cs, 33), sp(cs, 34), sp(cs, 35), sp(cs, 36), sp(cs, 37), sp(cs, 38), sp(cs, 39),
      cd(cs[0], 1),  cd(cs[1], 3),  cd(cs[2], 5),
      cd(cs[3], 12), cd(cs[4], 14), cd(cs[5], 16),
      cd(cs[6], 23), cd(cs[7], 25), cd(cs[8], 27)
    }) {}
  std::span<Drawable> display_list() {
    drawables[40] = cd(cs[0], 1);
    drawables[41] = cd(cs[1], 3);
    drawables[42] = cd(cs[2], 5);
    drawables[43] = cd(cs[3], 12);
    drawables[44] = cd(cs[4], 14);
    drawables[45] = cd(cs[5], 16);
    drawables[46] = cd(cs[6], 23);
    drawables[47] = cd(cs[7], 25);
    drawables[48] = cd(cs[8], 27);
    return drawables;
  }
};

Control c1("FEEDBACK", "1");
Control c2("FEEDBACK2", "1");
Control c3("FEEDBACK", "1");
Control c4("FEEDBACK2", "2");

std::array<std::optional<std::reference_wrapper<Controllable>>, 9> controls = {
  c1, c2, c3,
  c4, std::nullopt, std::nullopt,
  std::nullopt, std::nullopt, std::nullopt
};

Panel panel = Panel("Ctrls", controls);

void graphics() {
  while (1) {
    render();
  }
}

int main() {
  stdio_init_all();

  init();
  multicore_launch_core1(graphics);
  int32_t c = 0;
  while(true) {
    c1.update(1);
    if(is_ready()) {
      flip(panel.display_list());
    }
    if(c % 100000 == 0) {
      stats();
    }
    c++;
  }
}
