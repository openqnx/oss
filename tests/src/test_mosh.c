/* mosh on loopback: mosh-server starts a session (a shell command that
 * prints MOSH-QNX-$((6*7)) and waits) on a fixed UDP port of 127.0.0.1; its
 * "MOSH CONNECT <port> <key>" line must name that port and a 22-character
 * key. mosh-client, run on a pty with that key in MOSH_KEY, must then show
 * the command's output -- "MOSH-QNX-42", which only the shell's arithmetic
 * produces -- proving the encrypted UDP session, the server's pty and the
 * client's terminal emulation all work. Ctrl-^ . must end the client.
 * Also: the versions, and mosh-client -c reading the color count from
 * terminfo (xterm-256color: 256; vt100: none, -1). The server is stopped on
 * every exit path (atexit, SIGINT/SIGTERM/SIGHUP and an alarm). */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifdef __QNX__
#include <unix.h> /* forkpty() */
#else
#include <pty.h>
#endif
#include "../common.h"

static volatile pid_t server_pid = -1, client_pid = -1;

static void stop_all(void)
{
  if (client_pid > 0) { kill(client_pid, SIGKILL); waitpid(client_pid, NULL, 0); client_pid = -1; }
  if (server_pid > 0) {
    /* Detached from us, so not our child: SIGTERM, then poll until gone. */
    kill(server_pid, SIGTERM);
    for (int i = 0; i < 50 && kill(server_pid, 0) == 0; i++) usleep(100000);
    if (kill(server_pid, 0) == 0) kill(server_pid, SIGKILL);
    server_pid = -1;
  }
}

static void on_signal(int sig)
{
  stop_all();
  signal(sig, SIG_DFL);
  raise(sig);
}

/* Run argv with stdout+stderr into a pipe; collect up to cap bytes until
 * EOF or timeout_s. Returns the exit status (or -1). */
