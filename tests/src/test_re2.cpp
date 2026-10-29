// RE2: full match, partial match with captures, global replace.
#include "common.h"
#include <re2/re2.h>
int main() {
  QT_BEGIN("re2");
  QT_CHECK(RE2::FullMatch("aarch64le", "aarch64(le|be)"), "full match");
  std::string os; int major;
  QT_CHECK(RE2::PartialMatch("QNX 8.0.3", "(\\w+) (\\d+)\\.", &os, &major) && os == "QNX" && major == 8, "captures");
  std::string s = "a-b-c"; RE2::GlobalReplace(&s, "-", "+");
  QT_CHECK(s == "a+b+c", "global replace");
  QT_END();
}
