#include "common.h"
#include <yaml-cpp/yaml.h>
int main() {
  QT_BEGIN("yaml-cpp");
  YAML::Node n = YAML::Load("ecu:\n  name: gateway\n  cores: 4\n  nets: [can0, can1]\n");
  QT_CHECK(n["ecu"]["name"].as<std::string>() == "gateway", "nested scalar");
  QT_CHECK(n["ecu"]["cores"].as<int>() == 4, "int conversion");
  QT_CHECK(n["ecu"]["nets"].size() == 2, "sequence size");
  QT_CHECK(n["ecu"]["nets"][1].as<std::string>() == "can1", "sequence element");
  YAML::Emitter e; e << YAML::BeginMap << YAML::Key << "k" << YAML::Value << 1 << YAML::EndMap;
  QT_CHECK(std::string(e.c_str()) == "k: 1", "emitter");
  QT_END();
}
