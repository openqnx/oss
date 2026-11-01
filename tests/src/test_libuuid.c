/* libuuid (util-linux): parse/unparse of a known UUID and its version and
 * variant, name-based v3 (MD5) and v5 (SHA-1) UUIDs of www.example.com in
 * the DNS namespace against Python's uuid module, random (v4) UUIDs (1000,
 * all distinct), time-based v1 UUIDs whose embedded time is now and which
 * sort in creation order, v7 (Unix-time) UUIDs, compare/copy/clear. */
#include "common.h"
#include <uuid/uuid.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

static int cmpu(const void *a, const void *b) { return uuid_compare(*(const uuid_t *)a, *(const uuid_t *)b); }

int main(void) {
  uuid_t u, v; char s[37]; int i, ok;
  static uuid_t many[1000];
  QT_BEGIN("libuuid");
  QT_CHECK(uuid_parse("6ba7b810-9dad-11d1-80b4-00c04fd430c8", u) == 0, "uuid_parse (the RFC 4122 DNS namespace)");
  QT_CHECK(uuid_type(u) == UUID_TYPE_DCE_TIME && uuid_variant(u) == UUID_VARIANT_DCE, "it is a version 1, DCE variant UUID");
  uuid_unparse_upper(u, s);
  QT_CHECK(strcmp(s, "6BA7B810-9DAD-11D1-80B4-00C04FD430C8") == 0, "uuid_unparse_upper");
  QT_CHECK(uuid_parse("6ba7b810-9dad-11d1-80b4-00c04fd430c", v) != 0 && uuid_parse("6ba7b810x9dad-11d1-80b4-00c04fd430c8", v) != 0,
           "uuid_parse rejects a short or malformed string");
  QT_CHECK(uuid_get_template("dns") != NULL && uuid_compare(*uuid_get_template("dns"), u) == 0,
           "uuid_get_template(\"dns\") is the DNS namespace parsed above");
  if (!uuid_get_template("dns")) QT_END();
  uuid_generate_md5(v, *uuid_get_template("dns"), "www.example.com", 15); uuid_unparse(v, s);
  QT_CHECK(strcmp(s, "5df41881-3aed-3515-88a7-2f4a814cf09e") == 0 && uuid_type(v) == 3, "uuid_generate_md5(dns, www.example.com) = Python uuid3");
  uuid_generate_sha1(v, *uuid_get_template("dns"), "www.example.com", 15); uuid_unparse(v, s);
  QT_CHECK(strcmp(s, "2ed6657d-e927-568b-95e1-2665a8aea6a2") == 0 && uuid_type(v) == 5, "uuid_generate_sha1(dns, www.example.com) = Python uuid5");
  ok = 1;
  for (i = 0; i < 1000; i++) { uuid_generate_random(many[i]); ok &= uuid_type(many[i]) == 4 && uuid_variant(many[i]) == UUID_VARIANT_DCE; }
  qsort(many, 1000, sizeof(uuid_t), cmpu);
  for (i = 1; i < 1000; i++) ok &= uuid_compare(many[i - 1], many[i]) != 0;
  QT_CHECK(ok, "1000 uuid_generate_random(): all version 4, all distinct");
  {
    struct timeval tv; time_t now = time(NULL); uuid_t t1, t2;
    uuid_generate_time(t1); uuid_generate_time(t2);
    QT_CHECK(uuid_type(t1) == 1 && uuid_time(t1, &tv) >= now - 5 && tv.tv_sec <= now + 5, "uuid_generate_time: version 1, embedded time is now");
    {
      struct timeval a, b; uuid_time(t1, &a); uuid_time(t2, &b);
      QT_CHECK(a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_usec <= b.tv_usec), "a second v1 UUID is not older");
    }
    printf("  info uuid_generate_time_safe: %d (0 = generated safely)\n", uuid_generate_time_safe(t2));
    uuid_generate_time_v7(t1);
    QT_CHECK(uuid_type(t1) == 7 && uuid_time(t1, &tv) >= now - 5 && tv.tv_sec <= now + 5, "uuid_generate_time_v7: version 7, time is now");
  }
  uuid_copy(v, u);
  QT_CHECK(uuid_compare(u, v) == 0 && !uuid_is_null(v), "uuid_copy + uuid_compare");
  uuid_clear(v);
  QT_CHECK(uuid_is_null(v), "uuid_clear -> uuid_is_null");
  QT_END();
}
