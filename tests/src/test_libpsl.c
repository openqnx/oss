/* libpsl: public-suffix classification using the compiled-in list. */
#include "common.h"
#include <libpsl.h>
int main(void) {
  QT_BEGIN("libpsl");
  printf("  info libpsl %s\n", psl_get_version());
  const psl_ctx_t *psl = psl_builtin();
  QT_CHECK(psl != NULL, "psl_builtin list is compiled in");
  if (!psl) QT_END();
  QT_CHECK(psl_is_public_suffix(psl, "com"), "\"com\" is a public suffix");
  QT_CHECK(psl_is_public_suffix(psl, "co.uk"), "\"co.uk\" is a public suffix");
  QT_CHECK(!psl_is_public_suffix(psl, "example.com"), "\"example.com\" is not");
  const char *reg = psl_registrable_domain(psl, "www.foo.co.uk");
  QT_CHECK(reg && strcmp(reg, "foo.co.uk") == 0, "registrable domain of www.foo.co.uk");
  QT_CHECK(psl_suffix_count(psl) > 1000, "suffix list is populated");
  QT_CHECK(!psl_is_cookie_domain_acceptable(psl, "www.foo.co.uk", "co.uk"),
           "a cookie for co.uk is refused");
  QT_END();
}
