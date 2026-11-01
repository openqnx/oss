/* libxo: one xo_emit() format rendered as text, XML, JSON and HTML into
 * a tmpfile (xo_create_to_file; QNX has no open_memstream), compared with
 * the exact output libxo produces, a list included (a JSON array). */
#include <libxo/xo.h>
#include <stdlib.h>
#include "common.h"

static char *render(xo_style_t style, unsigned flags)
{
  static char buf[4096];
  FILE *fp = tmpfile();
  size_t n;
  xo_handle_t *xop;
  if (fp == NULL) return "";
  xop = xo_create_to_file(fp, style, flags);
  if (xop == NULL) { fclose(fp); return ""; }
  xo_open_container_h(xop, "top");
  xo_emit_h(xop, "{L:Name}: {:name/%s}, {:count/%d} {N:items}\n", "qnx", 42);
  xo_open_list_h(xop, "port");
  xo_open_instance_h(xop, "port");
  xo_emit_h(xop, "{:id/%u}\n", 7u);
  xo_close_instance_h(xop, "port");
  xo_open_instance_h(xop, "port");
  xo_emit_h(xop, "{:id/%u}\n", 8u);
  xo_close_instance_h(xop, "port");
  xo_close_list_h(xop, "port");
  xo_close_container_h(xop, "top");
  xo_finish_h(xop);
  xo_destroy(xop);
  rewind(fp);
  n = fread(buf, 1, sizeof buf - 1, fp);
  buf[n] = '\0';
  fclose(fp);
  return buf;
}

int main(void)
{
  char *s;
  QT_BEGIN("libxo");

  s = render(XO_STYLE_TEXT, 0);
  QT_CHECK(strcmp(s, "Name: qnx, 42 items\n7\n8\n") == 0, "text output");
  if (strcmp(s, "Name: qnx, 42 items\n7\n8\n")) printf("    got [%s]\n", s);

  s = render(XO_STYLE_XML, 0);
  QT_CHECK(strcmp(s, "<top><name>qnx</name><count>42</count><port><id>7</id></port><port><id>8</id></port></top>") == 0, "XML output");
  if (strstr(s, "<count>42</count>") == NULL) printf("    got [%s]\n", s);

  s = render(XO_STYLE_JSON, 0);
  QT_CHECK(strcmp(s, "{\"top\": {\"name\":\"qnx\",\"count\":42, \"port\": [{\"id\":7}, {\"id\":8}]}}\n") == 0, "JSON output");
  QT_CHECK(strstr(s, "\"port\": [") != NULL && strstr(s, "{\"id\":8}") != NULL, "JSON list as an array");
  printf("    json: %s", s);

  s = render(XO_STYLE_JSON, XOF_PRETTY);
  QT_CHECK(strstr(s, "\n  \"top\": {\n") != NULL, "JSON pretty-printed (XOF_PRETTY)");

  s = render(XO_STYLE_HTML, 0);
  QT_CHECK(strstr(s, "<div class=\"data\" data-tag=\"name\">qnx</div>") != NULL, "HTML output has the data div");

  QT_END();
}
