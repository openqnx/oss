/* fakeroot: start the faked daemon, then re-run this program with
 * libfakeroot.so preloaded and FAKEROOTKEY set (what the fakeroot wrapper
 * does). Inside, getuid() is 0, chown() to an arbitrary owner and mknod() of
 * a character device "succeed" and stat() reports them, while the real file
 * stays a plain file with its real owner. A second child sees the same faked
 * state through the daemon. The fakeroot wrapper script must preload the
 * library the runtime package ships (libfakeroot-0.so; the unversioned
 * libfakeroot.so is the -dev symlink) and is run as well when getopt(1)
 * exists. Every faked this test starts is killed (atexit), also on failure. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define LIBFAKEROOT "/usr/local/lib/libfakeroot-0.so"

/* faked daemonises: it is not our child, so it cannot be waited for. Kill it
   and poll (bounded) until it is gone, on every way out of main(). */
static int faked_pid = 0;
static void stop_faked(void) {
  if (faked_pid <= 0) return;
  kill(faked_pid, SIGTERM);
  for (int i = 0; i < 50 && kill(faked_pid, 0) == 0; i++) usleep(100000);
  if (kill(faked_pid, 0) == 0) { kill(faked_pid, SIGKILL); usleep(100000); }
  faked_pid = 0;
}

/* Child mode: argv[1] = "child", argv[2] = action, argv[3] = path. */
static int child(int argc, char **argv) {
  if (argc < 3) return 2;
  const char *a = argv[2], *p = argc > 3 ? argv[3] : "";
  struct stat st;
  if (strcmp(a, "ids") == 0) {
    printf("%d %d %d %d\n", (int)getuid(), (int)geteuid(), (int)getgid(), (int)getegid());
  } else if (strcmp(a, "chown") == 0) {
    int fd = open(p, O_CREAT | O_WRONLY, 0644); if (fd >= 0) close(fd);
    int r = chown(p, 1234, 5678);
    if (stat(p, &st) != 0) { printf("stat-failed\n"); return 1; }
    printf("%d %d %d\n", r, (int)st.st_uid, (int)st.st_gid);
  } else if (strcmp(a, "mknod") == 0) {
    int r = mknod(p, S_IFCHR | 0600, (dev_t)((4 << 10) | 2));
    if (stat(p, &st) != 0) { printf("stat-failed %d\n", r); return 1; }
    printf("%d %s\n", r, S_ISCHR(st.st_mode) ? "chr" : S_ISREG(st.st_mode) ? "reg" : "other");
  } else if (strcmp(a, "stat") == 0) {
    if (stat(p, &st) != 0) { printf("stat-failed\n"); return 1; }
    printf("%d %d %s\n", (int)st.st_uid, (int)st.st_gid, S_ISCHR(st.st_mode) ? "chr" : S_ISREG(st.st_mode) ? "reg" : "other");
  } else return 2;
  return 0;
}

static int run(const char *cmd, char *out, size_t n) {
  FILE *p = popen(cmd, "r");
  out[0] = 0;
  if (!p) return -1;
  if (!fgets(out, (int)n, p)) out[0] = 0;
  char junk[256]; while (fgets(junk, sizeof junk, p)) {}
  size_t l = strlen(out); if (l && out[l - 1] == '\n') out[l - 1] = 0;
  return pclose(p);
}

