#include "pico/stdlib.h"
#include "stdio.h"
#include "string.h"
#include "displays.hpp"
#include <vector>

void fill_all(uint32_t c) {
  for(int i = 0; i < 40; i++) {
    auto d = displays::get(i);
    std::vector<displays::Item> list;
    list.push_back(displays::FilledCircle(displays::Point(32 + (c % 32) * 2, 8 + c % 32), i % 4));
    list.push_back(displays::Line(displays::Point(i * 2, (c % 16) * 4), displays::Point(100, c % 16), i % 4));
    d.draw(list);
  }
}

int main() {
  stdio_init_all();
  printf("Start!\n");

  displays::init();

  auto d0 = displays::get(0);
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

  auto d1 = displays::get(1);
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
    displays::flip();
    absolute_time_t render_start = get_absolute_time();
    displays::clear();
    fill_all(c);
    displays::wait_ready();
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
