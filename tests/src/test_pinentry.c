/* pinentry: the headless flavours driven as gpg-agent drives them -- the
 * Assuan protocol on stdin/stdout -- with the dialog on a pseudo-terminal
 * that this test answers like a user. Checks, for pinentry-tty: the Assuan
 * greeting, GETINFO version (1.3.3) and flavor (tty), SETDESC/SETPROMPT,
 * GETPIN showing the prompt on the terminal and returning the typed PIN
 * percent-escaped ("s3cr3t%x" -> "D s3cr3t%25x"), CONFIRM's button
 * prompt answered "o" (OK) and "c" (ERR ... cancelled); for pinentry-curses (also installed as
 * pinentry): the curses dialog drawn on a vt100 pty with the description,
 * and the PIN typed into it returned. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unix.h>

#define BIN "/usr/local/bin/"
static pid_t pe = -1; static int to_pe = -1, from_pe = -1, master = -1;
static char tty[8192]; static size_t tn;
static void stop(void) { if (pe > 0) { kill(pe, SIGKILL); waitpid(pe, NULL, 0); pe = -1; } }
static void on_alarm(int s) { (void)s; if (pe > 0) kill(pe, SIGKILL); static const char m[] = "FAIL pinentry (watchdog)\n"; (void)!write(1, m, sizeof m - 1); _exit(2); }
static void drain_tty(int ms) {
  for (int t = 0; t < ms; t += 20) {
    struct pollfd p = { master, POLLIN, 0 };
    if (poll(&p, 1, 20) > 0) { ssize_t n = read(master, tty + tn, sizeof tty - 1 - tn); if (n > 0) { tn += (size_t)n; tty[tn] = 0; } }
  }
}
static int wait_tty(const char *want, int ms) {
  for (int t = 0; t < ms && !strstr(tty, want); t += 100) drain_tty(100);
  return strstr(tty, want) != NULL;
}
/* one Assuan reply: data lines joined, up to the OK/ERR line (returned) */
static char data[1024], line[1024];
static const char *reply(void) {
  static char buf[4096]; static size_t bn; data[0] = 0;
  for (int t = 0; t < 15000; ) {
    char *nl;
    while ((nl = memchr(buf, '\n', bn))) {
      size_t l = (size_t)(nl - buf); if (l >= sizeof line) l = sizeof line - 1;
      memcpy(line, buf, l); line[l] = 0; memmove(buf, nl + 1, bn - (size_t)(nl - buf) - 1); bn -= (size_t)(nl - buf) + 1;
      if (!strncmp(line, "D ", 2)) strncat(data, line + 2, sizeof data - strlen(data) - 1);
      else if (!strncmp(line, "OK", 2) || !strncmp(line, "ERR", 3)) return line;
    }
    struct pollfd p[2] = { { from_pe, POLLIN, 0 }, { master, POLLIN, 0 } };
    if (poll(p, 2, 100) > 0) {
      if (p[0].revents & (POLLIN | POLLHUP)) { ssize_t n = read(from_pe, buf + bn, sizeof buf - 1 - bn); if (n <= 0) return "EOF"; bn += (size_t)n; }
      if (p[1].revents & POLLIN) { ssize_t n = read(master, tty + tn, sizeof tty - 1 - tn); if (n > 0) { tn += (size_t)n; tty[tn] = 0; } }
    } else t += 100;
  }
  return "TIMEOUT";
}
static const char *cmd(const char *c) { (void)!write(to_pe, c, strlen(c)); (void)!write(to_pe, "\n", 1); return reply(); }
static int start(const char *prog) {
  int s = -1; char sname[64];
  struct winsize ws = { .ws_row = 24, .ws_col = 80 };
  if (openpty(&master, &s, sname, NULL, &ws) != 0) return 0;
  int in[2], out[2]; if (pipe(in) || pipe(out)) return 0;
  pe = fork();
  if (pe == 0) {
    dup2(in[0], 0); dup2(out[1], 1); close(in[1]); close(out[0]); close(master); close(s);
    execl(prog, prog, "--ttyname", sname, "--ttytype", "vt100", (char *)NULL);
    _exit(127);
  }
  close(in[0]); close(out[1]); close(s);
  to_pe = in[1]; from_pe = out[0]; tn = 0; tty[0] = 0;
  return pe > 0;
}
static void finish(void) {
  cmd("BYE"); close(to_pe); close(from_pe); close(master);
  for (int i = 0; i < 50 && pe > 0; i++) { if (waitpid(pe, NULL, WNOHANG) == pe) pe = -1; else usleep(100000); }
  stop();
}
int main(void) {
  QT_BEGIN("pinentry");
  signal(SIGALRM, on_alarm); alarm(120); atexit(stop); signal(SIGPIPE, SIG_IGN);
  setenv("TERM", "vt100", 1);
  if (access("/usr/local/share/terminfo", R_OK) == 0) setenv("TERMINFO", "/usr/local/share/terminfo", 1);

  /* ---- pinentry-tty ---- */
  QT_CHECK(start(BIN "pinentry-tty"), "pinentry-tty started (Assuan on pipes, dialog on a pty)");
  const char *r = reply();
  QT_CHECK(!strncmp(r, "OK Pleased to meet you", 22), "Assuan greeting: OK Pleased to meet you");
  r = cmd("GETINFO version");
  QT_CHECK(!strcmp(r, "OK") && !strcmp(data, "1.3.3"), "GETINFO version: 1.3.3");
  r = cmd("GETINFO flavor");
  QT_CHECK(!strcmp(r, "OK") && !strncmp(data, "tty", 3), "GETINFO flavor: tty");
  QT_CHECK(!strcmp(cmd("SETDESC Enter the QNX test PIN"), "OK") && !strcmp(cmd("SETPROMPT TestPIN:"), "OK"), "SETDESC, SETPROMPT");
  (void)!write(to_pe, "GETPIN\n", 7);
  int prompted = wait_tty("TestPIN:", 10000);
  QT_CHECK(prompted && strstr(tty, "Enter the QNX test PIN"), "GETPIN: description and prompt shown on the terminal");
  (void)!write(master, "s3cr3t%x\r", 9);
  r = reply();
  QT_CHECK(!strcmp(r, "OK") && !strcmp(data, "s3cr3t%25x"), "the typed PIN comes back percent-escaped: D s3cr3t%25x");
  if (strcmp(data, "s3cr3t%25x")) printf("  info got [%s] %s\n", data, r);
  cmd("SETDESC Really continue?");
  tn = 0; tty[0] = 0;
  (void)!write(to_pe, "CONFIRM\n", 8);
  /* the buttons are offered by their first letters: [oc]? for OK/Cancel */
  int asked = wait_tty("]? ", 10000); drain_tty(300);
  (void)!write(master, "o", 1);
  r = reply();
  QT_CHECK(asked && !strcmp(r, "OK"), "CONFIRM: the buttons prompt answered o -> OK");
  { char *b = strrchr(tty, '['); if (b) { char *e = strchr(b, '\n'); if (e) *e = 0; e = strchr(b, '\r'); if (e) *e = 0; printf("  info buttons: %s\n", b); } }
  /* with a Cancel button set, it is offered too: [oc]? */
  cmd("SETCANCEL Cancel");
  tn = 0; tty[0] = 0;
  (void)!write(to_pe, "CONFIRM\n", 8);
  int two = wait_tty("[oc]? ", 10000); drain_tty(300);
  QT_CHECK(two, "SETCANCEL: the prompt offers [oc]");
  (void)!write(master, "c", 1);
  r = reply();
  QT_CHECK(!strncmp(r, "ERR", 3) && strstr(r, "ancel"), "CONFIRM answered c -> ERR ... cancelled");
  printf("  info %s\n", r);
  finish();

  /* ---- pinentry-curses ---- */
  QT_CHECK(start(BIN "pinentry"), "pinentry (-> pinentry-curses) started on a vt100 pty");
  r = reply();
  QT_CHECK(!strncmp(r, "OK", 2), "Assuan greeting");
  cmd("GETINFO flavor");
  QT_CHECK(!strncmp(data, "curses", 6), "GETINFO flavor: curses");
  cmd("SETDESC Curses dialog on QNX"); cmd("SETPROMPT PIN"); cmd("SETOK Accept");
  tn = 0; tty[0] = 0;
  (void)!write(to_pe, "GETPIN\n", 7);
  int drawn = wait_tty("Curses dialog on QNX", 10000);
  drain_tty(800);
  QT_CHECK(drawn && strstr(tty, "Accept"), "the curses dialog is drawn (description and the OK button label)");
  (void)!write(master, "curses42\r", 9);
  r = reply();
  QT_CHECK(!strcmp(r, "OK") && !strcmp(data, "curses42"), "the PIN typed into the dialog: D curses42");
  if (strcmp(data, "curses42")) printf("  info got [%s] %s\n", data, r);
  finish();
  QT_END();
}
