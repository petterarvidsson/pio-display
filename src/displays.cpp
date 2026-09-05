#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include <cstring>
#include <array>
#include "spi.pio.h"
#include "internal.hpp"
#include "displays.hpp"
#include "fb_ops.h"

#define RESET 20
#define SCLK 16
#define MOSI 6
#define CS 17
#define DC 18
#define CP_PIO 19
#define CP_OR 15

#define DISPLAY_GROUPS 5
#define FB_SIZE (DISPLAY_ROWS * DISPLAY_ROW_SIZE)
#define ALL_FB_SIZE (FB_SIZE * DISPLAY_GROUPS)
#define INIT_SIZE (4 + 4 * DISPLAYS)
#define ALL_INIT_SIZE (INIT_SIZE * DISPLAY_GROUPS)

namespace displays {

  static uint dma_init(PIO pio, uint sm) {
    int channel = dma_claim_unused_channel(true);

    dma_channel_config channel_config = dma_channel_get_default_config(channel);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
    channel_config_set_bswap(&channel_config, true);
    dma_channel_configure(channel,
                          &channel_config,
                          &pio->txf[sm],
                          NULL,
                          0,
                          false);
    return channel;
  }

  static uint8_t all[ALL_FB_SIZE];
  static uint channel;

  static const std::array row_lut { []<auto...Y>(std::index_sequence<Y...>){
      return std::array<size_t, 64>{(DISPLAY_ROW_SIZE * (Y / 8) + DISPLAY_ROW_HEADER + (DISPLAYS - 1) - (Y % 8))...};
    }(std::make_index_sequence<64>{})
  };

  static void init_display_setup(uint8_t *fb, bool shift_to_next) {
    std::memset(fb, 0x00, FB_SIZE);
    fb[0] = 0x00;
    fb[1] = 0x01;
    fb[2] = 0x00;
    fb[3] = shift_to_next ? 0x02 : 0x00;
    const uint8_t display_init[] = {
      0x8d, 0x14, 0xaf, 0xe3
    };
    split_bytes_into(display_init, 4, fb + 4, DISPLAYS);
  }

  static void init_all_display_setup(uint8_t *all) {
    init_display_setup(all, false);
    for(size_t i = 1; i < DISPLAY_GROUPS; i++) {
      init_display_setup(all + i * INIT_SIZE, true);
    }
  }

  static void init_fb_headers(uint8_t *fb, bool shift_to_next) {
    memset(fb, 0x00, FB_SIZE);

    for(uint8_t row = 0; row < DISPLAY_ROWS; row++) {
      size_t off = row * DISPLAY_ROW_SIZE;
      fb[off] = 0x00;
      fb[off + 1] = 0x01;
      fb[off + 2] = 0x00;
      fb[off + 3] = shift_to_next && row == 0 ? 0x02 : 0x00;
      uint8_t row_header[] = {
        0xe3, 0xB0, 0x02, 0x10
      };
      row_header[1] = 0xB0 + row;
      split_bytes_into(row_header, 4, fb + off + 4, DISPLAYS);
      size_t off2 = off + 4 + 4 * DISPLAYS;
      fb[off2] = 0x00;
      fb[off2 + 1] = 0x20;
      fb[off2 + 2] = 0x00;
      fb[off2 + 3] = 0x01;
    }
  }

  static void init_all_fb_headers(uint8_t *all) {
    init_fb_headers(all, false);
    for(size_t i = 1; i < DISPLAY_GROUPS; i++) {
      init_fb_headers(all + i * FB_SIZE, true);
    }
  }

  static void activate_first_display() {
    // Activate first display group
    gpio_put(CS, 0);
    sleep_us(1);
    gpio_put(CP_OR, 1);
    sleep_us(1);
    gpio_put(CP_OR, 0);
    sleep_us(1);
    // Set CS back to one for coming groups
    gpio_put(CS, 1);
  }


  void init() {
    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1);

    gpio_init(CP_OR);
    gpio_set_dir(CP_OR, GPIO_OUT);
    gpio_put(CP_OR, 0);

    gpio_init(CP_PIO);
    gpio_set_dir(CP_PIO, GPIO_OUT);
    gpio_put(CP_PIO, 0);

    gpio_init(RESET);
    gpio_set_dir(RESET, GPIO_OUT);
    gpio_put(RESET, 1);
    sleep_ms(1);
    gpio_put(RESET, 0);
    sleep_ms(1);
    gpio_put(RESET, 1);

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &spi_program);
    uint sm = pio_claim_unused_sm(pio, true);

    spi_program_init(pio, sm, offset, MOSI, DC, SCLK);

    channel = dma_init(pio, sm);

    // Disable all display groups
    for(int i = 0; i < 5; i++) {
      gpio_put(CP_OR, 1);
      sleep_us(1);
      gpio_put(CP_OR, 0);
      sleep_us(1);
    }

    init_all_display_setup(all);
    activate_first_display();
    dma_channel_transfer_from_buffer_now(channel, all, ALL_INIT_SIZE / 4);
    dma_channel_wait_for_finish_blocking(channel);
    init_all_fb_headers(all);
    clear();
  }

  void clear() {
    for(size_t i = 0; i < DISPLAY_GROUPS; i++) {
      uint8_t *fb = all + FB_SIZE * i;
      for(size_t row = 0; row < DISPLAY_ROWS; row++) {
        size_t off = row * DISPLAY_ROW_SIZE + DISPLAY_ROW_HEADER;
        memset(fb + off, 0x00, DISPLAY_ROW);
      }
    }
  }

  void update() {
    activate_first_display();
    dma_channel_transfer_from_buffer_now(channel, all, ALL_FB_SIZE / 4);
    dma_channel_wait_for_finish_blocking(channel);
  }

  // works only for 8 displays
  inline static void pixel(uint8_t *fb, const uint8_t display, const uint8_t x, const uint8_t y, const bool on) {
    uint32_t i = row_lut[y] + x * DISPLAYS;
    // Update bit
    uint8_t seg = fb[i];
    fb[i] ^= (-on ^ seg) & (1 << display);
  }

  void Display::set_pixel(const uint8_t x, const uint8_t y, const bool on) const {
    pixel(fb, index, x, y, on);
  }

  static const std::array displays { []<auto...I>(std::index_sequence<I...>){
      return std::array<Display, DISPLAYS * DISPLAY_GROUPS>{Display(all + (I / 8) * FB_SIZE, I % 8)...};
    }(std::make_index_sequence<DISPLAYS * DISPLAY_GROUPS>{})
  };

  Display get(uint index) {
    return displays[index];
  }
}
