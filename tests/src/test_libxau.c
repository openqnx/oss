/* libXau: write an Xauthority file with two entries (MIT-MAGIC-COOKIE-1 for
 * a local display and one for a TCP host), read it back entry by entry,
 * look entries up by address and display number through XAUTHORITY, and
 * check the lock/unlock protocol on the file. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <X11/X.h>
#include <X11/Xauth.h>
static Xauth mk(unsigned short fam, const char *addr, const char *num, const char *name, const char *data, int dl) {
  Xauth a;
  a.family = fam;
  a.address = (char *)addr; a.address_length = (unsigned short)strlen(addr);
  a.number = (char *)num; a.number_length = (unsigned short)strlen(num);
  a.name = (char *)name; a.name_length = (unsigned short)strlen(name);
  a.data = (char *)data; a.data_length = (unsigned short)dl;
  return a;
}
static int same(const Xauth *a, const Xauth *b) {
  return a->family == b->family && a->address_length == b->address_length && !memcmp(a->address, b->address, a->address_length) &&
         a->number_length == b->number_length && !memcmp(a->number, b->number, a->number_length) &&
         a->name_length == b->name_length && !memcmp(a->name, b->name, a->name_length) &&
         a->data_length == b->data_length && !memcmp(a->data, b->data, a->data_length);
}
int main(void) {
  QT_BEGIN("libxau");
  char path[64]; snprintf(path, sizeof path, "/tmp/qnxport-xauth-%d", (int)getpid());
  static const char cookie1[16] = { 0x01, 0x23, 0x45, 0x67, (char)0x89, (char)0xab, (char)0xcd, (char)0xef,
                                    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
  static const char cookie2[16] = "QNX-cookie-0002";
  static const char ip[4] = { 127, 0, 0, 1 };
  Xauth e1 = mk(FamilyLocal, "qnxhost", "7", "MIT-MAGIC-COOKIE-1", cookie1, 16);
  Xauth e2 = mk(FamilyInternet, "", "12", "MIT-MAGIC-COOKIE-1", cookie2, 16);
  e2.address = (char *)ip; e2.address_length = 4;

  QT_CHECK(XauLockAuth(path, 3, 1, 0) == LOCK_SUCCESS, "XauLockAuth takes the lock");
  FILE *f = fopen(path, "wb");
  QT_CHECK(f != NULL, "create auth file in /tmp");
  if (!f) QT_END();
  QT_CHECK(XauWriteAuth(f, &e1) == 1 && XauWriteAuth(f, &e2) == 1, "XauWriteAuth two entries");
  fclose(f);
  XauUnlockAuth(path);
  char lk[80]; snprintf(lk, sizeof lk, "%s-c", path);
  struct stat st;
  QT_CHECK(stat(lk, &st) != 0, "lock file removed by XauUnlockAuth");
  QT_CHECK(stat(path, &st) == 0 && st.st_size == (2 + 2 + 7 + 2 + 1 + 2 + 18 + 2 + 16) + (2 + 2 + 4 + 2 + 2 + 2 + 18 + 2 + 16),
           "file size matches the Xauthority wire format");

  f = fopen(path, "rb");
  Xauth *r1 = f ? XauReadAuth(f) : NULL, *r2 = f ? XauReadAuth(f) : NULL, *r3 = f ? XauReadAuth(f) : NULL;
  if (f) fclose(f);
  QT_CHECK(r1 && same(r1, &e1), "first entry reads back identical");
  QT_CHECK(r2 && same(r2, &e2), "second entry reads back identical");
  QT_CHECK(r3 == NULL, "end of file after two entries");
  if (r1) XauDisposeAuth(r1);
  if (r2) XauDisposeAuth(r2);

  setenv("XAUTHORITY", path, 1);
  const char *fn = XauFileName();
  printf("  info XauFileName = %s\n", fn ? fn : "(null)");
  QT_CHECK(fn && strcmp(fn, path) == 0, "XauFileName honours XAUTHORITY");
  Xauth *g = XauGetAuthByAddr(FamilyInternet, 4, ip, 2, "12", 18, "MIT-MAGIC-COOKIE-1");
  QT_CHECK(g && g->data_length == 16 && memcmp(g->data, cookie2, 16) == 0, "XauGetAuthByAddr finds the TCP entry");
  if (g) XauDisposeAuth(g);
  g = XauGetAuthByAddr(FamilyLocal, 7, "qnxhost", 1, "7", 0, "");
  QT_CHECK(g && g->data_length == 16 && memcmp(g->data, cookie1, 16) == 0, "lookup by local host and display 7");
  if (g) XauDisposeAuth(g);
  g = XauGetAuthByAddr(FamilyLocal, 7, "qnxhost", 1, "8", 0, "");
  QT_CHECK(g == NULL, "no entry for display 8");
  if (g) XauDisposeAuth(g);
  unlink(path);
  QT_END();
}
