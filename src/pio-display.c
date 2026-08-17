#include "pico/stdlib.h"
#include "stdio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "spi.pio.h"

#define RESET 21
#define SCLK 18
#define MOSI 19
#define CS 17
#define DC 22

static uint dma_init(PIO pio, uint sm) {
  int channel = dma_claim_unused_channel(true);

  dma_channel_config channel_config = dma_channel_get_default_config(channel);
  channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
  dma_channel_configure(channel,
                        &channel_config,
                        &pio->txf[sm],
                        NULL,
                        0,
                        false);
  return channel;
}
#define BIT(D, N) (((D) >> (N)) & 0x1)
#define BIT2(D, N) (BIT(D, N) << ((N) * 2)) | (BIT(D, N) << ((N) * 2 + 1))
#define BYTE2(B) (BIT2(B, 0) | BIT2(B, 1) | BIT2(B, 2) | BIT2(B, 3) | BIT2(B, 4) | BIT2(B, 5) | BIT2(B, 6) | BIT2(B, 7))
#define INT16_SPREAD(I) (((uint32_t)BYTE2(((I) >> 8)) << 16) | BYTE2(I))
#define UINT32_SPREAD(U) INT16_SPREAD((U) >> 16),  INT16_SPREAD(U)

#define PATTERN UINT32_SPREAD(0x0000101)
#define WORDS 32

static uint32_t data[] = {
  1 << 16, UINT32_SPREAD(0x8d14afe3),
  1 << 16, UINT32_SPREAD(0xe3B00210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  1 << 16, UINT32_SPREAD(0xe3B10210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  1 << 16, UINT32_SPREAD(0xe3B20210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  1 << 16, UINT32_SPREAD(0xe3B30210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  1 << 16, UINT32_SPREAD(0xe3B40210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  1 << 16, UINT32_SPREAD(0xe3B50210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  1 << 16, UINT32_SPREAD(0xe3B60210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  1 << 16, UINT32_SPREAD(0xe3B70210),
  (WORDS << 16) | 1,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN,
  PATTERN, PATTERN, PATTERN, PATTERN ,PATTERN, PATTERN, PATTERN, PATTERN
};

int main() {
    stdio_init_all();

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
    dma_channel_transfer_from_buffer_now(channel, data, sizeof(data) / sizeof(*data));
    dma_channel_wait_for_finish_blocking(channel);
    absolute_time_t end = get_absolute_time();
    printf("DONE1 %llu %llu %llu us\n",to_us_since_boot(start), to_us_since_boot(end), absolute_time_diff_us(start, end));
}
