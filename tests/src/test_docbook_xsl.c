#include "common.h"
#include <string.h>
#include <libxml/parser.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>
#include <stdlib.h>
#include <unistd.h>
/* docbook-xsl through libxslt: a DocBook refentry turned into a man page
 * with the manpages stylesheet (EXSLT registered: it writes the page as a
 * file of its own, qnxdemo.1). Known answer: the .TH line with the
 * command's name and section, and its NAME section. */
static const char *doc =
  "<?xml version='1.0'?>"
  "<!DOCTYPE refentry PUBLIC '-//OASIS//DTD DocBook XML V4.5//EN' 'docbookx.dtd'>"
  "<refentry>"
  "<refmeta><refentrytitle>qnxdemo</refentrytitle><manvolnum>1</manvolnum></refmeta>"
  "<refnamediv><refname>qnxdemo</refname><refpurpose>demonstrate the stylesheets</refpurpose></refnamediv>"
  "<refsect1><title>Description</title><para>Runs on QNX.</para></refsect1></refentry>";
int main(void) {
  QT_BEGIN("docbook-xsl");
  exsltRegisterAll();
  char tmp[] = "/tmp/docbook-XXXXXX"; QT_CHECK(mkdtemp(tmp) && chdir(tmp) == 0, "work directory");
  const char *dirs[] = { "/usr/local/share/xml/docbook/xsl-stylesheets-1.79.2", "/data/oss/usr/local/share/xml/docbook/xsl-stylesheets-1.79.2" };
  char xsl[256]; FILE *f = NULL;
  for (int i = 0; i < 2 && !f; i++) { snprintf(xsl, sizeof xsl, "%s/manpages/docbook.xsl", dirs[i]); f = fopen(xsl, "r"); }
  QT_CHECK(f != NULL, "manpages/docbook.xsl installed"); if (f) fclose(f);
  xsltStylesheetPtr ss = xsltParseStylesheetFile((const xmlChar *)xsl);
  QT_CHECK(ss != NULL, "stylesheet parsed (with its includes)");
  xmlDocPtr in = xmlReadMemory(doc, (int)strlen(doc), "qnxdemo.xml", NULL, XML_PARSE_NONET);
  QT_CHECK(in != NULL, "DocBook source parsed");
  if (ss && in) {


    xmlDocPtr out = xsltApplyStylesheet(ss, in, NULL);
    QT_CHECK(out != NULL, "xsltApplyStylesheet");
    xmlChar *buf = NULL;
    FILE *m = fopen("qnxdemo.1", "r");
    QT_CHECK(m != NULL, "qnxdemo.1 written");
    if (m) { static char b[65536]; size_t n = fread(b, 1, sizeof b - 1, m); b[n] = 0; fclose(m); buf = xmlStrdup((xmlChar *)b); unlink("qnxdemo.1"); }
    const char *th = buf ? strstr((char *)buf, ".TH") : NULL;
    if (th) printf("  info %.*s\n", (int)(strchr(th, '\n') ? strchr(th, '\n') - th : 40), th);
    QT_CHECK(th && strstr(th, "QNXDEMO") && strstr(th, "\"1\""), ".TH QNXDEMO \"1\"");
    QT_CHECK(buf && strstr((char *)buf, "qnxdemo \\- demonstrate the stylesheets"), "NAME: qnxdemo \\- demonstrate the stylesheets");
    xmlFree(buf); if (out) xmlFreeDoc(out);
  }
  if (in) xmlFreeDoc(in);
  if (ss) xsltFreeStylesheet(ss);
  if (chdir("/") == 0) rmdir(tmp);
  xsltCleanupGlobals(); xmlCleanupParser();
  QT_END();
}
