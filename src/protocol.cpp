#include "protocol.hpp"
#include "overloaded.hpp"
#include <cstring>

namespace protocol {

  enum ItemType { LineType = 1, CircleType = 2, FilledCircleType = 3, SineSegmentType = 4, PrintType = 5, PrintCenterType = 6};

  Item decode_item(const uint8_t * const buf, size_t &pos) {
    ItemType item_type = (ItemType)buf[pos];
    pos += 1;
    const uint8_t * const obj = &buf[pos];
    switch(item_type) {
    case LineType:
      pos += sizeof(Line);
      return *(Line *)obj;
    case CircleType:
      pos += sizeof(Circle);
      return  *(Circle *)obj;
    case FilledCircleType:
      pos += sizeof(FilledCircle);
      return *(FilledCircle *)obj;
    case SineSegmentType:
      pos += sizeof(SineSegment);
      return *(SineSegment *)obj;
    case PrintType:
      pos += sizeof(Print);
      return *(Print *)obj;
    case PrintCenterType:
      pos += sizeof(PrintCenter);
      return *(PrintCenter *)obj;
    }
  }

  void decode_items(const uint8_t * const buf, size_t size, std::vector<Item> &items) {
    size_t pos = 0;
    while(pos < size) {
      Item item = decode_item(buf, pos);
      items.push_back(item);
    }
  }

  template<typename T> static void encode(uint8_t *buf, size_t &pos, const ItemType t, const T item) {
    buf[pos] = t;
    pos += 1;
    memcpy(&buf[pos], &item, sizeof(T));
    pos += sizeof(T);
  }

  void encode_item(uint8_t *buf, size_t &pos, const Item item) {
    std::visit(util::overloaded{
        [buf, &pos](const Line item) {
          encode(buf, pos, LineType, item);
        },
          [buf, &pos](const Circle item) {
            encode(buf, pos, CircleType, item);
          },
          [buf, &pos](const FilledCircle item) {
            encode(buf, pos, FilledCircleType, item);
          },
          [buf, &pos](const SineSegment item) {
            encode(buf, pos, SineSegmentType, item);
          },
          [buf, &pos](const Print item) {
            encode(buf, pos, PrintType, item);
          },
          [buf, &pos](const PrintCenter item) {
            encode(buf, pos, PrintCenterType, item);
          },
          }, item);
  }

  size_t encode_items(uint8_t *buf, const std::vector<Item> items) {
    size_t pos = sizeof(uint32_t);
    for(auto i: items) {
      encode_item(buf, pos, i);
    }
    *(uint32_t*)&buf[0] = pos - sizeof(uint32_t);
    return pos;
  }

}
