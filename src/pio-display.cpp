#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "stdio.h"
#include "string.h"
#include "displays.hpp"
#include "control.hpp"
#include "action.hpp"
#include "panel.hpp"
#include "overloaded.hpp"
#include "input.hpp"
#include <vector>
#include <string>
#include <iostream>
using namespace sdhi;

IntControl c1("FEEDBACK[1]0", "1", 0, 127);
IntControl c2("FEEDBACK[1]1", "1", -16, 16, -5);
IntControl c3("FEEDBACK[1]2", "1", 0, 4, 2, 2);
IntControl c4("FEEDBACK[2]3", "2", 0, 10);

std::array<std::optional<std::reference_wrapper<Controllable>>, 9> controls = {
  c1, c2, c3,
  c4, std::nullopt, std::nullopt,
  std::nullopt, std::nullopt, std::nullopt
};

void graphics() {
  while (1) {
    displays::render();
  }
}
int32_t change[9] = {0};
input::ControllerButtonState button_change[9] = {input::no_change};

static int32_t average_read_time = -1;
int main() {
  stdio_init_all();

  Panel panel = Panel("Ctrls", controls);
  displays::init();
  input::init();

  multicore_launch_core1(graphics);
  CommandQueue cq;
  int32_t c = 0;

  sleep_ms(10);

  while(true) {
    c1.update(1);
    if(displays::is_ready()) {
      displays::flip(panel.display_list());
    }
    absolute_time_t start = get_absolute_time();
    input::read_change(change, button_change);
    absolute_time_t end = get_absolute_time();
    int32_t time = absolute_time_diff_us(start, end);
    if(average_read_time == -1) {
        average_read_time = time;
      } else {
        average_read_time = (average_read_time + time) / 2;
      }
    if(c % 1000 == 0) {
      displays::stats();
      std::cout<<"Read: "<<average_read_time<<"us"<<std::endl;
    }

    for(int i=0; i < 9; i++) {
      if(change[i] != 0)
        std::cout<<i<<": "<<change[i]<<std::endl;
    }


    for(auto msg: cq.log) {
      std::cout<<msg<<std::endl;
    }
    cq.log.clear();

    c++;
  }
}
