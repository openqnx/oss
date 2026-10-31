/* retropie (EmulationStation and RetroArch configuration for QNX): read the
 * files the way the front ends do -- es_systems.cfg (libxml2) must define
 * the four systems with launch commands, every gamelist must parse with its
 * games' paths and names, the cover images they reference must exist and
 * decode (libpng; the SVG cover is XML), es_settings / es_input must parse,
 * retroarch.cfg must be a well-formed key = "value" file with the QNX
 * drivers selected, and the target scripts must be valid shell. */
#include "common.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <png.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

static char base[512];
static char *child_text(xmlNode *n, const char *name) {
  for (xmlNode *c = n->children; c; c = c->next)
    if (c->type == XML_ELEMENT_NODE && !strcmp((const char *)c->name, name)) return (char *)xmlNodeGetContent(c);
  return NULL;
}
static int png_ok(const char *path, int *w, int *h) {
  png_image im; memset(&im, 0, sizeof im); im.version = PNG_IMAGE_VERSION;
  if (!png_image_begin_read_from_file(&im, path)) return 0;
  im.format = PNG_FORMAT_RGBA;
  void *buf = malloc(PNG_IMAGE_SIZE(im));
  int ok = buf && png_image_finish_read(&im, NULL, buf, 0, NULL);
  *w = (int)im.width; *h = (int)im.height;
  free(buf); png_image_free(&im);
  return ok;
}