static int run_capture(char *const argv[], char *out, size_t cap, int timeout_s)
{
  int p[2];
  if (pipe(p) != 0) return -1;
  pid_t pid = fork();
  if (pid < 0) return -1;
  if (pid == 0) {
    dup2(p[1], 1); dup2(p[1], 2); close(p[0]); close(p[1]);
    execvp(argv[0], argv);
    _exit(127);
  }
  close(p[1]);
  size_t n = 0; time_t end = time(NULL) + timeout_s;
  for (;;) {
    struct pollfd pf = { p[0], POLLIN, 0 };
    int left = (int)(end - time(NULL));
    if (left <= 0 || poll(&pf, 1, left * 1000) <= 0) break;
    ssize_t r = read(p[0], out + n, cap - 1 - n);
    if (r <= 0) break;
    n += (size_t)r;
    if (n >= cap - 1) break;
  }
  out[n] = 0;
  close(p[0]);
  int st;
  if (waitpid(pid, &st, 0) != pid) return -1;
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main(void)
{
  QT_BEGIN("mosh");
  atexit(stop_all);
  signal(SIGINT, on_signal); signal(SIGTERM, on_signal);
  signal(SIGHUP, on_signal); signal(SIGALRM, on_signal);
  alarm(150);
  /* QNX's C locale is UTF-8, which is what mosh insists on. */
  setenv("LC_ALL", "C", 1);
  unsetenv("LANG"); unsetenv("LC_CTYPE");
  /* The server exits on its own if the client never reaches it. */
  setenv("MOSH_SERVER_NETWORK_TMOUT", "60", 1);

  char buf[8192];
  char *av_v[] = { "mosh-server", "--version", NULL };
  int rc = run_capture(av_v, buf, sizeof buf, 10);
  QT_CHECK(rc == 0 && strstr(buf, "mosh-server (mosh 1.4.0)"), "mosh-server --version: mosh 1.4.0");
  /* mosh-client has no version option; its usage text names the version. */
  char *av_h[] = { "mosh-client", "--help", NULL };
  run_capture(av_h, buf, sizeof buf, 10);
  QT_CHECK(strstr(buf, "mosh-client (mosh 1.4.0)") != NULL, "mosh-client usage names mosh 1.4.0");

  char *av_c[] = { "mosh-client", "-c", NULL };
  setenv("TERM", "xterm-256color", 1);
  rc = run_capture(av_c, buf, sizeof buf, 10);
  printf("  info mosh-client -c (xterm-256color): %s", buf);
  QT_CHECK(rc == 0 && atoi(buf) == 256, "terminfo: xterm-256color has 256 colors");
  setenv("TERM", "vt100", 1);
  rc = run_capture(av_c, buf, sizeof buf, 10);
  QT_CHECK(rc == 0 && atoi(buf) == -1, "terminfo: vt100 has no color count (-1)");

  char port[16];
  snprintf(port, sizeof port, "%d", 60101 + (int)(getpid() % 400));
  char *av_s[] = { "mosh-server", "new", "-i", "127.0.0.1", "-p", port, "-c", "8",
                   "-l", "LC_ALL=C", "--", "/bin/sh", "-c",
                   "echo MOSH-QNX-$((6*7)); sleep 40", NULL };
  rc = run_capture(av_s, buf, sizeof buf, 30);
  char *mc = strstr(buf, "MOSH CONNECT ");
  char cport[16] = "", key[64] = "";
  if (mc) sscanf(mc, "MOSH CONNECT %15s %63s", cport, key);
  char *dp = strstr(buf, "detached, pid = ");
  if (dp) server_pid = (pid_t)atoi(dp + strlen("detached, pid = "));
  if (!mc || rc != 0) printf("  info mosh-server output (rc=%d): %.600s\n", rc, buf);
  QT_CHECK(rc == 0 && mc != NULL, "mosh-server new prints MOSH CONNECT");
  QT_CHECK(strcmp(cport, port) == 0, "MOSH CONNECT names the requested UDP port");
  QT_CHECK(strlen(key) == 22, "MOSH CONNECT carries a 22-character session key");
  QT_CHECK(server_pid > 0 && kill(server_pid, 0) == 0, "mosh-server detached and is running");

  if (mc && strlen(key) == 22 && server_pid > 0) {
    int master;
    struct winsize ws = { 24, 80, 0, 0 };
    pid_t pid = forkpty(&master, NULL, NULL, &ws);
    if (pid == 0) {
      setenv("MOSH_KEY", key, 1);
      setenv("TERM", "vt100", 1);
      execlp("mosh-client", "mosh-client", "127.0.0.1", port, (char *)NULL);
      _exit(127);
    }
    QT_CHECK(pid > 0, "forkpty for mosh-client");
    if (pid > 0) {
      client_pid = pid;
      static char screen[65536]; size_t n = 0; int seen = 0;
      time_t end = time(NULL) + 40;
      while (!seen && time(NULL) < end) {
        struct pollfd pf = { master, POLLIN, 0 };
        if (poll(&pf, 1, 1000) <= 0) continue;
        ssize_t r = read(master, screen + n, sizeof screen - 1 - n);
        if (r <= 0) break;
        n += (size_t)r; screen[n] = 0;
        if (strstr(screen, "MOSH-QNX-42")) seen = 1;
        if (n > sizeof screen - 1024) n = 0;
      }
      if (!seen) {
        printf("  info client screen (%zu bytes):", n);
        for (size_t i = 0; i < n && i < 400; i++)
          putchar(screen[i] >= 32 && screen[i] < 127 ? screen[i] : '.');
        putchar('\n');
      }
      QT_CHECK(seen, "mosh-client shows the session's output (MOSH-QNX-42)");
      /* Ctrl-^ . is mosh's escape to quit the client. */
      (void)!write(master, "\x1e.", 2);
      int st = -1, done = 0;
      static char tail[8192]; size_t tn = 0;
      /* The client may take a few seconds to tell the server it is going. */
      time_t qend = time(NULL) + 30;
      while (!done && time(NULL) < qend) {
        /* Drain the pty so the client never blocks writing its exit; once the
           client has closed its side, poll reports a hang-up at once. */
        struct pollfd pf = { master, POLLIN, 0 };
        ssize_t r = -1;
        if (poll(&pf, 1, 100) > 0 && (pf.revents & POLLIN))
          r = read(master, tail + tn, sizeof tail - 1 - tn);
        if (r > 0) { tn += (size_t)r; if (tn > sizeof tail - 512) tn = 0; }
        else usleep(100000);
        if (waitpid(pid, &st, WNOHANG) == pid) done = 1;
      }
      if (done) client_pid = -1;
      tail[tn] = 0;
      printf("  info client %s; last output: ", done ? "exited" : "still running after 30 s");
      for (size_t i = tn > 200 ? tn - 200 : 0; i < tn; i++)
        putchar(tail[i] >= 32 && tail[i] < 127 ? tail[i] : '.');
      putchar('\n');
      QT_CHECK(done && WIFEXITED(st) && WEXITSTATUS(st) == 0, "Ctrl-^ . ends mosh-client (exit 0)");
      close(master);
    }
  }

  pid_t sp = server_pid;
  stop_all();
  QT_CHECK(sp <= 0 || kill(sp, 0) != 0, "mosh-server stopped");
  QT_END();
}
