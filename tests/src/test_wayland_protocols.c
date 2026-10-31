/* wayland-protocols: the protocol XML a compositor or client is generated
 * from. Every XML file must parse (expat) and describe protocols made of
 * interfaces with requests and events; xdg-shell must say what clients rely
 * on (xdg_wm_base v6+, get_xdg_surface, xdg_toplevel's state enum); the
 * package's enum headers must compile with the same values. When the build
 * host had wayland-scanner, xdg-shell.xml was also turned into code (as any
 * consumer does): the generated interface tables are linked with
 * libwayland-client and must agree with the XML. */
#include "common.h"
#include <dirent.h>
#include <expat.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xdg-shell-enum.h>
#include <viewporter-enum.h>
#include <linux-dmabuf-v1-enum.h>
#ifdef HAVE_XDG_SHELL_SCANNED
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#endif

/* what the parser saw, as "kind:iface.name=value" strings */
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
  else if (!strcmp(el, "interface")) { ninterfaces++; snprintf(cur_iface, sizeof cur_iface, "%s", attr(at, "name")); fact("interface:%s=%s%s", cur_iface, attr(at, "version"), ""); }
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
  QT_BEGIN("wayland-protocols");
  const char *roots[] = { getenv("QNX_PORT_DATA_ROOT"), "/usr/local/share", "/data/oss/usr/local/share" };
  char dir[512] = "";
  for (int i = 0; i < 3 && !dir[0]; i++) if (roots[i]) {
    char p[600]; snprintf(p, sizeof p, "%s/wayland-protocols/stable/xdg-shell/xdg-shell.xml", roots[i]);
    if (access(p, R_OK) == 0) snprintf(dir, sizeof dir, "%s/wayland-protocols", roots[i]);
  }
  QT_CHECK(dir[0], "share/wayland-protocols installed");
  if (!dir[0]) QT_END();
  walk(dir);
  printf("  info %d XML files: %d protocols, %d interfaces, %d requests, %d events\n", nfiles, nprotocols, ninterfaces, nrequests, nevents);
  QT_CHECK(nfiles >= 50 && nbad == 0, "every protocol XML file parses (50 or more)");
  QT_CHECK(nprotocols == nfiles && ninterfaces > nprotocols && nrequests > 200 && nevents > 100, "each file is a protocol of interfaces, requests, events");
  QT_CHECK(has("protocol:xdg_shell") && has("protocol:viewporter") && has("protocol:linux_dmabuf_v1"), "stable protocols: xdg_shell, viewporter, linux_dmabuf_v1");
  int ver = 0;
  for (int i = 0; i < nfacts; i++) if (!strncmp(facts[i], "interface:xdg_wm_base=", 22)) ver = atoi(facts[i] + 22);
  printf("  info xdg_wm_base version %d\n", ver);
  QT_CHECK(ver >= 6, "xdg_wm_base version >= 6");
  QT_CHECK(has("request:xdg_wm_base.get_xdg_surface") && has("event:xdg_wm_base.ping") && has("request:xdg_wm_base.pong"), "xdg_wm_base: get_xdg_surface, ping/pong");
  QT_CHECK(has("entry:xdg_toplevel.state.maximized=1") && has("entry:xdg_toplevel.state.fullscreen=2") && has("event:xdg_toplevel.configure"),
           "xdg_toplevel: state enum (maximized 1, fullscreen 2), configure event");
  QT_CHECK(XDG_TOPLEVEL_STATE_MAXIMIZED == 1 && XDG_TOPLEVEL_STATE_FULLSCREEN == 2 && XDG_WM_BASE_ERROR_ROLE == 0
           && XDG_POSITIONER_ANCHOR_TOP_LEFT == 5, "xdg-shell-enum.h: the same values");
  QT_CHECK(WP_VIEWPORT_ERROR_BAD_VALUE == 0 && ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT == 1, "viewporter and linux-dmabuf enum headers");
#ifdef HAVE_XDG_SHELL_SCANNED
  int nreq = 0, nev = 0;
  for (int i = 0; i < nfacts; i++) {
    if (!strncmp(facts[i], "request:xdg_toplevel.", 21)) nreq++;
    if (!strncmp(facts[i], "event:xdg_toplevel.", 19)) nev++;
  }
  printf("  info generated xdg_toplevel_interface: %d methods, %d events, version %d\n",
         xdg_toplevel_interface.method_count, xdg_toplevel_interface.event_count, xdg_toplevel_interface.version);
  QT_CHECK(!strcmp(xdg_wm_base_interface.name, "xdg_wm_base") && xdg_wm_base_interface.version == ver, "scanned code: xdg_wm_base interface, same version");
  QT_CHECK(xdg_toplevel_interface.method_count == nreq && xdg_toplevel_interface.event_count == nev, "scanned code: xdg_toplevel requests/events match the XML");
  QT_CHECK(!strcmp(xdg_wm_base_interface.methods[2].name, "get_xdg_surface") && xdg_wm_base_interface.methods[2].types[1] == &wl_surface_interface,
           "get_xdg_surface takes a wl_surface (resolved against libwayland-client)");
#else
  printf("  info wayland-scanner output not built in (no scanner on the build host)\n");
#endif
  QT_END();
}
