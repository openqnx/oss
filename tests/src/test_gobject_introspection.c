/* gobject-introspection: no typelibs ship on the target (GLib's own cannot be
 * generated in a cross build), so the test writes a small GIR describing
 * real GLib functions, compiles it with g-ir-compiler, and then:
 *  - loads it through libgirepository and looks up functions, a constant and
 *    an enumeration by name;
 *  - calls g_ascii_digit_value and g_str_has_prefix through
 *    g_function_info_invoke (libffi) and checks the results;
 *  - inspects it with g-ir-inspect and turns it back into GIR with
 *    g-ir-generate. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <girepository.h>

static const char *gir =
  "<?xml version=\"1.0\"?>\n"
  "<repository version=\"1.2\" xmlns=\"http://www.gtk.org/introspection/core/1.0\"\n"
  "            xmlns:c=\"http://www.gtk.org/introspection/c/1.0\">\n"
  "  <namespace name=\"QnxTest\" version=\"1.0\" shared-library=\"libglib-2.0.so.0\"\n"
  "             c:identifier-prefixes=\"Qnx\" c:symbol-prefixes=\"g\">\n"
  "    <constant name=\"ANSWER\" value=\"42\" c:type=\"QNX_ANSWER\">\n"
  "      <type name=\"gint\" c:type=\"gint\"/>\n"
  "    </constant>\n"
  "    <enumeration name=\"Color\" c:type=\"QnxColor\">\n"
  "      <member name=\"red\" value=\"0\" c:identifier=\"QNX_COLOR_RED\"/>\n"
  "      <member name=\"green\" value=\"1\" c:identifier=\"QNX_COLOR_GREEN\"/>\n"
  "      <member name=\"blue\" value=\"7\" c:identifier=\"QNX_COLOR_BLUE\"/>\n"
  "    </enumeration>\n"
  "    <function name=\"ascii_digit_value\" c:identifier=\"g_ascii_digit_value\">\n"
  "      <return-value transfer-ownership=\"none\"><type name=\"gint\" c:type=\"gint\"/></return-value>\n"
  "      <parameters>\n"
  "        <parameter name=\"c\" transfer-ownership=\"none\"><type name=\"gchar\" c:type=\"gchar\"/></parameter>\n"
  "      </parameters>\n"
  "    </function>\n"
  "    <function name=\"str_has_prefix\" c:identifier=\"g_str_has_prefix\">\n"
  "      <return-value transfer-ownership=\"none\"><type name=\"gboolean\" c:type=\"gboolean\"/></return-value>\n"
  "      <parameters>\n"
  "        <parameter name=\"str\" transfer-ownership=\"none\"><type name=\"utf8\" c:type=\"const gchar*\"/></parameter>\n"
  "        <parameter name=\"prefix\" transfer-ownership=\"none\"><type name=\"utf8\" c:type=\"const gchar*\"/></parameter>\n"
  "      </parameters>\n"
  "    </function>\n"
  "  </namespace>\n"
  "</repository>\n";

static char dir[64];
static int run_capture(const char *cmd, char *out, size_t n) {
  char full[768]; snprintf(full, sizeof full, "cd %s && %s 2>&1", dir, cmd);
  FILE *p = popen(full, "r"); if (!p) return -1;
  size_t k = fread(out, 1, n - 1, p); out[k] = 0;
  int st = pclose(p);
  printf("  info $ %s -> %d\n", cmd, st);
  return st;
}

int main(void) {
  QT_BEGIN("gobject-introspection");
  strcpy(dir, "/tmp/qnxport-gi.XXXXXX");
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char p[128]; snprintf(p, sizeof p, "%s/QnxTest-1.0.gir", dir);
  FILE *f = fopen(p, "w"); if (f) { fputs(gir, f); fclose(f); }
  static char out[16384];
  int st = run_capture("g-ir-compiler --output=QnxTest-1.0.typelib QnxTest-1.0.gir", out, sizeof out);
  if (st) printf("  info output: %s\n", out);
  QT_CHECK(st == 0, "g-ir-compiler compiled the GIR");
  snprintf(p, sizeof p, "%s/QnxTest-1.0.typelib", dir);
  char magic[16] = {0};
  f = fopen(p, "rb"); if (f) { if (fread(magic, 1, 16, f) != 16) magic[0] = 0; fclose(f); }
  QT_CHECK(!memcmp(magic, "GOBJ\nMETADATA\r\n\032", 16), "typelib has the GOBJ METADATA magic");

  g_irepository_prepend_search_path(dir);
  GError *err = NULL;
  GITypelib *tl = g_irepository_require(NULL, "QnxTest", "1.0", 0, &err);
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  QT_CHECK(tl != NULL, "g_irepository_require(QnxTest 1.0)");
  if (tl) {
    const char *shl = g_irepository_get_shared_library(NULL, "QnxTest");
    printf("  info shared library: %s\n", shl ? shl : "(null)");
    QT_CHECK(shl && !strcmp(shl, "libglib-2.0.so.0"), "shared-library recorded");
    int n = g_irepository_get_n_infos(NULL, "QnxTest");
    printf("  info %d info(s) in the namespace\n", n);
    QT_CHECK(n == 4, "4 entries: constant, enum, 2 functions");

    GIBaseInfo *c = g_irepository_find_by_name(NULL, "QnxTest", "ANSWER");
    QT_CHECK(c && g_base_info_get_type(c) == GI_INFO_TYPE_CONSTANT, "ANSWER is a constant");
    if (c) {
      GIArgument v; g_constant_info_get_value((GIConstantInfo *)c, &v);
      QT_CHECK(v.v_int == 42, "ANSWER = 42");
      g_constant_info_free_value((GIConstantInfo *)c, &v);
      g_base_info_unref(c);
    }
    GIBaseInfo *e = g_irepository_find_by_name(NULL, "QnxTest", "Color");
    QT_CHECK(e && g_base_info_get_type(e) == GI_INFO_TYPE_ENUM, "Color is an enum");
    if (e) {
      int nv = g_enum_info_get_n_values((GIEnumInfo *)e);
      GIValueInfo *b = nv == 3 ? g_enum_info_get_value((GIEnumInfo *)e, 2) : NULL;
      QT_CHECK(nv == 3 && b && !strcmp(g_base_info_get_name((GIBaseInfo *)b), "blue") && g_value_info_get_value(b) == 7,
               "Color has 3 members, blue = 7");
      if (b) g_base_info_unref((GIBaseInfo *)b);
      g_base_info_unref(e);
    }

    GIBaseInfo *fn = g_irepository_find_by_name(NULL, "QnxTest", "ascii_digit_value");
    QT_CHECK(fn && g_base_info_get_type(fn) == GI_INFO_TYPE_FUNCTION, "ascii_digit_value is a function");
    if (fn) {
      const char *sym = g_function_info_get_symbol((GIFunctionInfo *)fn);
      QT_CHECK(sym && !strcmp(sym, "g_ascii_digit_value"), "symbol g_ascii_digit_value");
      QT_CHECK(g_callable_info_get_n_args((GICallableInfo *)fn) == 1, "one argument");
      GIArgument in, ret;
      in.v_int8 = '7';
      gboolean ok = g_function_info_invoke((GIFunctionInfo *)fn, &in, 1, NULL, 0, &ret, &err);
      if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
      printf("  info invoke ascii_digit_value('7') = %d\n", ok ? ret.v_int : -999);
      QT_CHECK(ok && ret.v_int == 7, "invoke: g_ascii_digit_value('7') = 7");
      in.v_int8 = 'x';
      ok = g_function_info_invoke((GIFunctionInfo *)fn, &in, 1, NULL, 0, &ret, &err);
      g_clear_error(&err);
      QT_CHECK(ok && ret.v_int == -1, "invoke: g_ascii_digit_value('x') = -1");
      g_base_info_unref(fn);
    }
    fn = g_irepository_find_by_name(NULL, "QnxTest", "str_has_prefix");
    QT_CHECK(fn != NULL, "str_has_prefix found");
    if (fn) {
      GIArgument in[2], ret;
      in[0].v_string = (char *)"QNX ports"; in[1].v_string = (char *)"QNX";
      gboolean ok = g_function_info_invoke((GIFunctionInfo *)fn, in, 2, NULL, 0, &ret, &err);
      g_clear_error(&err);
      QT_CHECK(ok && ret.v_boolean, "invoke: g_str_has_prefix(\"QNX ports\", \"QNX\") = TRUE");
      in[1].v_string = (char *)"ports";
      ok = g_function_info_invoke((GIFunctionInfo *)fn, in, 2, NULL, 0, &ret, &err);
      g_clear_error(&err);
      QT_CHECK(ok && !ret.v_boolean, "invoke: g_str_has_prefix(\"QNX ports\", \"ports\") = FALSE");
      GITypeInfo *rt = g_callable_info_get_return_type((GICallableInfo *)fn);
      QT_CHECK(rt && g_type_info_get_tag(rt) == GI_TYPE_TAG_BOOLEAN, "return type tag is boolean");
      if (rt) g_base_info_unref((GIBaseInfo *)rt);
      g_base_info_unref(fn);
    }
  }

  setenv("GI_TYPELIB_PATH", dir, 1);
  st = run_capture("g-ir-inspect --print-shlibs --print-typelibs --version=1.0 QnxTest", out, sizeof out);
  printf("  info output: %s", out);
  QT_CHECK(st == 0 && strstr(out, "shlib: libglib-2.0.so.0"), "g-ir-inspect --print-shlibs");
  st = run_capture("g-ir-generate QnxTest-1.0.typelib", out, sizeof out);
  QT_CHECK(st == 0, "g-ir-generate");
  QT_CHECK(strstr(out, "<namespace name=\"QnxTest\"") && strstr(out, "c:identifier=\"g_str_has_prefix\"") &&
           strstr(out, "name=\"ANSWER\"") && strstr(out, "value=\"7\""),
           "g-ir-generate reproduces namespace, function, constant and enum");
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd);
  QT_END();
}
