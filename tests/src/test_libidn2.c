/* libidn2: IDNA2008 lookup and reverse conversion. */
#include "common.h"
#include <stdlib.h>
#include <idn2.h>
int main(void) {
  QT_BEGIN("libidn2");
  printf("  info libidn2 %s\n", idn2_check_version(NULL));
  char *ace = NULL;
  int rc = idn2_to_ascii_8z("bücher.example", &ace, IDN2_NONTRANSITIONAL);
  QT_CHECK(rc == IDN2_OK, "idn2_to_ascii_8z");
  QT_CHECK(ace && strcmp(ace, "xn--bcher-kva.example") == 0,
           "bücher.example -> xn--bcher-kva.example");
  char *back = NULL;
  rc = idn2_to_unicode_8z8z(ace ? ace : "", &back, 0);
  QT_CHECK(rc == IDN2_OK && back && strcmp(back, "bücher.example") == 0,
           "reverse conversion restores the U-label");
  idn2_free(ace); idn2_free(back);
  char *bad = NULL;
  /* A label with a leading hyphen-minus pair is invalid under IDNA2008. */
  rc = idn2_lookup_u8((const uint8_t *) "xn--", (uint8_t **)&bad, IDN2_NO_TR46);
  QT_CHECK(rc != IDN2_OK, "invalid A-label is rejected");
  if (bad) idn2_free(bad);
  QT_CHECK(idn2_strerror(IDN2_OK) != NULL, "idn2_strerror");
  QT_END();
}
