// double-conversion: shortest round-trip formatting and parsing.
#include "common.h"
#include <string>
#include <double-conversion/double-conversion.h>
using namespace double_conversion;
int main() {
  QT_BEGIN("double-conversion");
  char buf[64]; StringBuilder sb(buf, sizeof buf);
  DoubleToStringConverter::EcmaScriptConverter().ToShortest(0.1 + 0.2, &sb);
  QT_CHECK(std::string(sb.Finalize()) == "0.30000000000000004", "shortest representation");
  StringToDoubleConverter c(0, 0.0, 0.0, nullptr, nullptr); int n;
  QT_CHECK(c.StringToDouble("2.5e3", 5, &n) == 2500.0, "parse");
  QT_END();
}
