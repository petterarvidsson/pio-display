#include <acutest.h>
#include <vector>
#include "protocol.hpp"

using namespace protocol;

#define MAX_SIZE 64*1024

uint8_t buf[MAX_SIZE];


template<typename T> void test_encode_decode(T t) {
  size_t pos = 0;
  encode_item(buf, pos, t);
  size_t encoded_size = pos;
  pos = 0;
  Item item = decode_item(buf, pos);
  TEST_CHECK(encoded_size == pos);
  TEST_CHECK(std::holds_alternative<T>(item));
  T decoded = std::get<T>(item);
  TEST_CHECK(decoded == t);
}

void test_encode_decode_line() {
  Line line(3, Point(3,4), Point(5,6), 2);
  test_encode_decode(line);
}

void test_encode_decode_circle() {
  Circle circle(39, Point(34, 45), 8);
  test_encode_decode(circle);
}

void test_encode_decode_filled_circle() {
  FilledCircle circle(39, Point(34, 45), 8);
  test_encode_decode(circle);
}

void test_encode_decode_sine_segment() {
  SineSegment sine_segment(15, Point(3,7), 22, 5, 0, 8);
  test_encode_decode(sine_segment);
}

void test_encode_decode_print() {
  Print print(7, Point(32, 0), size_13, "Test 1234");
  test_encode_decode(print);
}

void test_encode_decode_print_center() {
  PrintCenter print(7, 32, size_13, "Test 4567");
  test_encode_decode(print);
}

void test_encode_decode() {
  std::vector<Item> items = {
    Line(3, Point(3,4), Point(5,6), 2),
    Circle(39, Point(34, 45), 8),
    FilledCircle(39, Point(34, 45), 8),
    SineSegment(15, Point(3,7), 22, 5, 0, 8),
    Print(7, Point(32, 0), size_13, "Test 1234"),
    PrintCenter(7, 32, size_13, "Test 4567")
  };
  size_t pos = 0;
  for(auto i: items) {
    encode_item(buf, pos, i);
  }
  size_t encoded_size = pos;
  pos = 0;
  std::vector<Item> decoded_items;
  while(pos < encoded_size) {
    Item item = decode_item(buf, pos);
    decoded_items.push_back(item);
  }
  TEST_CHECK(pos == encoded_size);
  TEST_CHECK(items == decoded_items);
}

void test_encode_decode_vector() {
  std::vector<Item> items = {
    Line(3, Point(3,4), Point(5,6), 2),
    Circle(39, Point(34, 45), 8),
    FilledCircle(39, Point(34, 45), 8),
    SineSegment(15, Point(3,7), 22, 5, 0, 8),
    Print(7, Point(32, 0), size_13, "Test 1234"),
    PrintCenter(7, 32, size_13, "Test 4567")
  };
  size_t size = encode_items(buf, items) - sizeof(uint32_t);

  TEST_CHECK(size == *(uint32_t *)&buf[0]);

  std::vector<Item> decoded_items;
  decode_items(buf + sizeof(uint32_t), size, decoded_items);
  TEST_CHECK(items == decoded_items);
}

TEST_LIST = {
  { "encode_decode_line", test_encode_decode_line },
  { "encode_decode_circle", test_encode_decode_circle },
  { "encode_decode_filled_circle", test_encode_decode_filled_circle },
  { "encode_decode_sine_segment", test_encode_decode_sine_segment },
  { "encode_decode_print", test_encode_decode_print },
  { "encode_decode_print_center", test_encode_decode_print_center },
  { "encode_decode", test_encode_decode },
  { "encode_decode_vector", test_encode_decode_vector },
  { NULL, NULL }
};
