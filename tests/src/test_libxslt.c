/* libxslt: compile a stylesheet and apply it to a document. */
#include "common.h"
#include <libxml/parser.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
int main(void) {
  QT_BEGIN("libxslt");
  static const char sheet[] =
      "<xsl:stylesheet version='1.0'"
      " xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>"
      "<xsl:output method='text'/>"
      "<xsl:template match='/'><xsl:for-each select='//p'>"
      "<xsl:value-of select='.'/>,</xsl:for-each></xsl:template>"
      "</xsl:stylesheet>";
  static const char doc[] = "<ports><p>zlib</p><p>zstd</p></ports>";
  xmlDocPtr sd = xmlReadMemory(sheet, (int)(sizeof sheet - 1), "s.xsl", NULL, 0);
  QT_CHECK(sd != NULL, "parse stylesheet");
  xsltStylesheetPtr st = xsltParseStylesheetDoc(sd);   /* takes ownership of sd */
  QT_CHECK(st != NULL, "xsltParseStylesheetDoc");
  xmlDocPtr in = xmlReadMemory(doc, (int)(sizeof doc - 1), "d.xml", NULL, 0);
  xmlDocPtr out = xsltApplyStylesheet(st, in, NULL);
  QT_CHECK(out != NULL, "xsltApplyStylesheet");
  xmlChar *text = NULL; int len = 0;
  QT_CHECK(xsltSaveResultToString(&text, &len, out, st) == 0, "xsltSaveResultToString");
  QT_CHECK(text && strcmp((char *)text, "zlib,zstd,") == 0, "transform produced expected text");
  if (text) xmlFree(text);
  xmlFreeDoc(out); xmlFreeDoc(in); xsltFreeStylesheet(st);
  xsltCleanupGlobals(); xmlCleanupParser();
  QT_END();
}
