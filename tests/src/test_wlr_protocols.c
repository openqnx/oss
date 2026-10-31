/* wlr-protocols: the wlroots protocol XML (layer shell, screencopy, output
 * management, ...). Every file must parse (expat) into a protocol with
 * interfaces; the layer shell must define the layers panels and launchers
 * use (background 0, bottom 1, top 2, overlay 3), its surface requests and
 * anchors; screencopy and output management must have their managers. */
#include "common.h"
#include <dirent.h>
#include <expat.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static char **facts; static int nfacts, capfacts;
static char cur_iface[96], cur_enum[96];
static int nprotocols, ninterfaces, nrequests, nevents;
static void fact(const char *fmt, const char *a, const char *b, const char *c) {
  if (nfacts == capfacts) { capfacts = capfacts ? capfacts * 2 : 1024; facts = realloc(facts, capfacts * sizeof *facts); }
  char buf[256]; snprintf(buf, sizeof buf, fmt, a, b, c); facts[nfacts++] = strdup(buf);
}
static int has(const char *s) { for (int i = 0; i < nfacts; i++) if (!strcmp(facts[i], s)) return 1; return 0; }
static const char *attr(const char **at, const char *k) { for (int i = 0; at[i]; i += 2) if (!strcmp(at[i], k)) return at[i + 1]; return ""; }
static void XMLCALL start(void *ud, const char *el, const char **at) {
  (void)ud;
  if (!strcmp(el, "protocol")) { nprotocols++; fact("protocol:%s%s%s", attr(at, "name"), "", ""); }
  else if (!strcmp(el, "interface")) { ninterfaces++; snprintf(cur_iface, sizeof cur_iface, "%s", attr(at, "name")); fact("interface:%s%s%s", cur_iface, "", ""); }
  else if (!strcmp(el, "request")) { nrequests++; fact("request:%s.%s%s", cur_iface, attr(at, "name"), ""); }
  else if (!strcmp(el, "event")) { nevents++; fact("event:%s.%s%s", cur_iface, attr(at, "name"), ""); }
  else if (!strcmp(el, "enum")) snprintf(cur_enum, sizeof cur_enum, "%s", attr(at, "name"));
  else if (!strcmp(el, "entry")) { char k[200]; snprintf(k, sizeof k, "%s.%s", cur_iface, cur_enum); fact("entry:%s.%s=%s", k, attr(at, "name"), attr(at, "value")); }
}
static void XMLCALL end(void *ud, const char *el) { (void)ud; (void)el; }
static int nfiles, nbad;
static void parse_file(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) { nbad++; return; }
  XML_Parser p = XML_ParserCreate(NULL);
  XML_SetElementHandler(p, start, end);
  char buf[16384]; size_t n; int ok = 1;
  while (ok && (n = fread(buf, 1, sizeof buf, f)) > 0) ok = XML_Parse(p, buf, (int)n, 0) != XML_STATUS_ERROR;
  if (ok) ok = XML_Parse(p, buf, 0, 1) != XML_STATUS_ERROR;
  if (!ok) { nbad++; printf("  info %s: %s at line %lu\n", path, XML_ErrorString(XML_GetErrorCode(p)), XML_GetCurrentLineNumber(p)); }
  XML_ParserFree(p); fclose(f); nfiles++;
}
static void walk(const char *dir) {
  DIR *d = opendir(dir); struct dirent *e;
  if (!d) return;
  while ((e = readdir(d))) {
    if (e->d_name[0] == '.') continue;
    char p[1024]; struct stat st; snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
    if (stat(p, &st)) continue;
    if (S_ISDIR(st.st_mode)) walk(p);
    else if (strlen(e->d_name) > 4 && !strcmp(e->d_name + strlen(e->d_name) - 4, ".xml")) parse_file(p);
  }
  closedir(d);
}

int main(void) {
  QT_BEGIN("wlr-protocols");
  const char *roots[] = { getenv("QNX_PORT_DATA_ROOT"), "/usr/local/share", "/data/oss/usr/local/share" };
  char dir[512] = "";
  for (int i = 0; i < 3 && !dir[0]; i++) if (roots[i]) {
    char p[600]; snprintf(p, sizeof p, "%s/wlr-protocols/unstable/wlr-layer-shell-unstable-v1.xml", roots[i]);
    if (access(p, R_OK) == 0) snprintf(dir, sizeof dir, "%s/wlr-protocols", roots[i]);
  }
  QT_CHECK(dir[0], "share/wlr-protocols installed");
  if (!dir[0]) QT_END();
  walk(dir);
  printf("  info %d XML files: %d protocols, %d interfaces, %d requests, %d events\n", nfiles, nprotocols, ninterfaces, nrequests, nevents);
  QT_CHECK(nfiles >= 10 && nbad == 0, "every protocol XML file parses (10 or more)");
  QT_CHECK(nprotocols == nfiles && ninterfaces >= nfiles, "each is a protocol with interfaces");
  QT_CHECK(has("protocol:wlr_layer_shell_unstable_v1") && has("interface:zwlr_layer_shell_v1") && has("request:zwlr_layer_shell_v1.get_layer_surface"),
           "layer shell: zwlr_layer_shell_v1.get_layer_surface");
  QT_CHECK(has("entry:zwlr_layer_shell_v1.layer.background=0") && has("entry:zwlr_layer_shell_v1.layer.bottom=1")
           && has("entry:zwlr_layer_shell_v1.layer.top=2") && has("entry:zwlr_layer_shell_v1.layer.overlay=3"), "layers background 0, bottom 1, top 2, overlay 3");
  QT_CHECK(has("request:zwlr_layer_surface_v1.set_anchor") && has("entry:zwlr_layer_surface_v1.anchor.top=1") && has("entry:zwlr_layer_surface_v1.anchor.right=8")
           && has("event:zwlr_layer_surface_v1.configure"), "layer surface: set_anchor, anchors top 1 .. right 8, configure");
  QT_CHECK(has("interface:zwlr_screencopy_manager_v1") && has("request:zwlr_screencopy_manager_v1.capture_output"), "screencopy: capture_output");
  QT_CHECK(has("interface:zwlr_output_manager_v1") && has("interface:zwlr_foreign_toplevel_manager_v1"), "output management, foreign toplevel management");
  QT_END();
}
