/* A one-connection inetd for test_inetutils: listens on 127.0.0.1:<port>,
 * accepts one TCP connection and runs the server program with the socket
 * as its stdin/stdout/stderr, as inetd does for telnetd and ftpd.
 * Usage: test_inetutils_inetd <port> <program> [args...]
 * Prints "listening" when ready; exits with the server's status.
 * Alone (it is installed as a test of its own), it checks itself: one
 * connection to an ephemeral port served by "sh -c 'echo inetd-ok'". */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int listen_on(int port, int *bound)
{
  struct sockaddr_in a;
  socklen_t l = sizeof a;
  int s = socket(AF_INET, SOCK_STREAM, 0), one = 1;
  if (s < 0) return -1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  memset(&a, 0, sizeof a);
  a.sin_family = AF_INET;
  a.sin_port = htons((unsigned short)port);
  a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(s, (struct sockaddr *)&a, sizeof a) < 0 || listen(s, 1) < 0) { close(s); return -1; }
  getsockname(s, (struct sockaddr *)&a, &l);
  *bound = ntohs(a.sin_port);
  return s;
}

/* Accept one connection on s and run prog on it; the server's exit status. */
static int serve(int s, char **prog)
{
  int c = accept(s, NULL, NULL), st = 0;
  pid_t p;
  if (c < 0) { perror("accept"); return 1; }
  close(s);
  p = fork();
  if (p == 0) {
    alarm(0);
    dup2(c, 0); dup2(c, 1); dup2(c, 2);
    if (c > 2) close(c);
    execv(prog[0], prog);
    _exit(127);
  }
  close(c);
  if (p < 0 || waitpid(p, &st, 0) < 0) return 1;
  return WIFEXITED(st) ? WEXITSTATUS(st) : 128 + WTERMSIG(st);
}

int main(int argc, char **argv)
{
  int s, port = 0;
  alarm(120);   /* never outlive the test */
  if (argc == 1) {
    char *echo[] = { "/bin/sh", "-c", "echo inetd-ok", NULL };
    struct sockaddr_in a;
    char buf[64] = "";
    int c, n = 0, k, rc;
    pid_t p;
    printf("== inetutils (inetd helper) ==\n");
    s = listen_on(0, &port);
    if (s < 0) { printf("FAIL cannot listen on 127.0.0.1\n"); return 1; }
    p = fork();
    if (p == 0) _exit(serve(s, echo));
    close(s);
    c = socket(AF_INET, SOCK_STREAM, 0);
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET; a.sin_port = htons((unsigned short)port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (c < 0 || connect(c, (struct sockaddr *)&a, sizeof a) < 0) { printf("FAIL connect\n"); return 1; }
    while (n < (int)sizeof buf - 1 && (k = (int)read(c, buf + n, sizeof buf - 1 - (size_t)n)) > 0) n += k;
    buf[n] = '\0';
    close(c);
    waitpid(p, &rc, 0);
    k = strcmp(buf, "inetd-ok\n") == 0;
    printf("  %s sh -c echo served on 127.0.0.1:%d answered \"inetd-ok\"\n", k ? "ok  " : "FAIL", port);
    printf(k ? "PASS inetutils (inetd helper)\n" : "FAIL inetutils (inetd helper)\n");
    return !k;
  }
  if (argc < 3) { fprintf(stderr, "usage: %s <port> <program> [args...]\n", argv[0]); return 2; }
  s = listen_on(atoi(argv[1]), &port);
  if (s < 0) { perror("bind/listen"); return 1; }
  printf("listening\n"); fflush(stdout);
  return serve(s, argv + 2);
}
