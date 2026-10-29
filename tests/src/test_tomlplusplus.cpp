// toml++: parse a TOML table.
#include "common.h"
#include <toml++/toml.hpp>
int main() {
  QT_BEGIN("tomlplusplus");
  auto t = toml::parse("[target]\nos = \"qnx\"\nsdp = 8\narches = [\"aarch64le\", \"x86_64\"]\n");
  QT_CHECK(t["target"]["os"].value_or(std::string()) == "qnx", "string");
  QT_CHECK(t["target"]["sdp"].value_or(0) == 8, "integer");
  QT_CHECK(t["target"]["arches"].as_array()->size() == 2, "array");
  QT_END();
}
