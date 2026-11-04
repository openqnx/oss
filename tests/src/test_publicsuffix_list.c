/* publicsuffix-list: the installed public_suffix_list.dat loaded into
 * libpsl at run time (psl_load_file), and upstream's own test_psl.txt
 * (installed next to it) run against it, every checkPublicSuffix(domain,
 * registrable-domain) line, the way libpsl's test suite reads that file.
 * Also: co.uk, github.io are public suffixes and example.co.uk is not, the
 * effective_tld_names.dat name loads the same list, and the psl tool
 * answers from the file. */
#include "common.h"
#include <stdlib.h>
#include <libpsl.h>

#define DIR "/usr/local/share/publicsuffix/"
int main(void) {
  QT_BEGIN("publicsuffix-list");
  psl_ctx_t *psl = psl_load_file(DIR "public_suffix_list.dat");
  QT_CHECK(psl != NULL, "psl_load_file(public_suffix_list.dat)");
  if (!psl) QT_END();
  printf("  info %d suffixes, %d exceptions\n", psl_suffix_count(psl), psl_suffix_exception_count(psl));
  QT_CHECK(psl_suffix_count(psl) > 9000, "more than 9000 rules");
  QT_CHECK(psl_is_public_suffix(psl, "co.uk") && psl_is_public_suffix(psl, "github.io") && !psl_is_public_suffix(psl, "example.co.uk"),
           "co.uk and github.io are public suffixes, example.co.uk is not");
  FILE *f = fopen(DIR "test_psl.txt", "r");
  QT_CHECK(f != NULL, "test_psl.txt is installed");
  int ok = 0, bad = 0; char buf[256];
  while (f && fgets(buf, sizeof buf, f)) {
    char d[128], e[128], semi[2]; const char *p = buf; int dn = 0, en = 0;
    while (*p == ' ' || *p == '\t') p++;
    if (strncmp(p, "checkPublicSuffix", 17)) continue;
    if (sscanf(p, "checkPublicSuffix ( '%127[^']' , '%127[^']' ) %1[;]", d, e, semi) == 3) ;
    else if (sscanf(p, "checkPublicSuffix ( '%127[^']' , null ) %1[;]", d, semi) == 2) en = 1;
    else if (sscanf(p, "checkPublicSuffix ( null , null ) %1[;]", semi) == 1) dn = en = 1;
    else { bad++; printf("  info malformed: %s", buf); continue; }
    char *lower = NULL; const char *dom = dn ? NULL : d;
    if (dom && psl_str_to_utf8lower(dom, "utf-8", "en", &lower) == PSL_SUCCESS) dom = lower;
    const char *r = psl_registrable_domain(psl, dom);
    if ((r && !en && !strcmp(r, e)) || (!r && en)) ok++;
    else { bad++; printf("  info %s -> %s (expected %s)\n", dom ? dom : "NULL", r ? r : "NULL", en ? "NULL" : e); }
    psl_free_string(lower);
  }
  if (f) fclose(f);
  printf("  info test_psl.txt: %d passed, %d failed\n", ok, bad);
  QT_CHECK(ok > 60 && bad == 0, "every test_psl.txt check (registrable domains) passes");
  psl_free(psl);
  psl = psl_load_file(DIR "effective_tld_names.dat");
  QT_CHECK(psl && psl_is_public_suffix(psl, "co.uk"), "effective_tld_names.dat loads the same list");
  psl_free(psl);
  FILE *p = popen("psl --load-psl-file " DIR "public_suffix_list.dat --print-reg-domain www.bbc.co.uk foo.github.io 2>&1", "r");
  char l1[128] = "", l2[128] = "";
  if (p) { if (!fgets(l1, sizeof l1, p)) l1[0] = 0; if (!fgets(l2, sizeof l2, p)) l2[0] = 0; pclose(p); }
  printf("  info psl: %s  info psl: %s", l1, l2);
  QT_CHECK(strstr(l1, "bbc.co.uk") && strstr(l2, "foo.github.io"), "psl --load-psl-file: bbc.co.uk, foo.github.io");
  QT_END();
}
