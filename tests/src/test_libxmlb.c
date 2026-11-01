/* libxmlb: XML compiled into a binary silo and queried. Checks: a silo
 * from an XML string; XPath-like queries by element, attribute predicate and
 * text predicate, with the number of results; attribute and text accessors
 * (also as integers); parent/child/next navigation; the silo saved to a file
 * and mmap-loaded back with the same GUID and answers; a builder importing
 * a gzip- and an xz-compressed source file (zlib/liblzma); export back to
 * XML; and a malformed query reported as an error. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <zlib.h>
#include <lzma.h>
#include <xmlb.h>

static const char XML[] =
  "<components origin=\"qnx\">"
  "<component type=\"desktop\"><id>org.qnx.Editor</id><name>Editor</name><priority>10</priority></component>"
  "<component type=\"console\"><id>org.qnx.Shell</id><name>Shell</name><priority>5</priority></component>"
  "<component type=\"desktop\"><id>org.qnx.Viewer</id><name>Viewer</name><priority>7</priority></component>"
  "</components>";

static int write_gz(const char *path) {
  gzFile g = gzopen(path, "wb9"); if (!g) return 0;
  int ok = gzwrite(g, XML, (unsigned)strlen(XML)) == (int)strlen(XML);
  return gzclose(g) == Z_OK && ok;
}
static int write_xz(const char *path) {
  uint8_t out[4096]; size_t pos = 0;
  if (lzma_easy_buffer_encode(6, LZMA_CHECK_CRC64, NULL, (const uint8_t *)XML, strlen(XML), out, &pos, sizeof out) != LZMA_OK) return 0;
  FILE *f = fopen(path, "wb"); if (!f) return 0;
  int ok = fwrite(out, 1, pos, f) == pos;
  return fclose(f) == 0 && ok;
}

int main(void) {
  QT_BEGIN("libxmlb");
  GError *err = NULL;
  XbSilo *silo = xb_silo_new_from_xml(XML, &err);
  QT_CHECK(silo != NULL, "silo compiled from XML");
  if (!silo) { printf("  info %s\n", err ? err->message : "?"); QT_END(); }

  GPtrArray *all = xb_silo_query(silo, "components/component", 0, &err);
  QT_CHECK(all && all->len == 3, "query components/component: 3 results");
  if (all) g_ptr_array_unref(all);
  g_clear_error(&err);
  GPtrArray *desk = xb_silo_query(silo, "components/component[@type='desktop']/id", 0, &err);
  g_clear_error(&err);
  QT_CHECK(desk && desk->len == 2 && !g_strcmp0(xb_node_get_text(g_ptr_array_index(desk, 0)), "org.qnx.Editor") &&
           !g_strcmp0(xb_node_get_text(g_ptr_array_index(desk, 1)), "org.qnx.Viewer"), "attribute predicate: the two desktop ids");
  if (desk) g_ptr_array_unref(desk);
  XbNode *sh = xb_silo_query_first(silo, "components/component/id[text()='org.qnx.Shell']/..", &err);
  g_clear_error(&err);
  QT_CHECK(sh && !g_strcmp0(xb_node_get_attr(sh, "type"), "console"), "text() predicate and .. find the shell component");
  if (sh) {
    QT_CHECK(xb_node_query_text_as_uint(sh, "priority", NULL) == 5, "child text as an integer: priority 5");
    XbNode *parent = xb_node_get_parent(sh);
    QT_CHECK(parent && !g_strcmp0(xb_node_get_element(parent), "components") && !g_strcmp0(xb_node_get_attr(parent, "origin"), "qnx"),
             "parent element and its attribute");
    XbNode *next = xb_node_get_next(sh);
    QT_CHECK(next && !g_strcmp0(xb_node_query_text(next, "name", NULL), "Viewer"), "next sibling");
    if (parent) g_object_unref(parent);
    if (next) g_object_unref(next);
    g_object_unref(sh);
  }
  g_clear_error(&err);
  GPtrArray *none = xb_silo_query(silo, "components/component[@type='nothing']", 0, &err);
  QT_CHECK(none == NULL && g_error_matches(err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND), "no match: G_IO_ERROR_NOT_FOUND");
  g_clear_error(&err);
  GPtrArray *bad = xb_silo_query(silo, "components/component[@type=", 0, &err);
  QT_CHECK(bad == NULL && err != NULL, "malformed query reported as an error");
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }

  char dir[64], path[128], gz[128], xz[128];
  snprintf(dir, sizeof dir, "/tmp/qnxport-xmlb-%d", (int)getpid());
  g_mkdir_with_parents(dir, 0755);
  snprintf(path, sizeof path, "%s/test.xmlb", dir);
  GFile *file = g_file_new_for_path(path);
  QT_CHECK(xb_silo_save_to_file(silo, file, NULL, &err), "silo saved to a file");
  char *guid = g_strdup(xb_silo_get_guid(silo));
  XbSilo *s2 = xb_silo_new();
  QT_CHECK(xb_silo_load_from_file(s2, file, XB_SILO_LOAD_FLAG_NONE, NULL, &err), "silo mapped back from the file");
  char *guid2 = g_strdup(xb_silo_get_guid(s2));
  QT_CHECK(!g_strcmp0(guid, guid2), "same GUID after the round trip");
  XbNode *v = xb_silo_query_first(s2, "components/component/id[text()='org.qnx.Viewer']/../priority", NULL);
  QT_CHECK(v && xb_node_get_text_as_uint(v) == 7, "loaded silo answers queries");
  if (v) g_object_unref(v);
  g_free(guid); g_free(guid2); g_object_unref(s2); g_object_unref(file);

  snprintf(gz, sizeof gz, "%s/a.xml.gz", dir);
  snprintf(xz, sizeof xz, "%s/b.xml.xz", dir);
  QT_CHECK(write_gz(gz) && write_xz(xz), "gzip and xz source files written");
  XbBuilder *b = xb_builder_new();
  const char *srcs[] = { gz, xz };
  int loaded = 0;
  for (int i = 0; i < 2; i++) {
    XbBuilderSource *src = xb_builder_source_new();
    GFile *sf = g_file_new_for_path(srcs[i]);
    if (xb_builder_source_load_file(src, sf, XB_BUILDER_SOURCE_FLAG_NONE, NULL, &err)) { xb_builder_import_source(b, src); loaded++; }
    else { printf("  info %s: %s\n", srcs[i], err ? err->message : "?"); g_clear_error(&err); }
    g_object_unref(sf); g_object_unref(src);
  }
  QT_CHECK(loaded == 2, "builder loads the gzip and the xz source");
  XbSilo *s3 = xb_builder_compile(b, XB_BUILDER_COMPILE_FLAG_NONE, NULL, &err);
  GPtrArray *both = s3 ? xb_silo_query(s3, "components/component/id", 0, NULL) : NULL;
  QT_CHECK(both && both->len == 6, "compiled silo of both sources: 6 components");
  if (both) g_ptr_array_unref(both);
  XbNode *root = s3 ? xb_silo_query_first(s3, "components", NULL) : NULL;
  char *xmlout = root ? xb_node_export(root, XB_NODE_EXPORT_FLAG_NONE, NULL) : NULL;
  QT_CHECK(xmlout && strstr(xmlout, "<id>org.qnx.Shell</id>") && strstr(xmlout, "origin=\"qnx\""), "node exported back to XML");
  g_free(xmlout);
  if (root) g_object_unref(root);
  if (s3) g_object_unref(s3);
  g_object_unref(b);
  g_object_unref(silo);
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
