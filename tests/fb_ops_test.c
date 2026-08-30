#include <acutest.h>
#include "fb_ops.h"
uint8_t buf[32];

void test_array(uint8_t *target, size_t size) {
  for(uint32_t i = 0; i < size; i++) {
    TEST_CHECK(target[i] == buf[i]);
    TEST_MSG("[%d]: %02x != %02x\n", i, target[i], buf[i]);
  }
}

void test_split_byte_into() {
  uint8_t r1[] = { 0xE0, 0x00, 0x07 };
  split_byte_into(0x81, buf, 3);
  test_array(r1, 3);
}

void test_split_bytes_into() {
  uint8_t t2[] = { 0x81, 0x18 };
  uint8_t r2[] = { 0xE0, 0x00, 0x07, 0x00, 0x7E, 0x00 };
  split_bytes_into(t2, 2, buf, 3);
  test_array(r2, 6);
}

void test_split_bytes_into_ff() {
  uint8_t t2[] = { 0xFF, 0xFF };
  uint8_t r2[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
  split_bytes_into(t2, 2, buf, 3);
  test_array(r2, 6);
}

void test_split_bytes_into_00() {
  uint8_t t2[] = { 0x00, 0x00 };
  uint8_t r2[] = { 0x00,0x00,0x00,0x00,0x00,0x00 };
  split_bytes_into(t2, 2, buf, 3);
  test_array(r2, 6);
}

void test_split_bytes_into_4bytes() {
  uint8_t t2[] = { 0x03, 0x8C };
  uint8_t r2[] = { 0x00,0x00,0x00,0xFF,0xF0,0x00,0xFF,0x00 };
  split_bytes_into(t2, 2, buf, 4);
  test_array(r2, 8);
}

TEST_LIST = {
  { "split_byte_into", test_split_byte_into },
  { "split_bytes_into", test_split_bytes_into },
  { "split_bytes_into FF", test_split_bytes_into_ff },
  { "split_bytes_into 00", test_split_bytes_into_00 },
  { "split_bytes_into 4 bytes", test_split_bytes_into_4bytes },
  { NULL, NULL }
};