int main(int argc, char **argv) {
  if (argc > 1 && strcmp(argv[1], "child") == 0) return child(argc, argv);
  QT_BEGIN("fakeroot");
  const char *self = argv[0];
  char dir[] = "/tmp/qnxport-fakeroot.XXXXXX", cmd[2048], out[512];
  if (!mkdtemp(dir)) { QT_CHECK(0, "mkdtemp"); QT_END(); }
  printf("  info real ids: uid=%d euid=%d\n", (int)getuid(), (int)geteuid());
  QT_CHECK(access(LIBFAKEROOT, R_OK) == 0, LIBFAKEROOT " installed");

  int rc = run("/usr/local/bin/faked 2>&1", out, sizeof out);
  printf("  info faked: %s\n", out);
  char key[64] = ""; int pid = 0;
  char *colon = strchr(out, ':');
  if (rc == 0 && colon) {
    snprintf(key, sizeof key, "%.*s", (int)(colon - out), out);
    pid = atoi(colon + 1);
  }
  if (pid > 0) { faked_pid = pid; atexit(stop_faked); }
  QT_CHECK(key[0] && pid > 0, "faked started and printed key:pid");
  if (!key[0] || pid <= 0) { snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd); QT_END(); }

  char env[512];
  snprintf(env, sizeof env, "FAKEROOTKEY=%s LD_PRELOAD=" LIBFAKEROOT " FAKEROOTDONTTRYCHOWN=1", key);
#define CHILD(action, path) (snprintf(cmd, sizeof cmd, "%s '%s' child %s %s 2>&1", env, self, action, path), \
                             run(cmd, out, sizeof out))
  rc = CHILD("ids", "");
  printf("  info ids under fakeroot: %s\n", out);
  QT_CHECK(rc == 0 && strcmp(out, "0 0 0 0") == 0, "getuid/geteuid/getgid/getegid are 0 under fakeroot");

  char f1[300], f2[300];
  snprintf(f1, sizeof f1, "%s/owned", dir);
  snprintf(f2, sizeof f2, "%s/tty", dir);
  rc = CHILD("chown", f1);
  printf("  info chown under fakeroot: %s\n", out);
  QT_CHECK(rc == 0 && strcmp(out, "0 1234 5678") == 0, "chown(1234:5678) succeeds and stat reports it");
  rc = CHILD("mknod", f2);
  printf("  info mknod under fakeroot: %s\n", out);
  QT_CHECK(rc == 0 && strcmp(out, "0 chr") == 0, "mknod(S_IFCHR) succeeds and stat reports a char device");
  rc = CHILD("stat", f1);
  QT_CHECK(rc == 0 && strcmp(out, "1234 5678 reg") == 0, "second process sees the faked owner (daemon state)");
  rc = CHILD("stat", f2);
  size_t ol = strlen(out);
  QT_CHECK(rc == 0 && ol > 4 && strcmp(out + ol - 4, " chr") == 0, "second process sees the faked char device");

  struct stat st;
  QT_CHECK(stat(f1, &st) == 0 && (int)st.st_uid == (int)getuid() && st.st_uid != 1234, "real file keeps its real owner");
  QT_CHECK(stat(f2, &st) == 0 && S_ISREG(st.st_mode), "real \"device\" is a plain file");

  stop_faked();
  QT_CHECK(kill(pid, 0) != 0, "faked stopped");

  /* The wrapper script: it must name a library the runtime package has, and
     run with the target's /bin/sh. */
  rc = run("grep -c -e '^FAKEROOT_LIB=libfakeroot-0.so$' -e '^#!/bin/sh' /usr/local/bin/fakeroot", out, sizeof out);
  QT_CHECK(strcmp(out, "2") == 0, "fakeroot wrapper: #!/bin/sh, preloads libfakeroot-0.so");
  if (system("command -v getopt >/dev/null 2>&1") == 0) {
    /* The wrapper starts its own faked and stops it from its EXIT trap. */
    snprintf(cmd, sizeof cmd, "/usr/local/bin/fakeroot -- '%s' child ids 2>&1", self);
    rc = run(cmd, out, sizeof out);
    printf("  info fakeroot wrapper: %s\n", out);
    QT_CHECK(rc == 0 && strcmp(out, "0 0 0 0") == 0, "fakeroot -- <cmd>: ids are 0");
  } else printf("  info no getopt(1) on this target -- the fakeroot wrapper script not run\n");

  snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir);
  system(cmd);
  QT_END();
}
