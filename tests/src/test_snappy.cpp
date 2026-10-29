// Snappy: compress and uncompress.
#include "common.h"
#include <snappy.h>
int main() {
  QT_BEGIN("snappy");
  std::string in(10000, 'q'), c, out;
  snappy::Compress(in.data(), in.size(), &c);
  QT_CHECK(c.size() < in.size() / 10, "compresses");
  QT_CHECK(snappy::Uncompress(c.data(), c.size(), &out) && out == in, "round trip");
  QT_END();
}
