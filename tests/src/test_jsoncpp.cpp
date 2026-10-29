/* jsoncpp: Value construction, writing and parsing. */
#include "common.h"
#include <json/json.h>
#include <sstream>
int main() {
  QT_BEGIN("jsoncpp");
  Json::Value root;
  root["name"] = "qnx";
  root["sdp"] = 8;
  root["arches"].append("aarch64le");
  root["arches"].append("x86_64");
  Json::StreamWriterBuilder wb;
  wb["indentation"] = "";
  const std::string text = Json::writeString(wb, root);
  QT_CHECK(!text.empty(), "writeString");
  Json::CharReaderBuilder rb;
  Json::Value back;
  std::string errs;
  std::istringstream in(text);
  QT_CHECK(Json::parseFromStream(rb, in, &back, &errs), "parseFromStream");
  QT_CHECK(back["sdp"].asInt() == 8, "integer survives");
  QT_CHECK(back["arches"].size() == 2, "array survives");
  std::istringstream bad("{nope");
  Json::Value ignored;
  QT_CHECK(!Json::parseFromStream(rb, bad, &ignored, &errs), "malformed input is rejected");
  QT_END();
}
