/* skalibs (libskarnet): number formatting and scanning, stralloc, the
 * SHA-256 of "abc" (FIPS 180-2 known answer), big-endian packing, IPv4
 * text form, a cdb written with cdbmake and read back (cdb_find),
 * openwritenclose/openslurpclose, the selfpipe catching a signal sent to
 * ourselves and seen through iopause (with a deadline from the TAI clock),
 * and child_spawn0 + wait_pid with a known exit status. */
#include "common.h"
#include <skalibs/stddjb.h>
#include <skalibs/sha256.h>
#include <skalibs/cdb.h>
#include <skalibs/cdbmake.h>
#include <skalibs/selfpipe.h>
#include <skalibs/iopause.h>
#include <skalibs/cspawn.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

int main(void) {
  QT_BEGIN("skalibs");
  char buf[256]; size_t n;

  n = ulong_fmt(buf, 4294967296UL); buf[n] = 0;
  QT_CHECK(!strcmp(buf, "4294967296"), "ulong_fmt(2^32) = 4294967296");
  n = ulong_xfmt(buf, 0xdeadbeefUL); buf[n] = 0;
  QT_CHECK(!strcmp(buf, "deadbeef"), "ulong_xfmt(0xdeadbeef)");
  unsigned long u = 0;
  QT_CHECK(ulong_scan("123456789xyz", &u) == 9 && u == 123456789UL, "ulong_scan stops after 9 digits");
  uint32_pack_big(buf, 0x01020304U);
  QT_CHECK(buf[0] == 1 && buf[1] == 2 && buf[2] == 3 && buf[3] == 4, "uint32_pack_big byte order");
  uint32_t x; uint32_unpack(buf, &x);
  QT_CHECK(x == 0x04030201U, "uint32_unpack (little endian) of 01 02 03 04");

  char ip[4];
  QT_CHECK(ip4_scan("192.168.1.20", ip) == 12, "ip4_scan");
  n = ip4_fmt(buf, ip); buf[n] = 0;
  QT_CHECK(!strcmp(buf, "192.168.1.20"), "ip4_fmt round trip");

  stralloc sa = STRALLOC_ZERO;
  QT_CHECK(stralloc_cats(&sa, "QNX ") && stralloc_cats(&sa, "SDP 8.0") && stralloc_0(&sa), "stralloc_cats");
  QT_CHECK(!strcmp(sa.s, "QNX SDP 8.0") && sa.len == 12, "stralloc contents and length");
  stralloc_free(&sa);

  SHA256Schedule ctx = SHA256_INIT();
  char dig[32];
  sha256_update(&ctx, "abc", 3); sha256_final(&ctx, dig);
  for (int i = 0; i < 32; i++) snprintf(buf + 2 * i, 3, "%02x", (unsigned char)dig[i]);
  QT_CHECK(!strcmp(buf, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), "SHA-256(\"abc\") known answer");

  char dir[64]; snprintf(dir, sizeof dir, "/tmp/qnxport-skalibs.%d", (int)getpid());
  QT_CHECK(mkdir(dir, 0700) == 0, "scratch directory");
  char path[128]; snprintf(path, sizeof path, "%s/test.cdb", dir);
  int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
  cdbmaker cm = CDBMAKER_ZERO;
  int ok = fd >= 0 && cdbmake_start(&cm, fd);
  ok = ok && cdbmake_add(&cm, "arch", 4, "aarch64le", 9) && cdbmake_add(&cm, "os", 2, "QNX", 3)
          && cdbmake_add(&cm, "arch", 4, "x86_64", 6) && cdbmake_finish(&cm);
  if (fd >= 0) close(fd);
  QT_CHECK(ok, "cdbmake: 3 records written");
  cdb c = CDB_ZERO; cdb_data d;
  QT_CHECK(cdb_init(&c, path), "cdb_init (mapped)");
  QT_CHECK(cdb_find(&c, &d, "os", 2) == 1 && d.len == 3 && !memcmp(d.s, "QNX", 3), "cdb_find(os) = QNX");
  cdb_find_state st = CDB_FIND_STATE_ZERO; int hits = 0;
  while (cdb_findnext(&c, &d, "arch", 4, &st) == 1) hits++;
  QT_CHECK(hits == 2, "cdb_findnext: two values for key arch");
  QT_CHECK(cdb_find(&c, &d, "nope", 4) == 0, "cdb_find(missing) = 0");
  cdb_free(&c);
  unlink(path);

  snprintf(path, sizeof path, "%s/file", dir);
  QT_CHECK(openwritenclose_unsafe(path, "hello skalibs\n", 14), "openwritenclose_unsafe");
  QT_CHECK(openslurpclose(&sa, path) && sa.len == 14 && !memcmp(sa.s, "hello skalibs\n", 14), "openslurpclose reads it back");
  stralloc_free(&sa);
  unlink(path); rmdir(dir);

  tain_now_set_stopwatch_g();
  int sfd = selfpipe_init();
  QT_CHECK(sfd >= 0, "selfpipe_init");
  QT_CHECK(selfpipe_trap(SIGUSR1), "selfpipe_trap(SIGUSR1)");
  kill(getpid(), SIGUSR1);
  iopause_fd fds = { .fd = sfd, .events = IOPAUSE_READ };
  tain deadline; tain_addsec_g(&deadline, 5);
  int r = iopause_g(&fds, 1, &deadline);
  QT_CHECK(r == 1 && (fds.revents & IOPAUSE_READ), "iopause: selfpipe readable");
  QT_CHECK(selfpipe_read() == SIGUSR1, "selfpipe_read() = SIGUSR1");
  /* a deadline 200 ms ahead with nothing to read: iopause times out */
  { tain t; tain_from_millisecs(&t, 200); tain_add_g(&deadline, &t); }
  tain before = STAMP;
  r = iopause_g(&fds, 1, &deadline);
  tain el; tain_sub(&el, &STAMP, &before);
  long ms = (long)(tai_sec(tain_secp(&el)) * 1000 + el.nano / 1000000);
  printf("  info iopause timeout after %ld ms\n", ms);
  QT_CHECK(r == 0 && ms >= 150 && ms < 2000, "iopause: times out at its deadline (~200 ms)");
  selfpipe_finish();

  char const *argv[] = { "/bin/sh", "-c", "exit 5", 0 };
  pid_t pid = child_spawn0(argv[0], argv, (char const *const *)environ);
  QT_CHECK(pid > 0, "child_spawn0");
  int wst = 0;
  QT_CHECK(pid > 0 && waitpid_nointr(pid, &wst, 0) == pid && WIFEXITED(wst) && WEXITSTATUS(wst) == 5, "child exit status 5");
  QT_END();
}
