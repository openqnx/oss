/* tzdata: the zoneinfo files as a QNX program uses them. QNX's libc takes
 * POSIX TZ strings, and also reads a TZif file when TZ names one by path
 * (TZ=/usr/local/share/zoneinfo/Europe/Berlin, or with a leading ':'); a bare
 * zone name is looked up in /usr/share/zoneinfo, which on QNX is not where
 * packages install, so the test uses paths.
 * Known answers: the files are TZif version 2+ with the expected POSIX
 * footer; localtime_r() through the libc gives the right offset, DST flag
 * and abbreviation for zones and dates; a past rule that the footer alone
 * does not carry (Sao Paulo's DST, abolished in 2019) proves the transition
 * table is read; zone.tab lists the zones. */
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *zdir(void) {
  const char *e = getenv("QNX_PORT_ZONEINFO");          /* another tree */
  return e ? e : "/usr/local/share/zoneinfo";
}

/* The last line of a TZif v2+ file: the POSIX TZ string for times after the
 * table. */
static int footer(const char *zone, char *out, size_t n, char *ver) {
  char p[512]; snprintf(p, sizeof p, "%s/%s", zdir(), zone);
  FILE *f = fopen(p, "rb");
  if (!f) return 0;
  char hdr[5] = {0};
  int ok = fread(hdr, 1, 5, f) == 5 && !memcmp(hdr, "TZif", 4);
  *ver = hdr[4];
  if (ok && fseek(f, -128, SEEK_END) == 0) {
    char buf[129] = {0}; size_t r = fread(buf, 1, 128, f);
    buf[r] = 0;
    /* ...\n<footer>\n at the end */
    if (r >= 2 && buf[r - 1] == '\n') {
      buf[r - 1] = 0;
      size_t k = r - 1;                 /* binary data before it: no strrchr */
      while (k > 0 && buf[k - 1] != '\n') k--;
      snprintf(out, n, "%s", buf + k);
    } else ok = 0;
  } else ok = 0;
  fclose(f);
  return ok;
}

struct lt { const char *zone; time_t t; const char *want; long gmtoff; int isdst; };
static const struct lt cases[] = {
  /* 2023-11-14 22:13:20 UTC and 2023-07-10 14:40:00 UTC */
  { "Europe/Berlin",       1700000000, "2023-11-14 23:13:20 CET",  3600, 0 },
  { "Europe/Berlin",       1689000000, "2023-07-10 16:40:00 CEST", 7200, 1 },
  { "America/New_York",    1700000000, "2023-11-14 17:13:20 EST", -18000, 0 },
  { "America/New_York",    1689000000, "2023-07-10 10:40:00 EDT", -14400, 1 },
  { "Asia/Kolkata",        1700000000, "2023-11-15 03:43:20 IST", 19800, 0 },
  { "Australia/Sydney",    1700000000, "2023-11-15 09:13:20 AEDT", 39600, 1 },
  /* 2018-12-01 00:00 UTC: Brazil still had DST (-02); the footer today is
   * "<-03>3", which alone would give 21:00 -03. */
  { "America/Sao_Paulo",   1543622400, "2018-11-30 22:00:00 -02", -7200, 1 },
  { "America/Sao_Paulo",   1700000000, "2023-11-14 19:13:20 -03", -10800, 0 },
  /* 2012-06-01 00:00 UTC: Moscow was on permanent +04 (2011-2014) */
  { "Europe/Moscow",       1338508800, "2012-06-01 04:00:00 MSK", 14400, 0 },
  { "Europe/Moscow",       1700000000, "2023-11-15 01:13:20 MSK", 10800, 0 },
  /* a link (backward): Asia/Calcutta -> Asia/Kolkata */
  { "Asia/Calcutta",       1700000000, "2023-11-15 03:43:20 IST", 19800, 0 },
  { "UTC",                 1700000000, "2023-11-14 22:13:20 UTC", 0, 0 },
};

