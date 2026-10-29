// pugixml: parse, query with XPath, and serialise.
#include "common.h"
#include <pugixml.hpp>
#include <sstream>
int main() {
  QT_BEGIN("pugixml");
  pugi::xml_document d;
  auto r = d.load_string("<ports><p name='zlib' v='1.3.1'/><p name='curl' v='8'/></ports>");
  QT_CHECK(r, "document parses");
  QT_CHECK(d.select_nodes("//p").size() == 2, "XPath finds two nodes");
  QT_CHECK(std::string(d.select_node("//p[@name='zlib']").node().attribute("v").value()) == "1.3.1", "attribute lookup");
  std::ostringstream os; d.save(os);
  QT_CHECK(os.str().find("curl") != std::string::npos, "serialises");
  QT_END();
}
