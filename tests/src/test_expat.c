/* expat: stream-parse a small document and count what the handlers saw. */
#include "common.h"
#include <expat.h>
static int starts, ends, chars;
static void XMLCALL on_start(void *u, const XML_Char *n, const XML_Char **a) {
  (void)u; (void)n; (void)a; ++starts;
}
static void XMLCALL on_end(void *u, const XML_Char *n) { (void)u; (void)n; ++ends; }
static void XMLCALL on_text(void *u, const XML_Char *s, int len) {
  (void)u; (void)s; chars += len;
}
int main(void) {
  QT_BEGIN("expat");
  XML_Parser p = XML_ParserCreate("UTF-8");
  QT_CHECK(p != NULL, "XML_ParserCreate");
  XML_SetElementHandler(p, on_start, on_end);
  XML_SetCharacterDataHandler(p, on_text);
  static const char doc[] = "<ports><p arch='aarch64le'>zlib</p><p>zstd</p></ports>";
  QT_CHECK(XML_Parse(p, doc, (int)(sizeof doc - 1), 1) == XML_STATUS_OK, "XML_Parse");
  QT_CHECK(starts == 3 && ends == 3, "three elements opened and closed");
  QT_CHECK(chars == 8, "character data collected");
  XML_ParserFree(p);
  p = XML_ParserCreate("UTF-8");
  static const char bad[] = "<a><b></a>";
  QT_CHECK(XML_Parse(p, bad, (int)(sizeof bad - 1), 1) == XML_STATUS_ERROR,
           "mismatched tags are rejected");
  QT_CHECK(XML_GetErrorCode(p) == XML_ERROR_TAG_MISMATCH, "error code is TAG_MISMATCH");
  XML_ParserFree(p);
  QT_END();
}
