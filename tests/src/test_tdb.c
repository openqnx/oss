/* tdb: a database file in /tmp used the way Samba uses its tdbs. Checks:
 * store/fetch/exists/delete, TDB_INSERT refusing an existing key,
 * tdb_append, a traversal counting every record, a cancelled transaction
 * leaving nothing behind and a committed one surviving a reopen, the
 * record lock held by a forked child (tdb_reopen, tdb_chainlock) making the parent's
 * tdb_chainlock_nonblock fail until the child unlocks, a write by the
 * child visible to the parent, tdb_check passing, and the tools: tdbdump
 * printing the records, tdbtool "keys"/"show", tdbbackup making a copy
 * tdbdump reads the same. Reports whether robust-mutex locking is
 * available (tdb_runtime_check_for_robust_mutexes). */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <tdb.h>

static TDB_DATA S(const char *s) { TDB_DATA d = { (unsigned char *)s, strlen(s) }; return d; }
static int eq(TDB_DATA d, const char *s) { int r = d.dptr && d.dsize == strlen(s) && !memcmp(d.dptr, s, d.dsize); free(d.dptr); return r; }
static int count_fn(struct tdb_context *t, TDB_DATA k, TDB_DATA v, void *p) { (void)t; (void)k; (void)v; (*(int *)p)++; return 0; }
static char out[16384];
static const char *run(const char *cmd) {
  FILE *p = popen(cmd, "r"); size_t n = p ? fread(out, 1, sizeof out - 1, p) : 0; out[n] = 0; if (p) pclose(p); return out;
}
int main(void) {
  QT_BEGIN("tdb");
  char path[96], bak[112], cmd[300]; snprintf(path, sizeof path, "/tmp/qnxport-tdb-%d.tdb", (int)getpid());
  snprintf(bak, sizeof bak, "%s.bak", path);
  unlink(path); unlink(bak);
  printf("  info robust mutexes for TDB_MUTEX_LOCKING: %s\n", tdb_runtime_check_for_robust_mutexes() ? "yes" : "no");
  struct tdb_context *t = tdb_open(path, 0, TDB_DEFAULT, O_RDWR | O_CREAT, 0600);
  QT_CHECK(t != NULL, "tdb_open creates the database");
  if (!t) QT_END();
  QT_CHECK(tdb_store(t, S("greeting"), S("hello qnx"), TDB_REPLACE) == 0 && eq(tdb_fetch(t, S("greeting")), "hello qnx"), "store/fetch");
  QT_CHECK(tdb_store(t, S("greeting"), S("again"), TDB_INSERT) == -1 && tdb_error(t) == TDB_ERR_EXISTS, "TDB_INSERT on an existing key: TDB_ERR_EXISTS");
  tdb_store(t, S("log"), S("a"), TDB_REPLACE); tdb_append(t, S("log"), S("bc"));
  QT_CHECK(eq(tdb_fetch(t, S("log")), "abc"), "tdb_append: a + bc = abc");
  for (int i = 0; i < 100; i++) { char k[16], v[16]; snprintf(k, sizeof k, "k%03d", i); snprintf(v, sizeof v, "v%d", i * i); tdb_store(t, S(k), S(v), TDB_REPLACE); }
  int n = 0; int tr = tdb_traverse(t, count_fn, &n);
  QT_CHECK(tr == 102 && n == 102, "traverse: 102 records");
  QT_CHECK(tdb_delete(t, S("k050")) == 0 && !tdb_exists(t, S("k050")) && eq(tdb_fetch(t, S("k051")), "v2601"), "delete one, the others intact");
  tdb_transaction_start(t); tdb_store(t, S("tx"), S("cancelled"), TDB_REPLACE); tdb_transaction_cancel(t);
  QT_CHECK(!tdb_exists(t, S("tx")), "a cancelled transaction leaves nothing");
  QT_CHECK(tdb_transaction_start(t) == 0 && tdb_store(t, S("tx"), S("committed"), TDB_REPLACE) == 0 && tdb_transaction_commit(t) == 0,
           "transaction start/store/commit");

  /* another process: holds a record lock, then writes */
  int p2c[2], c2p[2]; if (pipe(p2c) || pipe(c2p)) return 1;
  pid_t c = fork();
  if (c == 0) {
    /* after fork() a tdb is reopened (tdb_reopen), as Samba's daemons do:
       a second tdb_open of the same file in the copied process is refused */
    struct tdb_context *ct = t; char b = 0;
    close(p2c[1]); close(c2p[0]);
    if (tdb_reopen(ct) != 0 || tdb_chainlock(ct, S("greeting")) != 0) { (void)!write(c2p[1], "E", 1); _exit(1); }
    (void)!write(c2p[1], "L", 1);            /* locked */
    (void)!read(p2c[0], &b, 1);
    tdb_store(ct, S("greeting"), S("from the child"), TDB_REPLACE);
    tdb_chainunlock(ct, S("greeting"));
    (void)!write(c2p[1], "U", 1);            /* unlocked */
    tdb_close(ct); _exit(0);
  }
  close(p2c[0]); close(c2p[1]);
  char b = 0; (void)!read(c2p[0], &b, 1);
  int blocked = b == 'L' && tdb_chainlock_nonblock(t, S("greeting")) == -1;
  QT_CHECK(blocked, "a record locked by another process: tdb_chainlock_nonblock fails");
  (void)!write(p2c[1], "g", 1); (void)!read(c2p[0], &b, 1);
  int st = 0; waitpid(c, &st, 0);
  int relock = tdb_chainlock_nonblock(t, S("greeting")) == 0; if (relock) tdb_chainunlock(t, S("greeting"));
  QT_CHECK(b == 'U' && WIFEXITED(st) && WEXITSTATUS(st) == 0 && relock, "after the child unlocks, the lock is ours");
  QT_CHECK(eq(tdb_fetch(t, S("greeting")), "from the child"), "the child's write is seen by the parent");
  QT_CHECK(tdb_check(t, NULL, NULL) == 0, "tdb_check: consistent");
  tdb_close(t);
  t = tdb_open(path, 0, TDB_DEFAULT, O_RDWR, 0600);
  QT_CHECK(t && eq(tdb_fetch(t, S("tx")), "committed") && eq(tdb_fetch(t, S("k099")), "v9801"), "reopened: committed data persisted");
  if (t) tdb_close(t);

  snprintf(cmd, sizeof cmd, "tdbdump %s 2>&1", path); run(cmd);
  QT_CHECK(strstr(out, "key(8) = \"greeting\"") && strstr(out, "data(14) = \"from the child\""), "tdbdump prints the records");
  snprintf(cmd, sizeof cmd, "tdbtool %s show tx 2>&1", path); run(cmd);
  /* a hex dump: "data 9 bytes", then 63 6F 6D 6D 69 74 74 65 64 = "committed" */
  QT_CHECK(strstr(out, "data 9 bytes") && strstr(out, "63 6F 6D 6D 69 74 74 65  64"), "tdbtool <db> show tx: 9 bytes, hex of \"committed\"");
  if (!strstr(out, "data 9 bytes")) printf("  info tdbtool: %s\n", out);
  snprintf(cmd, sizeof cmd, "tdbbackup -s .bak %s 2>&1; tdbdump %s.bak 2>&1 | grep -c 'key('", path, path); run(cmd);
  QT_CHECK(atoi(out) == 102, "tdbbackup: the copy has all 102 records");
  printf("  info tdbbackup/tdbdump count: %s", out);
  unlink(path); unlink(bak);
  QT_END();
}
