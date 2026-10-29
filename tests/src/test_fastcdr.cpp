#include "common.h"
#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
int main() {
  QT_BEGIN("fastcdr");
  char raw[256];
  eprosima::fastcdr::FastBuffer buf(raw, sizeof(raw));
  eprosima::fastcdr::Cdr ser(buf);
  const uint32_t id = 4660; const double speed = 88.5; const std::string name = "adas";
  ser << id << speed << name;
  size_t len = ser.get_serialized_data_length();
  QT_CHECK(len > 0, "serialized some bytes");
  eprosima::fastcdr::Cdr de(buf);
  uint32_t id2 = 0; double speed2 = 0; std::string name2;
  de >> id2 >> speed2 >> name2;
  QT_CHECK(id2 == id, "uint32 round trip");
  QT_CHECK(speed2 == speed, "double round trip");
  QT_CHECK(name2 == name, "string round trip");
  QT_END();
}
