#include "input.hpp"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

namespace input {
  static const int scl = 27;
  static const int sda = 26;
  static const int controllers = 9;

  static const int addr0 = 0x20;
  static const int addr1 = 0x21;
  static const uint8_t reg0 = 0;

  // Pin connections for pin A, B and D
  // A   B   D
  static const uint8_t controller_connections[controllers][3] = {
    {3, 4, 5}, // E1 0xFFFFFFDF
    {10,  11,   12}, // E2 0xFFFFEFFF
    {13,  14,   15}, // E3 0xFFFF7FFF
    {0,  1,  2}, // E4 0xFFFFFFFB
    {24, 25, 26}, // E5 0xFBFFFFFF
    {27, 28, 29}, // E6 0xDFFFFFFF
    {21, 22, 23}, // E7 0xFF7FFFFF
    {18, 19, 20}, // E8 0xFFEFFFFF
    {30, 31, 17}  // E9 0x3FFDFFFF
  };

  static uint8_t old_b[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1
  };

  static uint32_t rxdata;

  static int32_t change[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  static uint8_t old_button[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  static ControllerButtonState button[] = {
    no_change, no_change, no_change,
    no_change, no_change, no_change,
    no_change, no_change, no_change
  };

  void init() {
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(26, GPIO_FUNC_I2C);
    gpio_set_function(27, GPIO_FUNC_I2C);
    gpio_pull_up(26);
    gpio_pull_up(27);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(sda, scl, GPIO_FUNC_I2C));

    // Initialize old_b
    i2c_write_blocking(i2c1, addr0, &reg0, 1, true);
    i2c_read_blocking(i2c1, addr0, (uint8_t*)&rxdata, 2, false);
    i2c_write_blocking(i2c1, addr1, &reg0, 1, true);
    i2c_read_blocking(i2c1, addr1, ((uint8_t*)&rxdata + 2), 2, false);

    for(int i = 0; i < controllers; i++) {
      old_b[i] = (rxdata >> controller_connections[i][1]) & 0x1;
    }
  }

  bool read_change(int32_t * const change_update, ControllerButtonState * const button_update) {
    i2c_write_blocking(i2c1, addr0, &reg0, 1, true);
    i2c_read_blocking(i2c1, addr0, (uint8_t*)&rxdata, 2, false);
    i2c_write_blocking(i2c1, addr1, &reg0, 1, true);
    i2c_read_blocking(i2c1, addr1, ((uint8_t*)&rxdata + 2), 2, false);

    for(int i = 0; i < controllers; i++) {
      uint8_t a = (rxdata >> controller_connections[i][0]) & 0x1;
      uint8_t b = (rxdata >> controller_connections[i][1]) & 0x1;
      uint8_t c = (rxdata >> controller_connections[i][2]) & 0x1;
      if(b != old_b[i] && a == 0) {
        if(b) {
          change[i]--;
        } else {
          change[i]++;
        }
      }
      old_b[i] = b;
      if(old_button[i] != c) {
        if(!c) {
          button[i] = pressed;
        } else {
          button[i] = released;
        }
      }
      old_button[i] = c;
    }
    bool changed = false;
    for(int i = 0; i < controllers; i++) {
      change_update[i] = change[i];
      if(change[i] != 0) {
        change[i] = 0;
        changed = true;
      }
      button_update[i] = button[i];
      if(button[i] != no_change) {
        button[i] = no_change;
        changed = true;
      }
    }
    return change;
  }
}
