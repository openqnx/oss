/* geoip: a two-entry GeoIP v1 country database written here (one trie node:
 * 0.0.0.0/1 -> US, 128.0.0.0/1 -> DE, then the structure-info trailer naming
 * the Country Edition), opened with the library and with geoiplookup.
 * Checks: the code table (US <-> its id, "United States"), the database
 * edition, lookups on each side of the split (8.8.8.8 -> US, 200.1.2.3 ->
 * DE, 127.255.255.255 -> US, 128.0.0.0 -> DE), the memory-cache mode giving
 * the same answers, and geoiplookup -f printing "US, United States". */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <GeoIP.h>

#define COUNTRY_BEGIN 16776960
static void rec(unsigned char *p, unsigned v) { p[0] = v & 255; p[1] = (v >> 8) & 255; p[2] = (v >> 16) & 255; }

int main(void) {
  QT_BEGIN("geoip");
  int us = GeoIP_id_by_code("US"), de = GeoIP_id_by_code("DE");
  QT_CHECK(us > 0 && de > 0 && !strcmp(GeoIP_code_by_id(us), "US") && !strcmp(GeoIP_name_by_id(us), "United States"),
           "country table: US <-> its id, \"United States\"");
  printf("  info GeoIP %s, US=%d DE=%d\n", GeoIP_lib_version(), us, de);
  char path[64]; snprintf(path, sizeof path, "/tmp/qnxport-geoip-%d.dat", (int)getpid());
  unsigned char db[10];
  rec(db, COUNTRY_BEGIN + us); rec(db + 3, COUNTRY_BEGIN + de);
  db[6] = db[7] = db[8] = 255; db[9] = GEOIP_COUNTRY_EDITION;
  FILE *f = fopen(path, "wb"); int w = f && fwrite(db, 1, sizeof db, f) == sizeof db; if (f) fclose(f);
  QT_CHECK(w, "database written");
  int flags[2] = { GEOIP_STANDARD, GEOIP_MEMORY_CACHE };
  for (int m = 0; m < 2; m++) {
    GeoIP *gi = GeoIP_open(path, flags[m]);
    char what[96];
    snprintf(what, sizeof what, "%s: GeoIP_open, Country Edition", m ? "GEOIP_MEMORY_CACHE" : "GEOIP_STANDARD");
    QT_CHECK(gi && GeoIP_database_edition(gi) == GEOIP_COUNTRY_EDITION, what);
    if (!gi) continue;
    const char *a = GeoIP_country_code_by_addr(gi, "8.8.8.8"), *b = GeoIP_country_code_by_addr(gi, "200.1.2.3");
    const char *c = GeoIP_country_code_by_addr(gi, "127.255.255.255"), *d = GeoIP_country_code_by_addr(gi, "128.0.0.0");
    snprintf(what, sizeof what, "%s: 8.8.8.8 -> US, 200.1.2.3 -> DE, the /1 boundary", m ? "cached" : "standard");
    QT_CHECK(a && !strcmp(a, "US") && b && !strcmp(b, "DE") && c && !strcmp(c, "US") && d && !strcmp(d, "DE"), what);
    const char *n = GeoIP_country_name_by_addr(gi, "200.1.2.3");
    if (!m) QT_CHECK(n && !strcmp(n, "Germany"), "country name by address: Germany");
    GeoIP_delete(gi);
  }
  char cmd[160]; snprintf(cmd, sizeof cmd, "geoiplookup -f %s 9.9.9.9 2>&1", path);
  FILE *p = popen(cmd, "r"); char line[256] = "";
  if (p) { if (!fgets(line, sizeof line, p)) line[0] = 0; pclose(p); }
  line[strcspn(line, "\n")] = 0;
  printf("  info geoiplookup: %s\n", line);
  QT_CHECK(strstr(line, "US, United States") != NULL, "geoiplookup -f db 9.9.9.9: \"US, United States\"");
  unlink(path);
  QT_END();
}
