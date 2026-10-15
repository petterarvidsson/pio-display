#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "stdio.h"
#include "string.h"
#include "displays.hpp"
#include "control.hpp"
#include "action.hpp"
#include "panel.hpp"
#include "overloaded.hpp"
#include <vector>
#include <string>
#include <iostream>

using namespace displays;
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
    render();
  }
}

int main() {
  stdio_init_all();

  Panel panel("Ctrls", controls);
  init();
  multicore_launch_core1(graphics);
  CommandQueue cq;
  int32_t c = 0;
  while(true) {
    c1.update(1);
    if(is_ready()) {
      flip(panel.display_list());
    }
    if(c % 100000 == 0) {
      stats();
    }
    for(auto msg: cq.log) {
      std::cout<<msg<<std::endl;
    }
    cq.log.clear();
    c++;
  }
}
