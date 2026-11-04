/* hwdata: the installed ID databases looked up the way lspci/lsusb read
 * them (a vendor line, then its tab-indented device lines). Checks:
 * 8086:100e "82540EM Gigabit Ethernet Controller" (QEMU's e1000),
 * 8086:1237 "440FX - 82441FX PMC [Natoma]", 1af4:1000 "Virtio network
 * device", 10de:1b80 "GP104 [GeForce GTX 1080]"; USB 1d6b:0002 "2.0 root
 * hub", 046d:c52b "Unifying Receiver"; pnp.ids PNP -> Microsoft; the OUI
 * registry 00-00-0C -> Cisco. */
#include "common.h"
#include <stdlib.h>

#define D "/usr/local/share/hwdata/"
static char vname[256], dname[256];
static int look(const char *file, const char *ven, const char *dev) {
  FILE *f = fopen(file, "r"); char l[1024]; int in = 0; size_t vl = strlen(ven), dl = strlen(dev);
  vname[0] = dname[0] = 0;
  while (f && fgets(l, sizeof l, f)) {
    l[strcspn(l, "\r\n")] = 0;
    if (l[0] == '#' || !l[0]) continue;
    if (l[0] != '\t') { if (in) break; if (!strncmp(l, ven, vl) && !strncmp(l + vl, "  ", 2)) { in = 1; snprintf(vname, sizeof vname, "%s", l + vl + 2); } continue; }
    if (in && l[1] != '\t' && !strncmp(l + 1, dev, dl) && !strncmp(l + 1 + dl, "  ", 2)) { snprintf(dname, sizeof dname, "%s", l + 1 + dl + 2); break; }
  }
  if (f) fclose(f);
  return dname[0] != 0;
}
static int grepline(const char *file, const char *prefix, const char *want) {
  FILE *f = fopen(file, "r"); char l[1024]; int ok = 0;
  while (f && !ok && fgets(l, sizeof l, f)) if (!strncmp(l, prefix, strlen(prefix)) && strstr(l, want)) ok = 1;
  if (f) fclose(f);
  return ok;
}
int main(void) {
  QT_BEGIN("hwdata");
  static const struct { const char *f, *v, *d, *vn, *dn; } t[] = {
    { D "pci.ids", "8086", "100e", "Intel Corporation", "82540EM Gigabit Ethernet Controller" },
    { D "pci.ids", "8086", "1237", "Intel Corporation", "440FX - 82441FX PMC [Natoma]" },
    { D "pci.ids", "1af4", "1000", "Red Hat, Inc.", "Virtio network device" },
    { D "pci.ids", "10de", "1b80", "NVIDIA Corporation", "GP104 [GeForce GTX 1080]" },
    { D "usb.ids", "1d6b", "0002", "Linux Foundation", "2.0 root hub" },
    { D "usb.ids", "046d", "c52b", "Logitech, Inc.", "Unifying Receiver" },
  };
  for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) {
    char what[256];
    int ok = look(t[i].f, t[i].v, t[i].d) && !strcmp(vname, t[i].vn) && !strcmp(dname, t[i].dn);
    snprintf(what, sizeof what, "%s %s:%s -> %s / %s", strstr(t[i].f, "pci") ? "PCI" : "USB", t[i].v, t[i].d, t[i].vn, t[i].dn);
    QT_CHECK(ok, what);
    if (!ok) printf("  info got \"%s\" / \"%s\"\n", vname, dname);
  }
  QT_CHECK(grepline(D "pnp.ids", "PNP\t", "Microsoft"), "pnp.ids: PNP -> Microsoft");
  QT_CHECK(grepline(D "oui.txt", "00-00-0C", "Cisco Systems"), "oui.txt: 00-00-0C -> Cisco Systems");
  QT_END();
}
