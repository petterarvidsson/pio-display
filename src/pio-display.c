#include "pico/stdlib.h"
#include "stdio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "spi.pio.h"

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
#define BIT(D, N) (((uint8_t)(D) >> (N)) & 0x1)
#define BIT2(D, N) (BIT(D, N) << ((N) * 2)) | (BIT(D, N) << ((N) * 2 + 1))
#define BS(B) (BIT2(((B >> 4) & 0xF), 0) | BIT2(((B >> 4) & 0xF), 1) | BIT2(((B >> 4) & 0xF), 2) | BIT2(((B >> 4) & 0xF), 3)), (BIT2(B, 0) | BIT2(B, 1) | BIT2(B, 2) | BIT2(B, 3))

#define PATTERN BS(0x01), BS(0x00), BS(0x00), BS(0x00)
#define WORDS 32

static uint8_t data[] = {
  0x00, 0x01, 0x00, 0x00, BS(0x8d), BS(0x14), BS(0xaf), BS(0xe3),
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB0), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB1), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB2), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB3), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB4), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB5), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB6), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  0x00, 0x01, 0x00, 0x00, BS(0xe3), BS(0xB7), BS(0x02), BS(0x10),
  0x00, 0x20, 0x00, 0x01,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN
};

#define DISPLAYS 2
#define DISPLAY_ROW (32 * 4 * DISPLAYS)
#define DISPLAY_ROW_HEADER (8 + 4 * DISPLAYS)
#define DISPLAY_ROWS 8
#define DISPLAY_ROW_SIZE (DISPLAY_ROW + DISPLAY_ROW_HEADER)

void pixel(const uint8_t display, const uint8_t x, const uint8_t y, const bool on) {
  uint8_t *fb = data + 3 * 4;

  // Display riow to update
  uint32_t row = y / 8;
  // y position in row (0 - 7)
  uint32_t y_in_row = y % 8;

  // First byte of DISPLAYS bytes where the bit is found
  uint32_t i = DISPLAY_ROW_SIZE * row + DISPLAY_ROW_HEADER + x * DISPLAYS;
  // Index of the bit within DISPLAYS bytes (LSB)
  uint32_t bit = y_in_row * DISPLAYS + display;
  // Byte within DISPLAYS bytes that contain the bit
  uint32_t i_off = (DISPLAYS - 1) - bit / 8;
  // Index of bit within the byte
  uint32_t bit_i = bit % 8;

  // Update bit
  uint8_t seg = fb[i + i_off];
  fb[i + i_off] ^= (-on ^ seg) & (1 << bit_i);
}


int main() {
    stdio_init_all();
    pixel(0, 0, 0, 1);

    pixel(0, 0, 1, 1);
    pixel(0, 0, 2, 1);
    pixel(0, 0, 3, 1);
    pixel(0, 1, 0, 1);
    pixel(0, 2, 0, 1);
    pixel(0, 3, 0, 1);

    pixel(0, 1, 1, 1);
    pixel(0, 2, 2, 1);
    pixel(0, 3, 3, 1);
    pixel(0, 4, 4, 1);
    pixel(0, 5, 5, 1);
    pixel(0, 6, 6, 1);
    pixel(0, 7, 7, 1);
    pixel(0, 8, 8, 1);

    pixel(1, 0, 0, 1);
    pixel(1, 0, 1, 1);
    pixel(1, 0, 2, 1);
    pixel(1, 0, 3, 1);
    pixel(1, 127, 63, 1);

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

    spi_program_init(pio, sm, offset, MOSI, DC, SCLK);

    uint channel = dma_init(pio, sm);

    gpio_put(CS, 0);

    absolute_time_t start = get_absolute_time();
    dma_channel_transfer_from_buffer_now(channel, data, sizeof(data) / 4);
    dma_channel_wait_for_finish_blocking(channel);
    absolute_time_t end = get_absolute_time();
    printf("DONE1 %llu %llu %llu us\n",to_us_since_boot(start), to_us_since_boot(end), absolute_time_diff_us(start, end));
}
