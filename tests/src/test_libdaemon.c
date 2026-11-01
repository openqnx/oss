/* libdaemon: a real daemon round trip. daemon_fork() detaches a child
 * (double fork, setsid); the child writes its PID file, installs a signal
 * pipe for SIGUSR1/SIGTERM and reports "ready" (42) back through the
 * retval pipe; the parent waits for that value, finds the daemon through
 * the PID file, sends SIGUSR1 (the daemon answers by writing a file) and
 * stops it with daemon_pid_file_kill_wait(SIGTERM). Also daemon_exec()
 * (a child's exit status and its output through the log) and
 * daemon_nonblock(). The daemon is killed on every exit path. */
#include "common.h"
#include <libdaemon/daemon.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static char dir[64], pidfile[128], usr1file[128], sidfile[128];
static pid_t daemon_pid = -1, test_sid = -1;
static const char *pidproc(void) { return pidfile; }

static void cleanup(void) {
  if (daemon_pid > 0 && kill(daemon_pid, 0) == 0) { kill(daemon_pid, SIGKILL); }
  unlink(pidfile); unlink(usr1file); unlink(sidfile); rmdir(dir);
}

static int exists(const char *p) { struct stat st; return stat(p, &st) == 0; }

static void run_daemon(void) {
  /* in the detached child */
  if (daemon_close_all(-1) < 0) { daemon_retval_send(1); _exit(1); }
  if (daemon_pid_file_create() < 0) { daemon_retval_send(2); _exit(1); }
  if (getsid(0) != test_sid) { int f = open(sidfile, O_CREAT | O_WRONLY, 0644); if (f >= 0) close(f); }
  if (daemon_signal_init(SIGUSR1, SIGTERM, 0) < 0) { daemon_retval_send(3); goto out; }
  daemon_retval_send(42);
  int fd = daemon_signal_fd();
  for (int i = 0; i < 300; i++) {          /* at most ~30 s */
    fd_set fds; FD_ZERO(&fds); FD_SET(fd, &fds);
    struct timeval tv = { 0, 100000 };
    int r = select(fd + 1, &fds, NULL, NULL, &tv);
    if (r < 0 && errno != EINTR) break;
    if (r <= 0) continue;
    int sig = daemon_signal_next();
    if (sig == SIGUSR1) { int f = open(usr1file, O_CREAT | O_WRONLY, 0644); if (f >= 0) { (void)write(f, "usr1\n", 5); close(f); } }
    else if (sig == SIGTERM) break;
  }
  daemon_signal_done();
out:
  daemon_pid_file_remove();
  _exit(0);
}

int main(void) {
  QT_BEGIN("libdaemon");
  snprintf(dir, sizeof dir, "/tmp/qnxport-libdaemon.%d", (int)getpid());
  QT_CHECK(mkdir(dir, 0700) == 0, "scratch directory");
  snprintf(pidfile, sizeof pidfile, "%s/testd.pid", dir);
  snprintf(usr1file, sizeof usr1file, "%s/usr1", dir);
  snprintf(sidfile, sizeof sidfile, "%s/sid", dir);
  atexit(cleanup);
  daemon_pid_file_proc = pidproc;
  daemon_log_ident = "qnxport-libdaemon";
  daemon_log_use = DAEMON_LOG_STDERR;

  QT_CHECK(daemon_pid_file_is_running() < 0, "no daemon running before the test");
  test_sid = getsid(0);
  QT_CHECK(daemon_retval_init() == 0, "daemon_retval_init");
  pid_t p = daemon_fork();
  if (p == 0) run_daemon();
  printf("  info daemon_fork returned %d\n", (int)p);
  QT_CHECK(p > 0, "daemon_fork returns the daemon's PID in the parent");
  int rv = daemon_retval_wait(10);
  printf("  info daemon_retval_wait: %d\n", rv);
  QT_CHECK(rv == 42, "daemon reported ready (retval 42)");
  daemon_pid = daemon_pid_file_is_running();
  QT_CHECK(daemon_pid > 0 && daemon_pid == p, "PID file names the daemon daemon_fork reported");
  QT_CHECK(exists(sidfile), "the daemon runs in a session of its own (setsid)");

  QT_CHECK(daemon_pid_file_kill(SIGUSR1) == 0, "daemon_pid_file_kill(SIGUSR1)");
  int seen = 0;
  for (int i = 0; i < 50 && !seen; i++) { seen = exists(usr1file); if (!seen) usleep(100000); }
  QT_CHECK(seen, "daemon handled SIGUSR1 through its signal pipe");
  QT_CHECK(daemon_pid_file_kill_wait(SIGTERM, 10) == 0, "daemon_pid_file_kill_wait(SIGTERM)");
  QT_CHECK(daemon_pid > 0 && kill(daemon_pid, 0) < 0, "daemon process gone");
  QT_CHECK(!exists(pidfile), "daemon removed its PID file");
  daemon_retval_done();

  /* daemon_exec() needs the signal pipe for SIGCHLD */
  QT_CHECK(daemon_signal_init(SIGCHLD, 0) == 0, "daemon_signal_init(SIGCHLD)");
  int ret = -1;
  int r = daemon_exec("/", &ret, "/bin/sh", "sh", "-c", "echo exec-output; exit 7", (char *)NULL);
  QT_CHECK(r == 0 && ret == 7, "daemon_exec: child exit status 7");
  daemon_signal_done();

  int fds[2];
  QT_CHECK(pipe(fds) == 0, "pipe");
  QT_CHECK(daemon_nonblock(fds[0], 1) == 0 && (fcntl(fds[0], F_GETFL) & O_NONBLOCK), "daemon_nonblock(1) sets O_NONBLOCK");
  QT_CHECK(daemon_nonblock(fds[0], 0) == 0 && !(fcntl(fds[0], F_GETFL) & O_NONBLOCK), "daemon_nonblock(0) clears it");
  close(fds[0]); close(fds[1]);
  QT_END();
}