int main(void) {
  QT_BEGIN("retropie");
  const char *roots[] = { getenv("QNX_PORT_DATA_ROOT"), "/usr/local/share", "/data/oss/usr/local/share" };
  for (int i = 0; i < 3 && !base[0]; i++) if (roots[i]) {
    char p[600]; snprintf(p, sizeof p, "%s/retropie/configs/emulationstation/es_systems.cfg", roots[i]);
    if (access(p, R_OK) == 0) snprintf(base, sizeof base, "%s/retropie", roots[i]);
  }
  QT_CHECK(base[0], "share/retropie installed");
  if (!base[0]) QT_END();
  char p[1024], es[700];
  snprintf(es, sizeof es, "%s/configs/emulationstation", base);

  snprintf(p, sizeof p, "%s/es_systems.cfg", es);
  xmlDoc *d = xmlReadFile(p, NULL, XML_PARSE_NONET);
  xmlNode *root = d ? xmlDocGetRootElement(d) : NULL;
  QT_CHECK(root && !strcmp((const char *)root->name, "systemList"), "es_systems.cfg parses: <systemList>");
  int nsys = 0, complete = 0; char names[256] = "";
  for (xmlNode *s = root ? root->children : NULL; s; s = s->next) {
    if (s->type != XML_ELEMENT_NODE || strcmp((const char *)s->name, "system")) continue;
    nsys++;
    char *nm = child_text(s, "name"), *full = child_text(s, "fullname"), *cmd = child_text(s, "command"), *ext = child_text(s, "extension");
    if (nm) { strcat(names, " "); strncat(names, nm, 40); }
    complete += nm && full && cmd && ext && strstr(cmd, "%ROM%") && ext[0] == '.';
    xmlFree(nm); xmlFree(full); xmlFree(cmd); xmlFree(ext);
  }
  printf("  info systems:%s\n", names);
  QT_CHECK(nsys == 4 && strstr(names, " retroarch") && strstr(names, " picoeight") && strstr(names, " openttd") && strstr(names, " testsuite"),
           "4 systems: retroarch, picoeight, openttd, testsuite");
  QT_CHECK(complete == nsys, "each has a name, full name, extension and a %ROM% command");
  if (d) xmlFreeDoc(d);

  const char *lists[] = { "openttd", "picoeight", "retroarch", "testsuite" };
  int games = 0, named = 0, covers = 0, covers_ok = 0, lists_ok = 0;
  for (int i = 0; i < 4; i++) {
    snprintf(p, sizeof p, "%s/gamelists/%s/gamelist.xml", es, lists[i]);
    xmlDoc *g = xmlReadFile(p, NULL, XML_PARSE_NONET);
    xmlNode *r = g ? xmlDocGetRootElement(g) : NULL;
    lists_ok += r && !strcmp((const char *)r->name, "gameList");
    for (xmlNode *x = r ? r->children : NULL; x; x = x->next) {
      if (x->type != XML_ELEMENT_NODE || strcmp((const char *)x->name, "game")) continue;
      games++;
      char *path = child_text(x, "path"), *nm = child_text(x, "name"), *img = child_text(x, "image");
      named += path && nm && nm[0];             /* "Empty Core Bootup" has an empty path on purpose */
      const char *pre = "~/.emulationstation/gamecovers/";
      if (img && !strncmp(img, pre, strlen(pre))) {
        covers++;
        char cp[1400]; snprintf(cp, sizeof cp, "%s/gamecovers/%s", es, img + strlen(pre));
        covers_ok += access(cp, R_OK) == 0;
      }
      xmlFree(path); xmlFree(nm); xmlFree(img);
    }
    if (g) xmlFreeDoc(g);
  }
  printf("  info %d games in 4 gamelists, %d cover references\n", games, covers);
  QT_CHECK(lists_ok == 4 && games >= 10 && named == games, "4 gamelists parse; every game has a path element and a name");
  QT_CHECK(covers > 0 && covers_ok == covers, "every referenced game cover is installed");

  int npng = 0, pngs_ok = 0, w = 0, h = 0;
  snprintf(p, sizeof p, "%s/gamecovers", es);
  DIR *dd = opendir(p); struct dirent *e;
  while (dd && (e = readdir(dd))) {
    if (!strstr(e->d_name, ".png")) continue;
    npng++;
    char fp[1024]; snprintf(fp, sizeof fp, "%s/%s", p, e->d_name);
    if (png_ok(fp, &w, &h) && w > 0 && h > 0) pngs_ok++;
    else printf("  info %s does not decode\n", e->d_name);
  }
  if (dd) closedir(dd);
  QT_CHECK(npng >= 5 && pngs_ok == npng, "every PNG cover decodes (libpng)");
  snprintf(p, sizeof p, "%s/gamecovers/2048.png", es);
  QT_CHECK(png_ok(p, &w, &h) && w == 250 && h == 250, "2048.png is 250x250");
  snprintf(p, sizeof p, "%s/gamecovers/openttd.svg", es);
  xmlDoc *svg = xmlReadFile(p, NULL, XML_PARSE_NONET | XML_PARSE_HUGE);
  QT_CHECK(svg && !strcmp((const char *)xmlDocGetRootElement(svg)->name, "svg"), "openttd.svg is an SVG document");
  if (svg) xmlFreeDoc(svg);

  /* es_settings.cfg has no single root element: ES reads it wrapped */
  snprintf(p, sizeof p, "%s/es_settings.cfg", es);
  FILE *f = fopen(p, "rb"); char *txt = NULL; long len = 0;
  if (f) { fseek(f, 0, SEEK_END); len = ftell(f); fseek(f, 0, SEEK_SET); txt = calloc(1, len + 32); if (fread(txt, 1, len, f) != (size_t)len) len = 0; fclose(f); }
  char *body = txt ? strstr(txt, "?>") : NULL; body = body ? body + 2 : txt;
  char *wrapped = malloc(len + 64); int settings = 0;
  if (body) { snprintf(wrapped, len + 64, "<config>%s</config>", body);
    xmlDoc *sd = xmlReadMemory(wrapped, (int)strlen(wrapped), "es_settings.cfg", NULL, XML_PARSE_NONET);
    for (xmlNode *x = sd ? xmlDocGetRootElement(sd)->children : NULL; x; x = x->next) settings += x->type == XML_ELEMENT_NODE && xmlHasProp(x, (const xmlChar *)"name");
    if (sd) xmlFreeDoc(sd);
  }
  free(wrapped); free(txt);
  QT_CHECK(settings >= 10, "es_settings.cfg: named settings parse");
  snprintf(p, sizeof p, "%s/es_input.cfg", es);
  xmlDoc *in = xmlReadFile(p, NULL, XML_PARSE_NONET);
  xmlNode *ir = in ? xmlDocGetRootElement(in) : NULL; int kb = 0;
  for (xmlNode *x = ir ? ir->children : NULL; x; x = x->next) {
    xmlChar *t = x->type == XML_ELEMENT_NODE ? xmlGetProp(x, (const xmlChar *)"type") : NULL;
    if (t && !strcmp((char *)t, "keyboard")) for (xmlNode *k = x->children; k; k = k->next) kb += k->type == XML_ELEMENT_NODE;
    xmlFree(t);
  }
  QT_CHECK(ir && !strcmp((const char *)ir->name, "inputList") && kb >= 8, "es_input.cfg: a keyboard input configuration");
  if (in) xmlFreeDoc(in);

  snprintf(p, sizeof p, "%s/configs/retroarch.cfg", base);
  f = fopen(p, "r"); char line[1024]; int keys = 0, bad = 0, drivers = 0;
  while (f && fgets(line, sizeof line, f)) {
    if (line[0] == '#' || line[0] == '\n') continue;
    char k[256], v[700];
    if (sscanf(line, "%255s = \"%699[^\"]\"", k, v) == 2 || sscanf(line, "%255s = \"%[^\n]", k, v) >= 1) {
      keys++;
      if (!strcmp(k, "video_driver") && !strcmp(v, "vulkan")) drivers++;
      if (!strcmp(k, "input_driver") && !strcmp(v, "qnx_input")) drivers++;
      if (!strcmp(k, "menu_driver") && !strcmp(v, "ozone")) drivers++;
    } else bad++;
  }
  if (f) fclose(f);
  printf("  info retroarch.cfg: %d settings, %d malformed\n", keys, bad);
  QT_CHECK(keys > 3000 && bad == 0, "retroarch.cfg: 3000+ key = \"value\" settings, none malformed");
  QT_CHECK(drivers == 3, "video_driver vulkan, input_driver qnx_input, menu_driver ozone");

  const char *scripts[] = { "backup.sh", "uninstall.sh" };
  for (int i = 0; i < 2; i++) {
    char cmd[1200], what[100];
    snprintf(cmd, sizeof cmd, "sh -n '%s/target_scripts/%s' 2>&1", base, scripts[i]);
    snprintf(what, sizeof what, "target_scripts/%s is valid shell (sh -n)", scripts[i]);
    QT_CHECK(system(cmd) == 0, what);
  }
  QT_END();
}
