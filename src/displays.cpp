#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include <cstring>
#include <array>
#include <numbers>
#include <cmath>
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
  /* Include all fonts */
#include <fonts.inc>

  struct Box {
    uint8_t width;
    uint8_t height;
  };

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

  inline static void safe_pixel(uint8_t * const fb, const uint8_t display, const int16_t x, const int16_t y, const bool on) {
    const uint8_t x_safe = x < 0 ? 0 : x > 127 ? 127 : x;
    const uint8_t y_safe = y < 0 ? 0 : y > 63 ? 63 : y;
    pixel(fb, display, x_safe, y_safe, on);
  }


  inline static void sized_pixel(uint8_t * const fb, const uint8_t display, const uint8_t x, const uint8_t y, const uint8_t size) {
    const uint8_t x_min = x - size < 0 ? 0 : x - size;
    const uint8_t x_max = x + size > 127 ? 127 : x + size;
    const uint8_t y_min = y - size < 0 ? 0 : y - size;
    const uint8_t y_max = y + size > 63 ? 63 : y + size;

    for(uint8_t xi = x_min; xi <= x_max ; xi++) {
      for(uint8_t yi = y_min; yi <= y_max; yi++) {
        pixel(fb, display, xi, yi, true);
      }
    }
  }

  inline static void fill_x(uint8_t * const fb, const uint8_t display, const int16_t x, const int16_t y, const uint8_t length) {
    for(uint8_t i = 0; i < length; i++) {
      const int16_t xi = x + i;
      if(xi > 0 && xi <= 127) {
        pixel(fb, display, xi, y, true);
      }
    }
  }

  void Display::line(uint8_t x0, uint8_t y0, const uint8_t x1, const uint8_t y1, const uint8_t size) const {
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t dx =  abs(x1 - x0);
    int16_t dy = -abs(y1 - y0);
    int16_t err = dx + dy;
    int16_t e2;

    while(x0 != x1 || y0 != y1) {
      sized_pixel(fb, index, x0, y0, size);
      e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
    sized_pixel(fb, index, x0, y0, size);
  }

  void Display::circle(const uint8_t x0, const uint8_t y0, const uint8_t radius) const {
    int f = 1 - radius;
    int ddf_x = 1;
    int ddf_y = -2 * radius;
    int x = 0;
    int y = radius;

    safe_pixel(fb, index, x0, y0 + radius, true);
    safe_pixel(fb, index, x0, y0 - radius, true);
    safe_pixel(fb, index, x0 + radius, y0, true);
    safe_pixel(fb, index, x0 - radius, y0, true);
    while (x < y) {
      if(f >= 0) {
        y--;
        ddf_y += 2;
        f += ddf_y;
      }
      x++;
      ddf_x += 2;
      f += ddf_x;
      safe_pixel(fb, index, x0 + x, y0 + y, true);
      safe_pixel(fb, index, x0 - x, y0 + y, true);
      safe_pixel(fb, index, x0 + x, y0 - y, true);
      safe_pixel(fb, index, x0 - x, y0 - y, true);
      safe_pixel(fb, index, x0 + y, y0 + x, true);
      safe_pixel(fb, index, x0 - y, y0 + x, true);
      safe_pixel(fb, index, x0 + y, y0 - x, true);
      safe_pixel(fb, index, x0 - y, y0 - x, true);
    }
  }

  void Display::sine(const uint8_t from, const uint8_t to, const uint8_t x, const uint8_t y, const uint8_t length, const uint8_t amplitude) const {
    const float ffrom = (float)from * (std::numbers::pi / 8);
    const float interval = (float)to * (std::numbers::pi / 8) - ffrom;
    uint8_t old_yi = y + (int8_t)(std::sin(ffrom) * amplitude);
    for(uint8_t i = 1; i < length; i++) {
      uint8_t xi = x + i;
      int16_t yi = y + (int16_t)(std::sin(ffrom + interval * ((float)i / (float)length)) * amplitude);
      line(xi - 1, old_yi, xi, yi, 0);
      old_yi = yi;
    }
  }

  void Display::filled_circle(const uint8_t x0, const uint8_t y0, const uint8_t radius) const {
    int f = 1 - radius;
    int ddf_x = 1;
    int ddf_y = -2 * radius;
    int x = 0;
    int y = radius;

    safe_pixel(fb, index, x0, y0 + radius, true);
    safe_pixel(fb, index, x0, y0 - radius, true);
    safe_pixel(fb, index, x0 + radius, y0, true);
    safe_pixel(fb, index, x0 - radius, y0, true);
    fill_x(fb, index, x0 - radius, y0, radius * 2);

    while (x < y) {
      if (f >= 0) {
        y--;
        ddf_y += 2;
        f += ddf_y;
      }
      x++;
      ddf_x += 2;
      f += ddf_x;
      safe_pixel(fb, index, x0 + x, y0 + y, true);
      safe_pixel(fb, index, x0 - x, y0 + y, true);
      fill_x(fb, index, x0 - x, y0 + y, x * 2);
      safe_pixel(fb, index, x0 + x, y0 - y, true);
      safe_pixel(fb, index, x0 - x, y0 - y, true);
      fill_x(fb, index, x0 - x, y0 - y, x * 2);
      safe_pixel(fb, index, x0 + y, y0 + x, true);
      safe_pixel(fb, index, x0 - y, y0 + x, true);
      fill_x(fb, index, x0 - y, y0 + x, y * 2);
      safe_pixel(fb, index, x0 + y, y0 - x, true);
      safe_pixel(fb, index, x0 - y, y0 - x, true);
      fill_x(fb, index, x0 - y, y0 - x, y * 2);
    }
  }

  void Display::rectangle(const uint8_t startx, const uint8_t starty,
                          const uint8_t endx, const uint8_t endy) const {
    for(uint8_t x = startx; x <= endx; x++) {
      for(uint8_t y = starty; y <= endy; y++) {
        pixel(fb, index, x, y, true);
      }
    }
  }

  static uint8_t font_bytes[] = {1, 1, 2, 2};
  static uint8_t font_width[] = {7, 8, 16, 16};
  static uint8_t font_height[] = {13, 18, 28, 32};
  static uint8_t font_offset[] = {13, 18, 28*2, 32*2};
  static uint8_t *fonts[] = {font_13, font_18, font_28, NULL};

  void Display::printc(const uint8_t startx, const uint8_t starty, const FontSize font_size, const bool on, const char c) const {
    uint32_t index = (uint32_t)c * font_offset[font_size];
    for(uint8_t i = 0; i < font_height[font_size]; i++) {
      uint8_t y = starty + font_height[font_size] - i;
      for(uint8_t b = 0; b < font_bytes[font_size]; b++) {
        uint8_t segment = fonts[font_size][index + (i * font_bytes[font_size]) + b];
        for(uint8_t j = 0; j < 8; j++) {
          uint8_t p = (segment >> j) & 0x01;
          if(p) {
            uint8_t x = startx + (8 - j) + (8 * b);
            pixel(fb, index, x, y, on);
          }
        }
      }
    }
  }

  void Display::print(const uint8_t startx, const uint8_t starty, const FontSize font_size, const bool on, const char * const str) const {
    const char * c = str;
    uint8_t x = startx;
    while (*c) {
      const char chr = *c++;
      if(chr != ' ') {
        printc(x, starty, font_size, on, chr);
        x += font_bytes[font_size]*8;
      } else {
        x += 8;
      }
    }
  }

  static Box text_box(const FontSize font_size, const char * const str) {
    const char * c = str;
    uint8_t len = 0;
    while (*c) {
      const char chr = *c++;
      if(chr != ' ') {
        len += font_bytes[font_size];
      } else {
        len += 1;
      }
    }
    const uint8_t width = 8 * len;

    const Box box = {width, font_height[font_size]};
    return box;
  }

  static uint8_t center_box_x(const Box box) {
    return (128 - box.width) / 2;
  }

  static uint8_t center_box_y(const Box box) {
    return (64 - box.height) / 2;
  }

  void Display::print_center(uint8_t * const fb, const uint8_t y, const FontSize font_size, const bool on, const char * const str) const {
    const Box box = text_box(font_size, str);
    if(box.width < 128) {
      uint8_t offset = center_box_x(box);
      print(offset, y, font_size, on, str);
    }
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
