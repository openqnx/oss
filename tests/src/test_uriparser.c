/* uriparser: split a URI into its parts. */
#include "common.h"
#include <uriparser/Uri.h>
int main(void) {
  QT_BEGIN("uriparser");
  UriUriA u; const char *err;
  QT_CHECK(uriParseSingleUriA(&u, "https://repo.example.com:8443/8.0/x86_64/INDEX?x=1", &err) == URI_SUCCESS, "parses");
  QT_CHECK(u.hostText.afterLast - u.hostText.first == 16 && !strncmp(u.hostText.first, "repo.example.com", 16), "host");
  QT_CHECK(!strncmp(u.portText.first, "8443", 4), "port");
  uriFreeUriMembersA(&u);
  QT_END();
}
