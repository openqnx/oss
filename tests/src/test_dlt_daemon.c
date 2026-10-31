/* dlt-daemon end to end on loopback: start dlt-daemon with a private config
 * on a private TCP port, attach dlt-receive to it writing a DLT file, log
 * five verbose messages ("qnx-dlt-msg" <i>) from this process through libdlt
 * (app QNXT, context TCTX), then read the received file back with the libdlt
 * file API and with dlt-convert, and check every message arrived with its
 * app/context IDs and payload. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(ctx)

#define NMSG 5
static char dir[] = "/tmp/qnxport-dlt.XXXXXX";

/* Every process this test starts is remembered here, and killed and reaped
   on every exit path: normal return, QT_END on an early failure (atexit),
   a fatal signal, or the overall time limit (alarm). */
static volatile pid_t children[2] = { -1, -1 };
static void forget(pid_t p) {
  for (int i = 0; i < 2; i++) if (children[i] == p) children[i] = -1;
}
static void kill_children(void) {  /* async-signal-safe */
  for (int i = 0; i < 2; i++) {
    pid_t p = children[i];
    if (p > 0) { kill(p, SIGKILL); waitpid(p, NULL, 0); children[i] = -1; }
  }
}
static void on_fatal(int sig) {
  static const char m[] = "  FAIL killed by a signal or the time limit; stopping the daemons\n";
  kill_children();
  if (write(1, m, sizeof m - 1) < 0) {}
  _exit(128 + sig);
}
static pid_t spawn(const char *log, char *const argv[]) {
  pid_t p = fork();
  if (p == 0) {
    int fd = open(log, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); close(fd); }
    execvp(argv[0], argv);
    _exit(127);
  }
  for (int i = 0; i < 2 && p > 0; i++) if (children[i] <= 0) { children[i] = p; break; }
  return p;
}
static void stop(pid_t p, const char *name) {
  if (p <= 0) return;
  kill(p, SIGTERM);
  for (int i = 0; i < 50; i++) {
    int st;
    if (waitpid(p, &st, WNOHANG) == p) { forget(p); return; }
    usleep(100000);
  }
  printf("  info %s did not stop on SIGTERM, killing\n", name);
  kill(p, SIGKILL);
  waitpid(p, NULL, 0);
  forget(p);
}
static int port_open(int port) {
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) return 0;
  struct sockaddr_in a;
  memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int ok = connect(s, (struct sockaddr *)&a, sizeof a) == 0;
  close(s);
  return ok;
}
static void show(const char *path) {
  char line[256];
  FILE *f = fopen(path, "r");
  if (!f) return;
  for (int n = 0; n < 12 && fgets(line, sizeof line, f); n++) printf("  info | %s", line);
  fclose(f);
}
static int run(const char *cmd, char *out, size_t n) {
  out[0] = 0;
  FILE *p = popen(cmd, "r");
  if (!p) return -1;
  size_t len = fread(out, 1, n - 1, p);
  out[len] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main(void) {
  QT_BEGIN("dlt-daemon");
  signal(SIGPIPE, SIG_IGN);
  atexit(kill_children);
  signal(SIGINT, on_fatal); signal(SIGTERM, on_fatal); signal(SIGHUP, on_fatal);
  signal(SIGSEGV, on_fatal); signal(SIGABRT, on_fatal); signal(SIGALRM, on_fatal);
  alarm(120);  /* the whole test takes about 10 s; never leave a daemon behind */
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char conf[256], dlog[256], rlog[256], rx[256], cmd[512], portstr[16];
  static char out[65536];
  int port = 13490 + getpid() % 200;
  snprintf(portstr, sizeof portstr, "%d", port);
  snprintf(conf, sizeof conf, "%s/dlt.conf", dir);
  snprintf(dlog, sizeof dlog, "%s/daemon.log", dir);
  snprintf(rlog, sizeof rlog, "%s/receive.log", dir);
  snprintf(rx, sizeof rx, "%s/rx.dlt", dir);
  FILE *f = fopen(conf, "w");
  if (f) {
    fprintf(f, "SendContextRegistration = 1\nECUId = QNX1\nSharedMemorySize = 100000\n"
               "LoggingMode = 0\nLoggingLevel = 6\nLoggingFilename = %s/internal.log\n"
               "TimeOutOnSend = 4\nRingbufferMinSize = 500000\nRingbufferMaxSize = 10000000\n"
               "RingbufferStepSize = 500000\nControlSocketPath = %s/dlt-ctrl.sock\n", dir, dir);
    fclose(f);
  }
  struct stat sb;
  if (stat("/tmp/dlt", &sb) == 0) printf("  info /tmp/dlt (the application socket) already exists\n");

  char *dargv[] = { "dlt-daemon", "-c", conf, "-p", portstr, NULL };
  pid_t dpid = spawn(dlog, dargv);
  int up = 0;
  for (int i = 0; i < 100 && !up; i++) {
    int st;
    if (waitpid(dpid, &st, WNOHANG) == dpid) { forget(dpid); dpid = -1; break; }
    up = port_open(port) && stat("/tmp/dlt", &sb) == 0;
    if (!up) usleep(100000);
  }
  QT_CHECK(up, "dlt-daemon listens on 127.0.0.1 and created /tmp/dlt");
  if (!up) { show(dlog); stop(dpid, "dlt-daemon"); QT_END(); }
  printf("  info dlt-daemon pid %d on port %d\n", (int)dpid, port);

  char *rargv[] = { "dlt-receive", "-p", portstr, "-o", rx, "127.0.0.1", NULL };
  pid_t rpid = spawn(rlog, rargv);
  usleep(1500000); /* let dlt-receive connect before anything is logged */

  DLT_REGISTER_APP("QNXT", "QNX run-level test");
  DLT_REGISTER_CONTEXT(ctx, "TCTX", "test context");
  for (int i = 0; i < NMSG; i++) {
    DLT_LOG(ctx, DLT_LOG_INFO, DLT_STRING("qnx-dlt-msg"), DLT_INT32(i));
    usleep(50000);
  }
  QT_CHECK(1, "five messages logged through libdlt");

  /* Wait until all five reached the file (or give up after 10 s). */
  int seen = 0;
  for (int i = 0; i < 100 && seen < NMSG; i++) {
    usleep(100000);
    snprintf(cmd, sizeof cmd, "dlt-convert -a %s 2>/dev/null", rx);
    if (i % 5 == 4 && run(cmd, out, sizeof out) >= 0) {
      seen = 0;
      for (char *p = out; (p = strstr(p, "qnx-dlt-msg")) != NULL; p++) seen++;
    }
  }
  DLT_UNREGISTER_CONTEXT(ctx);
  DLT_UNREGISTER_APP();
  usleep(300000);
  stop(rpid, "dlt-receive");
  stop(dpid, "dlt-daemon");

  /* The file, through the libdlt file API. */
  DltFile file;
  int found[NMSG] = {0}, ids_ok = 1, total = 0;
  dlt_file_init(&file, 0);
  int opened = dlt_file_open(&file, rx, 0) >= DLT_RETURN_OK;
  QT_CHECK(opened, "dlt_file_open on the received file");
  if (opened) {
    while (dlt_file_read(&file, 0) >= DLT_RETURN_OK) {}
    total = file.counter;
    for (int i = 0; i < file.counter; i++) {
      char text[512];
      if (dlt_file_message(&file, i, 0) < DLT_RETURN_OK) continue;
      if (dlt_message_payload(&file.msg, text, sizeof text, DLT_OUTPUT_ASCII, 1) < DLT_RETURN_OK) continue;
      const char *m = strstr(text, "qnx-dlt-msg");
      if (!m) continue;
      int k = atoi(m + strlen("qnx-dlt-msg"));
      if (k >= 0 && k < NMSG) found[k]++;
      if (!file.msg.extendedheader || memcmp(file.msg.extendedheader->apid, "QNXT", 4) ||
          memcmp(file.msg.extendedheader->ctid, "TCTX", 4))
        ids_ok = 0;
    }
  }
  int all = 1;
  for (int k = 0; k < NMSG; k++) all &= found[k] == 1;
  printf("  info %d message(s) in the file, ours: %d %d %d %d %d\n", total, found[0], found[1],
         found[2], found[3], found[4]);
  QT_CHECK(all, "each of qnx-dlt-msg 0..4 received exactly once");
  QT_CHECK(all && ids_ok, "app ID QNXT / context ID TCTX on our messages");
  if (!all) { show(dlog); show(rlog); }
  dlt_file_free(&file, 0);

  snprintf(cmd, sizeof cmd, "dlt-convert -a %s 2>&1", rx);
  int rc = run(cmd, out, sizeof out);
  const char *l = strstr(out, "qnx-dlt-msg 3");
  int line_ok = 0;
  if (l) { /* the same line carries the ECU, app and context IDs */
    const char *b = l;
    while (b > out && b[-1] != '\n') b--;
    char line[512];
    snprintf(line, sizeof line, "%.*s", (int)(l - b) + 13, b);
    printf("  info %s\n", line);
    line_ok = strstr(line, "QNXT") && strstr(line, "TCTX") && strstr(line, "log info");
  }
  QT_CHECK(rc == 0 && line_ok, "dlt-convert -a prints \"... QNXT TCTX log info ... qnx-dlt-msg 3\"");
  snprintf(cmd, sizeof cmd, "dlt-convert -c %s 2>&1", rx);
  rc = run(cmd, out, sizeof out);
  printf("  info dlt-convert -c: %.100s", out);
  QT_CHECK(rc == 0 && strstr(out, "Total number of messages"), "dlt-convert -c counts the messages");

  snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  if (system(cmd) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
