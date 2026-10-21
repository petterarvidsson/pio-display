#include "displays.hpp"

namespace protocol {
  using namespace displays;

  enum ItemType { LineType = 1, CircleType = 2, FilledCircleType = 3, SineSegmentType = 4, PrintType = 5, PrintCenterType = 6};

  displays::Item decode_item(uint8_t *buf, size_t *pos) {
    ItemType item_type = (ItemType)buf[*pos];
    *pos += 1;
    uint8_t *obj = buf + 1;
    switch(item_type) {
    case LineType:
     *pos += sizeof(Line);
     return *(Line *)obj;
    case CircleType:
     *pos += sizeof(Circle);
     return  *(Circle *)obj;
    case FilledCircleType:
     *pos += sizeof(FilledCircle);
     return *(FilledCircle *)obj;
    case SineSegmentType:
     *pos += sizeof(SineSegment);
     return *(SineSegment *)obj;
    case PrintType:
     *pos += sizeof(Print);
     return *(Print *)obj;
    case PrintCenterType:
     *pos += sizeof(PrintCenter);
     return *(PrintCenter *)obj;
    }
  }
}
