// magic_enum: enum <-> string reflection.
#include "common.h"
// 0.9.6 moved the headers into include/magic_enum/
#if __has_include(<magic_enum/magic_enum.hpp>)
#include <magic_enum/magic_enum.hpp>
#else
#include <magic_enum.hpp>
#endif
enum class Arch { aarch64le, x86_64 };
int main() {
  QT_BEGIN("magic_enum");
  QT_CHECK(magic_enum::enum_name(Arch::x86_64) == "x86_64", "enum_name");
  QT_CHECK(magic_enum::enum_cast<Arch>("aarch64le").value() == Arch::aarch64le, "enum_cast");
  QT_CHECK(magic_enum::enum_count<Arch>() == 2, "enum_count");
  QT_END();
}
