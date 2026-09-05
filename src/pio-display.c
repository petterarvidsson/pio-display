#include "pico/stdlib.h"
#include "stdio.h"
#include "string.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "spi.pio.h"
#include "fb_ops.h"

#define RESET 20
#define SCLK 16
#define MOSI 6
#define CS 17
#define DC 18
#define CP_PIO 19
#define CP_OR 15

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
#define DISPLAY_GROUPS 5
#define DISPLAY_ROW (32 * 4 * DISPLAYS)
#define DISPLAY_ROW_HEADER (8 + 4 * DISPLAYS)
#define DISPLAY_ROWS 8
#define DISPLAY_ROW_SIZE (DISPLAY_ROW + DISPLAY_ROW_HEADER)
#define FB_SIZE (DISPLAY_ROWS * DISPLAY_ROW_SIZE)
#define ALL_FB_SIZE (FB_SIZE * DISPLAY_GROUPS)
#define INIT_SIZE (4 + 4 * DISPLAYS)
#define ALL_INIT_SIZE (INIT_SIZE * DISPLAY_GROUPS)

static uint8_t all[ALL_FB_SIZE];

void init_display_setup(uint8_t *fb, bool shift_to_next) {
  memset(fb, 0x00, FB_SIZE);
  fb[0] = 0x00;
  fb[1] = 0x01;
  fb[2] = 0x00;
  fb[3] = shift_to_next ? 0x02 : 0x00;
  const uint8_t display_init[] = {
    0x8d, 0x14, 0xaf, 0xe3
  };
  split_bytes_into(display_init, 4, fb + 4, DISPLAYS);
}

void init_all_display_setup(uint8_t *all) {
  init_display_setup(all, false);
  for(size_t i = 1; i < DISPLAY_GROUPS; i++) {
    init_display_setup(all + i * INIT_SIZE, true);
  }
}

void init_fb_headers(uint8_t *fb, bool shift_to_next) {
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

void init_all_fb_headers(uint8_t *all) {
  init_fb_headers(all, false);
  for(size_t i = 1; i < DISPLAY_GROUPS; i++) {
    init_fb_headers(all + i * FB_SIZE, true);
  }
}

void clear_displays(uint8_t *all) {
  for(size_t i = 0; i < DISPLAY_GROUPS; i++) {
    uint8_t *fb = all + FB_SIZE * i;
    for(size_t row = 0; row < DISPLAY_ROWS; row++) {
      size_t off = row * DISPLAY_ROW_SIZE + DISPLAY_ROW_HEADER;
      memset(fb + off, 0x00, DISPLAY_ROW);
    }
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

void fill_all(uint8_t *all, uint32_t c) {
  for(int group = 0; group < DISPLAY_GROUPS; group++) {
    uint8_t *fb = all + group * FB_SIZE;
    for(int i = 0; i < DISPLAYS; i++) {
      for(int x = 0; x < 128; x++) {
        for(int y = 0; y < 64; y++) {
          pixel(fb, i, x, y, (i * x + y) % (c % 32));
        }
      }
    }
  }
}

void activate_first_display() {
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

int main() {
  stdio_init_all();
  fill_row_lut();
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

  uint channel = dma_init(pio, sm);

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
  //clear_displays(all);
  pixel(all, 0, 0, 0, 1);
  pixel(all, 0, 0, 1, 1);
  pixel(all, 0, 0, 2, 1);
  pixel(all, 0, 0, 3, 1);
  pixel(all, 0, 1, 0, 1);
  pixel(all, 0, 2, 0, 1);
  pixel(all, 0, 3, 0, 1);

  pixel(all, 0, 1, 1, 1);
  pixel(all, 0, 2, 2, 1);
  pixel(all, 0, 3, 3, 1);
  pixel(all, 0, 4, 4, 1);
  pixel(all, 0, 5, 5, 1);
  pixel(all, 0, 6, 6, 1);
  pixel(all, 0, 7, 7, 1);
  pixel(all, 0, 8, 8, 1);

  pixel(all, 1, 0, 0, 1);
  pixel(all, 1, 0, 1, 1);
  pixel(all, 1, 0, 2, 1);
  pixel(all, 1, 0, 3, 1);
  pixel(all, 1, 127, 63, 1);

  //  clear_displays(fb);
  int64_t average_time = 0;
  int64_t average_render = 0;
  absolute_time_t start = get_absolute_time();
  uint32_t c = 1;

  clear_displays(all);
  fill_all(all, c);

  while(true) {
    activate_first_display();
    dma_channel_transfer_from_buffer_now(channel, all, ALL_FB_SIZE / 4);
    dma_channel_wait_for_finish_blocking(channel);
    absolute_time_t render_start = get_absolute_time();
    clear_displays(all);
    fill_all(all, c);
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
