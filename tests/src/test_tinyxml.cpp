// TinyXML 2.6.2 (libtinyxml, STL interface): parse a document and walk it
// (elements, attributes with typed queries, text, comments, entities),
// error reporting with row and column, building a document and printing it
// with TiXmlPrinter, save/load through a file, the std::string and stream
// interfaces, and the two parser bugs fixed in the port: a truncated UTF-8
// lead byte (CVE-2021-42260, used to loop forever) and a cut-off XML
// declaration (CVE-2023-34194) both end in a parse error.
#include "common.h"
#include <tinyxml.h>
#include <sstream>
#include <string>
#include <cstdio>
#include <unistd.h>

int main() {
  QT_BEGIN("tinyxml");
  const char *xml =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<!-- config -->\n"
    "<config version=\"3\" ratio=\"0.75\">\n"
    "  <item id=\"1\">alpha &amp; beta</item>\n"
    "  <item id=\"2\" enabled=\"true\">gamma</item>\n"
    "  <empty/>\n"
    "</config>\n";
  TiXmlDocument doc;
  doc.Parse(xml, 0, TIXML_ENCODING_UTF8);
  QT_CHECK(!doc.Error(), "Parse() of a well-formed document");
  TiXmlElement *root = doc.RootElement();
  QT_CHECK(root && std::string(root->Value()) == "config", "root element is <config>");
  int ver = 0; double ratio = 0;
  QT_CHECK(root && root->QueryIntAttribute("version", &ver) == TIXML_SUCCESS && ver == 3 &&
           root->QueryDoubleAttribute("ratio", &ratio) == TIXML_SUCCESS && ratio == 0.75, "typed attributes version=3, ratio=0.75");
  QT_CHECK(root && root->QueryIntAttribute("missing", &ver) == TIXML_NO_ATTRIBUTE, "a missing attribute: TIXML_NO_ATTRIBUTE");
  int n = 0; std::string texts;
  for (TiXmlElement *e = root ? root->FirstChildElement("item") : 0; e; e = e->NextSiblingElement("item")) { n++; texts += e->GetText(); texts += "|"; }
  QT_CHECK(n == 2 && texts == "alpha & beta|gamma|", "two <item>s, entity &amp; decoded");
  TiXmlHandle h(&doc);
  TiXmlElement *second = h.FirstChildElement("config").Child("item", 1).ToElement();
  QT_CHECK(second && second->Attribute(std::string("enabled")) && *second->Attribute(std::string("enabled")) == "true", "TiXmlHandle path to the second item");
  QT_CHECK(doc.FirstChild() && doc.FirstChild()->ToDeclaration() && std::string(doc.FirstChild()->ToDeclaration()->Version()) == "1.0", "declaration version 1.0");
  QT_CHECK(doc.FirstChild()->NextSibling()->ToComment() != 0, "the comment node");

  TiXmlDocument bad;
  bad.Parse("<a>\n  <b>\n</a>");
  QT_CHECK(bad.Error() && bad.ErrorRow() >= 2, "mismatched tags: an error, with its row");
  printf("  info error: %s (row %d, col %d)\n", bad.ErrorDesc(), bad.ErrorRow(), bad.ErrorCol());

  TiXmlDocument out;
  out.LinkEndChild(new TiXmlDeclaration("1.0", "", ""));
  TiXmlElement *r = new TiXmlElement("r");
  r->SetAttribute("n", 42);
  r->LinkEndChild(new TiXmlText("x < y"));
  out.LinkEndChild(r);
  TiXmlPrinter pr; pr.SetIndent(""); pr.SetLineBreak("");
  out.Accept(&pr);
  QT_CHECK(std::string(pr.CStr()) == "<?xml version=\"1.0\" ?><r n=\"42\">x &lt; y</r>", "TiXmlPrinter output, text escaped");
  char path[64]; snprintf(path, sizeof path, "/tmp/qnx-tinyxml-%d.xml", (int)getpid());
  QT_CHECK(out.SaveFile(path), "SaveFile");
  TiXmlDocument in(path);
  QT_CHECK(in.LoadFile() && in.RootElement() && std::string(in.RootElement()->GetText()) == "x < y", "LoadFile reads it back");
  remove(path);
  std::ostringstream os; os << *r;
  QT_CHECK(os.str() == "<r n=\"42\">x &lt; y</r>", "operator<< (STL interface)");
  std::istringstream is("<s a=\"1\"><t/></s>"); TiXmlElement se("");
  is >> se;
  QT_CHECK(std::string(se.Value()) == "s" && se.FirstChildElement("t"), "operator>> (STL interface)");

  TiXmlDocument c1; c1.Parse("<a>\xef", 0, TIXML_ENCODING_UTF8);
  QT_CHECK(c1.Error(), "truncated UTF-8 lead byte: an error, no hang (CVE-2021-42260)");
  TiXmlDocument c2; c2.Parse("<?xml version=\"1.0\" ", 0, TIXML_ENCODING_UTF8);
  QT_CHECK(c2.Error() || !c2.RootElement(), "cut-off XML declaration: no abort (CVE-2023-34194)");
  QT_END();
}
