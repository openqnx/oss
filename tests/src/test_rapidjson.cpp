// RapidJSON: DOM parse, modify, write.
#include "common.h"
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
int main() {
  QT_BEGIN("rapidjson");
  rapidjson::Document d;
  d.Parse("{\"sdp\":8,\"arches\":[\"aarch64le\",\"x86_64\"]}");
  QT_CHECK(!d.HasParseError(), "parses");
  QT_CHECK(d["sdp"].GetInt() == 8, "number");
  QT_CHECK(d["arches"].Size() == 2, "array");
  d["sdp"].SetInt(9);
  rapidjson::StringBuffer sb; rapidjson::Writer<rapidjson::StringBuffer> w(sb); d.Accept(w);
  QT_CHECK(std::string(sb.GetString()).find("\"sdp\":9") != std::string::npos, "writes modified value");
  QT_END();
}
