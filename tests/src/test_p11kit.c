/* p11-kit: the PKCS#11 module loader GnuTLS and friends use to reach smart
 * cards and software token stores. */
#include "common.h"
#include <stdlib.h>
#include <p11-kit/p11-kit.h>
#include <p11-kit/uri.h>
int main(void) {
  QT_BEGIN("p11-kit");
  /* URI parsing is pure logic and must work regardless of installed modules. */
  P11KitUri *uri = p11_kit_uri_new();
  QT_CHECK(uri != NULL, "p11_kit_uri_new");
  int rc = p11_kit_uri_parse("pkcs11:token=QNX%20Test;object=mykey;type=private",
                             P11_KIT_URI_FOR_OBJECT_ON_TOKEN, uri);
  QT_CHECK(rc == P11_KIT_URI_OK, "p11_kit_uri_parse");
  if (rc == P11_KIT_URI_OK) {
    CK_TOKEN_INFO *ti = p11_kit_uri_get_token_info(uri);
    QT_CHECK(ti && memcmp(ti->label, "QNX Test", 8) == 0,
             "token label decoded from percent-encoding");
    CK_ATTRIBUTE *attr = p11_kit_uri_get_attribute(uri, CKA_LABEL);
    QT_CHECK(attr && attr->ulValueLen == 5 && memcmp(attr->pValue, "mykey", 5) == 0,
             "object label attribute");
  }
  char *formatted = NULL;
  QT_CHECK(p11_kit_uri_format(uri, P11_KIT_URI_FOR_OBJECT_ON_TOKEN, &formatted)
           == P11_KIT_URI_OK && formatted, "p11_kit_uri_format");
  if (formatted) { printf("  info %s\n", formatted); free(formatted); }
  p11_kit_uri_free(uri);
  QT_CHECK(p11_kit_uri_parse("not-a-pkcs11-uri", P11_KIT_URI_FOR_ANY,
                             (uri = p11_kit_uri_new())) != P11_KIT_URI_OK,
           "a malformed URI is rejected");
  p11_kit_uri_free(uri);

  /* Module enumeration: an empty list is a legitimate result on a bare target,
   * but the call itself must succeed. */
  CK_FUNCTION_LIST **modules = p11_kit_modules_load_and_initialize(0);
  if (modules) {
    int n = 0;
    while (modules[n]) ++n;
    printf("  info %d PKCS#11 module(s) configured\n", n);
    p11_kit_modules_finalize_and_release(modules);
    QT_CHECK(1, "p11_kit_modules_load_and_initialize");
  } else {
    printf("  info no modules configured: %s\n", p11_kit_message());
    QT_CHECK(p11_kit_message() != NULL, "the empty case is reported cleanly");
  }
  QT_END();
}
