#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "displays.hpp"
#include "protocol.hpp"
#include <vector>
#include <iostream>

static const int sck = 2;
static const int miso = 4;
static const int cs = 5;
static const int busy = 3;

void graphics() {
  while (1) {
    displays::render();
  }
}

static void init_spi() {
  // Enable SPI0 at 1 MHz
  spi_init (spi_default, 1000000);
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
  displays::init();
  multicore_launch_core1(graphics);

  while(true) {
    gpio_put(busy, !displays::is_ready());
    if(displays::is_ready() && spi_is_readable(spi_default)) {
      gpio_put(busy, 1);
      uint32_t size;
      spi_read_blocking(spi_default, 0x00, (uint8_t*)&size, sizeof(uint32_t));
      hard_assert(size <= spi_buffer_size);
      spi_read_blocking(spi_default, 0x00, spi_buffer, size);
      std::vector<protocol::Item> display_items;
      protocol::decode_items(spi_buffer, size, display_items);
      displays::flip(display_items);
    }
  }

}
