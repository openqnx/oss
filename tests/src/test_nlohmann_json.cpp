#include "common.h"
#include <nlohmann/json.hpp>
int main() {
  QT_BEGIN("nlohmann_json");
  auto j = nlohmann::json::parse(R"({"ecu":"gateway","ports":[30490,30491],"up":true})");
  QT_CHECK(j["ecu"] == "gateway", "string field");
  QT_CHECK(j["ports"][1] == 30491, "array element");
  QT_CHECK(j["up"].get<bool>(), "bool field");
  nlohmann::json out; out["id"] = 42; out["name"] = "adas";
  QT_CHECK(out.dump() == R"({"id":42,"name":"adas"})", "serialize");
  printf("  info nlohmann_json %d.%d.%d\n", NLOHMANN_JSON_VERSION_MAJOR,
         NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
  QT_END();
}
