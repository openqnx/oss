/* libxml2: parse a document, walk it with XPath, serialise it back. */
#include "common.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
int main(void) {
  QT_BEGIN("libxml2");
  LIBXML_TEST_VERSION
  static const char doc[] =
      "<ports><p arch='aarch64le'>zlib</p><p arch='x86_64'>zstd</p></ports>";
  xmlDocPtr d = xmlReadMemory(doc, (int)(sizeof doc - 1), "mem.xml", NULL, 0);
  QT_CHECK(d != NULL, "xmlReadMemory");
  xmlXPathContextPtr ctx = xmlXPathNewContext(d);
  xmlXPathObjectPtr r = xmlXPathEvalExpression((const xmlChar *) "/ports/p", ctx);
  QT_CHECK(r && r->nodesetval && r->nodesetval->nodeNr == 2, "XPath found both <p>");
  xmlChar *arch = xmlGetProp(r->nodesetval->nodeTab[0], (const xmlChar *) "arch");
  QT_CHECK(arch && xmlStrcmp(arch, (const xmlChar *) "aarch64le") == 0, "attribute read");
  xmlFree(arch);
  xmlChar *out = NULL; int len = 0;
  xmlDocDumpMemory(d, &out, &len);
  QT_CHECK(out && len > 0, "xmlDocDumpMemory");
  xmlFree(out);
  xmlXPathFreeObject(r); xmlXPathFreeContext(ctx); xmlFreeDoc(d);
  xmlDocPtr bad = xmlReadMemory("<a><b></a>", 10, "bad.xml", NULL, XML_PARSE_NOERROR);
  QT_CHECK(bad == NULL, "mismatched tags are rejected");
  xmlCleanupParser();
  QT_END();
}
