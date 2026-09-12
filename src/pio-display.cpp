#include "pico/stdlib.h"
#include "stdio.h"
#include "string.h"
#include "displays.hpp"
#include <vector>
#include <string>

using namespace displays;

static constexpr Line start = Line(Point(0, 31), Point(63, 31), 2);
static constexpr Line end = Line(Point(63, 31), Point(127, 31), 2);
static constexpr Line top = Line(Point(63, 0), Point(63, 31), 2);
static constexpr Line bottom = Line(Point(63, 31), Point(63, 63), 2);

static constexpr std::array<Item, 2> start_end = {
  &start,
  &end
};
static constexpr std::array<Item, 2> top_bottom = {
  &top,
  &bottom
};
static constexpr std::array<Item, 2> start_top = {
  &start,
  &top
};
static constexpr std::array<Item, 2> start_bottom = {
  &start,
  &bottom
};
static constexpr std::array<Item, 2> end_top = {
  &end,
  &top
};
static constexpr std::array<Item, 2> end_bottom = {
  &end,
  &bottom
};
static constexpr std::array<Item, 4> start_end_top_bottom = {
  &start,
  &end,
  &top,
  &bottom
};
static constexpr std::array<Item, 3> start_end_top = {
  &start,
  &end,
  &top
};
static constexpr std::array<Item, 3> start_end_bottom = {
  &start,
  &end,
  &bottom
};
static constexpr std::array<Item, 3> start_top_bottom = {
  &start,
  &top,
  &bottom
};
static constexpr std::array<Item, 3> end_top_bottom = {
  &end,
  &top,
  &bottom
};
static constexpr std::array<Item, 0> empty = {
};


class SeparatorStartEndTopBottom : public Drawable {
public:
  SeparatorStartEndTopBottom(const uint8_t display) : Drawable(start_end_top_bottom, display) {}
};

class SeparatorStartEndTop : public Drawable {
public:
  SeparatorStartEndTop(const uint8_t display) : Drawable(start_end_top, display) {}
};

class SeparatorStartEndBottom : public Drawable {
public:
  SeparatorStartEndBottom(const uint8_t display) : Drawable(start_end_bottom, display) {}
};

class SeparatorStartTopBottom : public Drawable {
public:
  SeparatorStartTopBottom(const uint8_t display) : Drawable(start_top_bottom, display) {}
};

class SeparatorEndTopBottom : public Drawable {
public:
  SeparatorEndTopBottom(const uint8_t display) : Drawable(end_top_bottom, display) {}
};

class SeparatorStartEnd : public Drawable {
public:
  SeparatorStartEnd(const uint8_t display) : Drawable(start_end, display) {}
};

class SeparatorTopBottom : public Drawable {
public:
  SeparatorTopBottom(const uint8_t display) : Drawable(top_bottom, display) {}
};

class SeparatorStartTop : public Drawable {
public:
  SeparatorStartTop(const uint8_t display) : Drawable(start_top, display) {}
};

class SeparatorStartBottom : public Drawable {
public:
  SeparatorStartBottom(const uint8_t display) : Drawable(start_bottom, display) {}
};

class SeparatorEndTop : public Drawable {
public:
  SeparatorEndTop(const uint8_t display) : Drawable(end_top, display) {}
};

class SeparatorEndBottom : public Drawable {
public:
  SeparatorEndBottom(const uint8_t display) : Drawable(end_bottom, display) {}
};

class SeparatorEmpty : public Drawable {
public:
  SeparatorEmpty(const uint8_t display) : Drawable(empty, display) {}
};

constexpr Drawable Separator(bool start, bool end, bool top, bool bottom, uint8_t display) {
  if(start && end && top && bottom) {
    return SeparatorStartEndTopBottom(display);
  } else if(start && end && top) {
    return SeparatorStartEndTop(display);
  } else if(start && end && bottom) {
    return SeparatorStartEndBottom(display);
  } else if(start && top && bottom) {
    return SeparatorStartTopBottom(display);
  } else if(end && top && bottom) {
    return SeparatorEndTopBottom(display);
  } else if(start && end) {
    return SeparatorStartEnd(display);
  } else if(top && bottom) {
    return SeparatorTopBottom(display);
  } else if(start && top) {
    return SeparatorStartTop(display);
  } else if(start && bottom) {
    return SeparatorStartBottom(display);
  } else if(end && top) {
    return SeparatorEndTop(display);
  } else if(end && bottom) {
    return SeparatorEndBottom(display);
  } else {
    return SeparatorEmpty(display);
  }
}

