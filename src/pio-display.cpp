#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "stdio.h"
#include "string.h"
#include "displays.hpp"
#include "protocol.hpp"
#include "control.hpp"
#include "action.hpp"
#include "panel.hpp"
#include "overloaded.hpp"
#include "input.hpp"
#include <vector>
#include <string>
#include <iostream>
using namespace sdhi;

static const int sck = 2;
static const int miso = 4;
static const int cs = 5;
static const int busy = 3;

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


static void init_spi() {
  // Enable SPI0 at 4 MHz
  spi_init (spi_default, 4 * 1000000);
  spi_set_slave (spi_default, true);
  // Assign SPI functions to the default SPI pins
  gpio_set_function (miso, GPIO_FUNC_SPI);
  gpio_set_function (sck, GPIO_FUNC_SPI);
  gpio_set_function (cs, GPIO_FUNC_SPI);

  gpio_init(busy);
  gpio_set_dir(busy, GPIO_OUT);
  gpio_put(busy, 0);
}

static const size_t spi_buffer_size = 64 * 1024;
static uint8_t spi_buffer[spi_buffer_size];

int main() {
  stdio_init_all();
  init_spi();
  std::cout<<"Slave ready!"<<std::endl;
  /*
  Panel panel = Panel("Ctrls", controls);
  displays::init();
  input::init();

  multicore_launch_core1(graphics);
  CommandQueue cq;
  int32_t c = 0;

  std::vector<displays::DisplayItem> display_items;
  while(true) {
    if(displays::is_ready()) {
      panel.display_items(display_items);
      displays::flip(display_items);
      display_items.clear();
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
    panel.update(change, button_change);
    for(auto msg: cq.log) {
      std::cout<<msg<<std::endl;
    }
    cq.log.clear();

    c++;
  }
  */

  while(true) {
    gpio_put(busy, !displays::is_ready());
    if(displays::is_ready() && spi_is_readable(spi_default)) {
      gpio_put(busy, 1);
      uint32_t size;
      spi_read_blocking(spi_default, 0x00, (uint8_t*)&size, sizeof(uint32_t));
      hard_assert(size <= spi_buffer_size);
      spi_read_blocking(spi_default, 0x00, spi_buffer, size);
      size_t pos = 0;
      std::vector<displays::Item> display_items;
      protocol::decode_items(spi_buffer, display_items);
      gpio_put(busy, 0);
    }
  }

}
