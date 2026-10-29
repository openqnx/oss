/* simdjson: on-demand parsing, including the runtime kernel dispatch that
 * picks a SIMD implementation for the CPU the test is actually running on. */
#include "common.h"
#include <simdjson.h>
#include <string>
int main() {
  QT_BEGIN("simdjson");
  QT_CHECK(simdjson::get_active_implementation() != nullptr, "an implementation was selected");
  printf("  info implementation = %s\n",
         simdjson::get_active_implementation()->name().c_str());
  simdjson::ondemand::parser parser;
  auto json = simdjson::padded_string(std::string(
      R"({"name":"qnx","sdp":8,"arches":["aarch64le","x86_64"]})"));
  simdjson::ondemand::document doc;
  QT_CHECK(parser.iterate(json).get(doc) == simdjson::SUCCESS, "iterate");
  int64_t sdp = 0;
  QT_CHECK(doc["sdp"].get_int64().get(sdp) == simdjson::SUCCESS && sdp == 8, "integer field");
  size_t count = 0;
  for (auto v : doc["arches"]) { (void)v; ++count; }
  QT_CHECK(count == 2, "array iteration");
  auto bad = simdjson::padded_string(std::string(R"({"broken":)"));
  simdjson::ondemand::parser p2;
  simdjson::ondemand::document d2;
  auto err = p2.iterate(bad).get(d2);
  bool rejected = (err != simdjson::SUCCESS);
  if (!rejected) { int64_t x; rejected = (d2["broken"].get_int64().get(x) != simdjson::SUCCESS); }
  QT_CHECK(rejected, "malformed input is rejected");
  QT_END();
}
