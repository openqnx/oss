// CLI11: parse a command line.
#include "common.h"
#include <CLI/CLI.hpp>
int main() {
  QT_BEGIN("cli11");
  CLI::App app{"t"}; int n = 0; std::string arch; bool v = false;
  app.add_option("-n", n); app.add_option("--arch", arch); app.add_flag("-v", v);
  const char *argv[] = {"t", "-n", "3", "--arch", "x86_64", "-v"};
  bool ok = true; try { app.parse(6, argv); } catch (...) { ok = false; }
  QT_CHECK(ok, "parses");
  QT_CHECK(n == 3 && arch == "x86_64" && v, "values");
  QT_END();
}
