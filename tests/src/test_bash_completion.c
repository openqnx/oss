/* bash-completion: the ported bash run interactively on a pseudo-terminal
 * (forkpty), the framework sourced into it, and TAB pressed the way a user
 * would; what readline echoes back is checked. Checks: bash_completion
 * loads; "ssh qnx<TAB><TAB>" lists the two matching hosts from
 * ~/.ssh/config (and not the third); ssh's completion was loaded on demand
 * (complete -p ssh names _comp_cmd_ssh); "kill -TER<TAB>" completes to
 * -TERM; "cd d/sub<TAB><TAB>" lists the two directories, "cat d/fi<TAB>"
 * completes the file;
 * and the installed completions directory holds hundreds of commands. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unix.h>

#define L "/usr/local"
static int pty = -1; static pid_t sh = -1;
static char out[65536]; static size_t on;
static void on_alarm(int s) { (void)s; if (sh > 0) kill(sh, SIGKILL); static const char m[] = "FAIL bash-completion (watchdog)\n"; (void)!write(1, m, sizeof m - 1); _exit(2); }
/* read what bash prints for ms milliseconds (or until want appears) */
static int pump(int ms, const char *want) {
  for (int t = 0; t < ms; t += 50) {
    struct pollfd p = { pty, POLLIN, 0 };
    if (poll(&p, 1, 50) > 0) {
      ssize_t n = read(pty, out + on, sizeof out - 1 - on);
      if (n <= 0) return 0;
      on += (size_t)n; out[on] = 0;
    }
    if (want && strstr(out, want)) return 1;
  }
  return want ? strstr(out, want) != NULL : 1;
}
static void type(const char *s) { (void)!write(pty, s, strlen(s)); }
/* type s, let readline answer, return everything echoed; then clear the line */
static const char *tab(const char *s, const char *want) {
  on = 0; out[0] = 0;
  /* the line, then each TAB on its own once readline has answered the last */
  for (const char *c = s; *c; c++) {
    (void)!write(pty, c, 1);
    if (*c == '\t') pump(1500, NULL);
  }
  pump(3000, want);
  static char got[8192]; snprintf(got, sizeof got, "%s", out);
  type("\x15"); pump(300, NULL);   /* Ctrl-U: kill the line */
  return got;
}
int main(void) {
  QT_BEGIN("bash-completion");
  signal(SIGALRM, on_alarm); alarm(120);
  char T[] = "/tmp/qnxport-bashcomp.XXXXXX";
  if (!mkdtemp(T)) { QT_CHECK(0, "temp dir"); QT_END(); }
  char p[256];
  snprintf(p, sizeof p, "%s/home", T); mkdir(p, 0700);
  snprintf(p, sizeof p, "%s/home/.ssh", T); mkdir(p, 0700);
  snprintf(p, sizeof p, "%s/home/.ssh/config", T);
  FILE *f = fopen(p, "w"); if (f) { fputs("Host qnxbox\n  HostName 192.0.2.7\nHost qnxother\n  HostName 192.0.2.8\nHost elsewhere\n", f); fclose(f); }
  snprintf(p, sizeof p, "%s/d", T); mkdir(p, 0755);
  snprintf(p, sizeof p, "%s/d/sub1", T); mkdir(p, 0755);
  snprintf(p, sizeof p, "%s/d/sub2", T); mkdir(p, 0755);
  snprintf(p, sizeof p, "%s/d/file.txt", T); f = fopen(p, "w"); if (f) fclose(f);

  struct winsize ws = { .ws_row = 50, .ws_col = 200 };
  sh = forkpty(&pty, NULL, NULL, &ws);
  if (sh == 0) {
    char home[256]; snprintf(home, sizeof home, "%s/home", T);
    if (chdir(T) != 0) _exit(126);
    setenv("HOME", home, 1); setenv("TERM", "dumb", 1); setenv("INPUTRC", "/dev/null", 1);
    unsetenv("BASH_COMPLETION_USER_DIR");
    execl(L "/bin/bash", "bash", "--norc", "--noprofile", "-i", (char *)NULL);
    execlp("bash", "bash", "--norc", "--noprofile", "-i", (char *)NULL);
    _exit(127);
  }
  QT_CHECK(sh > 0, "bash -i on a pseudo-terminal (forkpty)");
  if (sh <= 0) QT_END();
  pump(1500, NULL);
  on = 0; out[0] = 0;
  type("PS1='$ '; . " L "/share/bash-completion/bash_completion && echo LOADED-$((6*7))\n");
  QT_CHECK(pump(20000, "LOADED-42"), "bash_completion sourced in an interactive bash");

  const char *r = tab("ssh qnx\t\t", "qnxother");
  QT_CHECK(strstr(r, "qnxbox") && strstr(r, "qnxother") && !strstr(r, "elsewhere"), "ssh qnx<TAB><TAB>: qnxbox qnxother (from ~/.ssh/config)");
  if (!strstr(r, "qnxother")) printf("  info got: %s\n", r);
  on = 0; out[0] = 0; type("complete -p ssh\n");
  QT_CHECK(pump(4000, "_comp_cmd_ssh ssh"), "ssh's completion loaded on demand: complete -F _comp_cmd_ssh ssh");
  /* signal names are offered without their SIG prefix: -TER completes to -TERM */
  r = tab("kill -TER\t", "-TERM");
  QT_CHECK(strstr(r, "-TERM") != NULL, "kill -TER<TAB>: -TERM (bash's signal list)");
  if (!strstr(r, "-TERM")) printf("  info got: %s\n", r);
  /* typed up to the common prefix, so the second TAB lists (a TAB that
     inserts text starts the count again) */
  r = tab("cd d/sub\t\t", "sub2");
  QT_CHECK(strstr(r, "sub1") && strstr(r, "sub2") && !strstr(r, "file.txt"), "cd d/sub<TAB><TAB>: sub1/ sub2/ (directories only)");
  r = tab("cat d/fi\t", "file.txt");
  QT_CHECK(strstr(r, "file.txt") != NULL, "cat d/fi<TAB>: d/file.txt (files for cat)");
  type("exit\n");
  int st = 0;
  for (int i = 0; i < 50; i++) { if (waitpid(sh, &st, WNOHANG) == sh) { sh = -1; break; } pump(100, NULL); }
  if (sh > 0) { kill(sh, SIGKILL); waitpid(sh, NULL, 0); }

  int n = 0; DIR *d = opendir(L "/share/bash-completion/completions-core");
  struct dirent *e; while (d && (e = readdir(d))) if (strstr(e->d_name, ".bash")) n++;
  if (d) closedir(d);
  printf("  info %d command completions\n", n);
  QT_CHECK(n > 300, "more than 300 command completions installed");
  char cmd[300]; snprintf(cmd, sizeof cmd, "rm -rf %s", T); (void)!system(cmd);
  QT_END();
}
