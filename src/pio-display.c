#include "pico/stdlib.h"
#include "stdio.h"
#include "string.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "spi.pio.h"
#include "fb_ops.h"

#define RESET 19
#define SCLK 16
#define MOSI 6
#define CS 17
#define DC 18

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

#define DISPLAYS 8
#define FB_HEADER (4 + 4 * DISPLAYS)
#define DISPLAY_ROW (32 * 4 * DISPLAYS)
#define DISPLAY_ROW_HEADER (8 + 4 * DISPLAYS)
#define DISPLAY_ROWS 8
#define DISPLAY_ROW_SIZE (DISPLAY_ROW + DISPLAY_ROW_HEADER)
#define FB_SIZE (FB_HEADER + DISPLAY_ROWS * DISPLAY_ROW_SIZE)
static uint8_t fb1[FB_SIZE];

void initialize_fb_headers(uint8_t *fb) {
  memset(fb, 0x00, FB_SIZE);
  fb[0] = 0x00;
  fb[1] = 0x01;
  fb[2] = 0x00;
  fb[3] = 0x00;
  const uint8_t display_init[] = {
    0x8d, 0x14, 0xaf, 0xe3
  };
  split_bytes_into(display_init, 4, fb + 4, DISPLAYS);

  for(uint8_t row = 0; row < DISPLAY_ROWS; row++) {
    size_t off = FB_HEADER + row * DISPLAY_ROW_SIZE;
    fb[off] = 0x00;
    fb[off + 1] = 0x01;
    fb[off + 2] = 0x00;
    fb[off + 3] = 0x00;
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

void clear_displays(uint8_t *fb) {
  for(uint8_t row = 0; row < DISPLAY_ROWS; row++) {
    size_t off = FB_HEADER + row * DISPLAY_ROW_SIZE + DISPLAY_ROW_HEADER;
    memset(fb + off, 0x00, DISPLAY_ROW);
  }
}

// Byte index of y lut
static size_t row_lut[64];

static void fill_row_lut() {
  for(int y = 0; y < 64; y++) {
    uint8_t y_in_row = y % 8;
    size_t row = y / 8;
    row_lut[y] = DISPLAY_ROW_SIZE * row + DISPLAY_ROW_HEADER + (DISPLAYS - 1) - y_in_row;
  }
}

// works only for 8 displays
static void pixel(uint8_t *fb, const uint8_t display, const uint8_t x, const uint8_t y, const bool on) {
  uint32_t i = row_lut[y] + x * DISPLAYS;
  // Update bit
  uint8_t seg = fb[i];
  fb[i] ^= (-on ^ seg) & (1 << display);
}

void fill_all(uint8_t *fb) {
  for(int i = 0; i < DISPLAYS; i++) {
    for(int x = 0; x < 128; x++) {
      for(int y = 0; y < 64; y++) {
        pixel(fb, i, x, y, 1);
      }
    }
  }
}

int main() {
  uint8_t *fb = fb1 + FB_HEADER;
  stdio_init_all();
  fill_row_lut();
  initialize_fb_headers(fb1);
  clear_displays(fb1);
  pixel(fb, 0, 0, 0, 1);
  pixel(fb, 0, 0, 1, 1);
  pixel(fb, 0, 0, 2, 1);
  pixel(fb, 0, 0, 3, 1);
  pixel(fb, 0, 1, 0, 1);
  pixel(fb, 0, 2, 0, 1);
  pixel(fb, 0, 3, 0, 1);

  pixel(fb, 0, 1, 1, 1);
  pixel(fb, 0, 2, 2, 1);
  pixel(fb, 0, 3, 3, 1);
  pixel(fb, 0, 4, 4, 1);
  pixel(fb, 0, 5, 5, 1);
  pixel(fb, 0, 6, 6, 1);
  pixel(fb, 0, 7, 7, 1);
  pixel(fb, 0, 8, 8, 1);

  pixel(fb, 1, 0, 0, 1);
  pixel(fb, 1, 0, 1, 1);
  pixel(fb, 1, 0, 2, 1);
  pixel(fb, 1, 0, 3, 1);
  pixel(fb, 1, 127, 63, 1);

  printf("----------\n");
  //compare_byte_bits();
  gpio_init(CS);
  gpio_set_dir(CS, GPIO_OUT);
  gpio_put(CS, 1);

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

  spi_program_init(pio, sm, offset, MOSI, DISPLAYS, DC, SCLK);

  uint channel = dma_init(pio, sm);

  gpio_put(CS, 0);
  //  clear_displays(fb1);
  int64_t average_time = 0;
  int64_t average_render = 0;
  absolute_time_t start = get_absolute_time();
  uint32_t c = 0;
  while(true) {
    dma_channel_transfer_from_buffer_now(channel, fb1, FB_SIZE / 4);
    dma_channel_wait_for_finish_blocking(channel);
    absolute_time_t render_start = get_absolute_time();
    clear_displays(fb1);
    fill_all(fb);
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
    if(c % 100 == 0)
      printf("%llu %llu \n",average_time, average_render);
    c++;
  }
}
