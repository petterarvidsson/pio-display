#include "pico/stdlib.h"
#include "stdio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "spi.pio.h"

#define SCLK 18
#define MOSI 19
#define CS 20
#define DC 21

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

static uint32_t data[] = {
  1 << 16, 0x8d14afe3,
  1 << 16, 0xe3B00210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  1 << 16, 0xe3B10210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  1 << 16, 0xe3B20210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  1 << 16, 0xe3B30210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  1 << 16, 0xe3B40210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  1 << 16, 0xe3B50210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  1 << 16, 0xe3B60210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  1 << 16, 0xe3B70210,
  (32 << 16) | 1,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001,
  0x00000001, 0x00000001, 0x00000001, 0x00000001 ,0x00000001, 0x00000001, 0x00000001, 0x00000001
};

int main() {
    stdio_init_all();

    gpio_init(CS);
    gpio_set_dir(CS, GPIO_OUT);
    gpio_put(CS, 1);

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