struct Control {
  std::string title;
  std::string group;
  Control(std::string title, std::string group) : title(title), group(group) {}
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
  Cross(-1,-1,-1, 0), Bar(-1, 0), Cross(-1,-1, 0, 1), Bar(-1, 0), Cross(-1,-1, 1, 2), Bar(-1, 2), Cross(-1,-1, 2,-1),
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
  std::string title;
  static constexpr Drawable sp(std::span<Control, 9> cs, uint8_t display) {
    return std::visit(overloaded{
        [cs, display](const Cross cross) {
          std::string start_top = cross.start_top == -1 ? "" : cs[cross.start_top].group;
          std::string end_top = cross.end_top == -1 ? "" : cs[cross.end_top].group;
          std::string start_bottom = cross.start_bottom == -1 ? "" : cs[cross.start_bottom].group;
          std::string end_bottom = cross.end_bottom == -1 ? "" : cs[cross.end_bottom].group;
          return Separator(start_top != start_bottom, end_top != end_bottom, start_top != end_top, start_bottom != end_bottom, display);
        },
          [cs, display](const Bar bar) {
            std::string top = bar.top == -1 ? "" : cs[bar.top].group;
            std::string bottom = bar.bottom == -1 ? "" : cs[bar.bottom].group;
            return Separator(top != bottom, top != bottom, false, false, display);
          }
          }, ctrl_index[display]);
  }
public:
  std::array<Drawable, 40> drawables;
  Panel(std::string title, std::array<Control, 9> cs) : title(title), drawables({
      sp(cs,  0), sp(cs,  1), sp(cs,  2), sp(cs,  3), sp(cs,  4), sp(cs,  5), sp(cs,  6),
      sp(cs,  7),             sp(cs,  8),             sp(cs,  9),             sp(cs, 10),
      sp(cs, 11), sp(cs, 12), sp(cs, 13), sp(cs, 14), sp(cs, 15), sp(cs, 16), sp(cs, 17),
      sp(cs, 18),             sp(cs, 19),             sp(cs, 20),             sp(cs, 21),
      sp(cs, 22), sp(cs, 23), sp(cs, 24), sp(cs, 25), sp(cs, 26), sp(cs, 27), sp(cs, 28),
      sp(cs, 29),             sp(cs, 30),             sp(cs, 31),             sp(cs, 32),
      sp(cs, 33), sp(cs, 34), sp(cs, 35), sp(cs, 36), sp(cs, 37), sp(cs, 38), sp(cs, 39)
    }) {}
};


class LinePoint : public Drawable {
  FilledCircle circle;
  Line line;
  std::array<Item, 2> items;

public:
  constexpr LinePoint() : circle(Point(0,0), 0), line(Point(0,0), Point(0,0), 0), items({
    &circle,
    &line
    }), Drawable(items, 0) {}
  void update(uint32_t c) {
    circle.center.x = 32 + (c % 32) * 2;
    circle.center.y = 8 + c % 32;
    circle.radius = display % 4;
    line.start.x = display * 2;
    line.start.y = (c % 16) * 4;
    line.end.x = 100;
    line.end.y = c % 16;
    line.size = display % 4;
  }
};

const Drawable separator = Separator(true,true,true,true, 1);
LinePoint lp = LinePoint();
static std::array<const Drawable *, 2> list = {
  &separator,
  &lp
};

Panel panel = Panel("Ctrls", {
    Control("", "1"), Control("", ""), Control("", ""),
    Control("", ""),  Control("", ""), Control("", ""),
    Control("", ""),  Control("", ""), Control("", "")
  });
void fill_all(uint32_t c) {
  draw(panel.drawables);
}

int main() {
  stdio_init_all();
  printf("Start!\n");

  init();

  auto d0 = get(0);
  d0.set_pixel(0, 0, 1);
  d0.set_pixel(0, 1, 1);
  d0.set_pixel(0, 2, 1);
  d0.set_pixel(0, 3, 1);
  d0.set_pixel(1, 0, 1);
  d0.set_pixel(2, 0, 1);
  d0.set_pixel(3, 0, 1);

  d0.set_pixel(1, 1, 1);
  d0.set_pixel(2, 2, 1);
  d0.set_pixel(3, 3, 1);
  d0.set_pixel(4, 4, 1);
  d0.set_pixel(5, 5, 1);
  d0.set_pixel(6, 6, 1);
  d0.set_pixel(7, 7, 1);
  d0.set_pixel(8, 8, 1);

  auto d1 = get(1);
  d1.set_pixel(0, 0, 1);
  d1.set_pixel(0, 1, 1);
  d1.set_pixel(0, 2, 1);
  d1.set_pixel(0, 3, 1);
  d1.set_pixel(127, 63, 1);

  int64_t average_time = 0;
  int64_t average_render = 0;
  absolute_time_t start = get_absolute_time();
  uint32_t c = 1;
  while(true) {
    flip();
    absolute_time_t render_start = get_absolute_time();
    clear();
    fill_all(c);
    wait_ready();
    absolute_time_t new_start = get_absolute_time();

    if(average_render == 0) {
      average_render = absolute_time_diff_us(render_start, new_start);
    } else {
      average_render = (absolute_time_diff_us(render_start, new_start) + average_render) / 2;
    }
    if(average_time == 0) {
      average_time = absolute_time_diff_us(start, new_start);
    } else {
      average_time = (absolute_time_diff_us(start, new_start) + average_time) / 2;
    }
    start = new_start;
    if(c % 10 == 0)
      printf("%llu %llu\n",average_time, average_render);
    c++;
  }
}
