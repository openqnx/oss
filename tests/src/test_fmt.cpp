#include "common.h"
#include <fmt/core.h>
#include <fmt/format.h>
int main() {
  QT_BEGIN("fmt");
  QT_CHECK(fmt::format("{}-{}", "ecu", 7) == "ecu-7", "positional format");
  QT_CHECK(fmt::format("{:04d}", 42) == "0042", "zero padded");
  QT_CHECK(fmt::format("{:.3f}", 3.14159) == "3.142", "float precision");
  printf("  info fmt %d\n", FMT_VERSION);
  QT_END();
}
