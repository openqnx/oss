#include "common.h"
#include <tinyxml2.h>
#include <string>
int main() {
  QT_BEGIN("tinyxml2");
  using namespace tinyxml2;
  XMLDocument doc;
  QT_CHECK(doc.Parse("<cfg><svc id=\"4660\" name=\"diag\"/></cfg>") == XML_SUCCESS, "parse");
  auto *svc = doc.FirstChildElement("cfg")->FirstChildElement("svc");
  QT_CHECK(svc != NULL, "element lookup");
  int id = 0;
  QT_CHECK(svc->QueryIntAttribute("id", &id) == XML_SUCCESS && id == 4660, "int attribute");
  QT_CHECK(std::string(svc->Attribute("name")) == "diag", "string attribute");
  printf("  info tinyxml2 %d.%d.%d\n", TIXML2_MAJOR_VERSION, TIXML2_MINOR_VERSION, TIXML2_PATCH_VERSION);
  QT_END();
}