int main(void) {
  QT_BEGIN("tzdata");
  char what[800], ft[128], ver = 0;

  int ok = footer("America/New_York", ft, sizeof ft, &ver);
  printf("  info America/New_York: TZif version '%c', footer \"%s\"\n", ver ? ver : '?', ft);
  QT_CHECK(ok && ver >= '2', "America/New_York is a TZif file, version 2 or later");
  QT_CHECK(ok && !strcmp(ft, "EST5EDT,M3.2.0,M11.1.0"), "its POSIX footer is EST5EDT,M3.2.0,M11.1.0");
  ok = footer("Europe/Berlin", ft, sizeof ft, &ver);
  QT_CHECK(ok && !strcmp(ft, "CET-1CEST,M3.5.0,M10.5.0/3"), "Europe/Berlin footer CET-1CEST,M3.5.0,M10.5.0/3");
  ok = footer("posixrules", ft, sizeof ft, &ver);
  QT_CHECK(ok && !strcmp(ft, "EST5EDT,M3.2.0,M11.1.0"), "posixrules is America/New_York's rules");

  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    const struct lt *c = &cases[i];
    char tz[512];
    /* both spellings the libc accepts for a file: plain path and ":path" */
    snprintf(tz, sizeof tz, "%s%s/%s", (i & 1) ? ":" : "", zdir(), c->zone);
    setenv("TZ", tz, 1); tzset();
    struct tm tm; char buf[64] = "";
    int got = localtime_r(&c->t, &tm) != NULL;
    if (got) strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S %Z", &tm);
    snprintf(what, sizeof what, "TZ=%s at %lld: %s (got \"%s\", gmtoff %ld, isdst %d)", tz, (long long)c->t,
             c->want, buf, got ? (long)tm.tm_gmtoff : 0L, got ? tm.tm_isdst : -1);
    QT_CHECK(got && !strcmp(buf, c->want) && tm.tm_gmtoff == c->gmtoff && (tm.tm_isdst > 0) == c->isdst, what);
  }

  /* mktime goes the other way: local Berlin wall clock -> epoch */
  { char tz[512]; snprintf(tz, sizeof tz, "%s/Europe/Berlin", zdir()); setenv("TZ", tz, 1); tzset();
    struct tm tm = {0}; tm.tm_year = 2024 - 1900; tm.tm_mon = 2; tm.tm_mday = 31; tm.tm_hour = 12; tm.tm_isdst = -1;
    time_t t = mktime(&tm);   /* 2024-03-31 12:00 CEST = 10:00 UTC = 1711879200 */
    snprintf(what, sizeof what, "mktime(2024-03-31 12:00 Berlin, after the switch) = 1711879200 (%lld, isdst %d)", (long long)t, tm.tm_isdst);
    QT_CHECK(t == 1711879200 && tm.tm_isdst == 1, what); }

  /* zone.tab: the table of zones by country */
  { char p[512], line[512]; int n = 0, berlin = 0;
    snprintf(p, sizeof p, "%s/zone.tab", zdir());
    FILE *f = fopen(p, "r");
    while (f && fgets(line, sizeof line, f)) {
      if (line[0] == '#') continue;
      n++;
      if (!strncmp(line, "DE\t", 3) && strstr(line, "\tEurope/Berlin")) berlin = 1;
    }
    if (f) fclose(f);
    snprintf(what, sizeof what, "zone.tab lists %d zones, DE -> Europe/Berlin among them", n);
    QT_CHECK(n > 300 && berlin, what); }
  { char p[512], v[32] = ""; snprintf(p, sizeof p, "%s/+VERSION", zdir());
    FILE *f = fopen(p, "r"); if (f) { if (fgets(v, sizeof v, f)) v[strcspn(v, "\n")] = 0; fclose(f); }
    printf("  info tzdata version %s\n", v);
    QT_CHECK(v[0] == '2' && strlen(v) == 5, "+VERSION names the release"); }
  QT_END();
}
