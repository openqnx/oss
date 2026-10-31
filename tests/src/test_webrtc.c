/* webrtc: what the package runs on a target is its example signalling
 * server, lib/webrtc/peerconnection_server (the -dev package holds the
 * headers, no library). Start it on a loopback port and drive its HTTP
 * protocol the way two peerconnection clients do: both sign in, the first
 * learns of the second through its hanging /wait, a message posted to the
 * second is delivered to its /wait with the sender's id, and signing out is
 * announced to the remaining peer. */
#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

static int port;
struct resp { int status, pragma; char body[512]; };

static struct resp http(const char *method, const char *path, const char *body) {
  struct resp r; memset(&r, 0, sizeof r); r.pragma = -1;
  int s = socket(AF_INET, SOCK_STREAM, 0);
  struct timeval tv = { 3, 0 };
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  struct sockaddr_in sa; memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET; sa.sin_port = htons((unsigned short)port); sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(s, (struct sockaddr *)&sa, sizeof sa)) { close(s); return r; }
  char req[1024];
  int n = snprintf(req, sizeof req, "%s %s HTTP/1.0\r\nHost: localhost\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n%s",
                   method, path, body ? (int)strlen(body) : 0, body ? body : "");
  if (write(s, req, n) != n) { close(s); return r; }
  char buf[4096]; int got = 0, k;
  while (got < (int)sizeof buf - 1 && (k = (int)read(s, buf + got, sizeof buf - 1 - got)) > 0) {
    got += k; buf[got] = 0;
    char *hdr_end = strstr(buf, "\r\n\r\n"), *cl = strstr(buf, "Content-Length:");
    if (hdr_end && cl && (int)(hdr_end + 4 - buf) + atoi(cl + 15) <= got) break;   /* complete; /wait keeps it open */
  }
  close(s);
  buf[got] = 0;
  if (sscanf(buf, "HTTP/1.%*d %d", &r.status) != 1) return r;
  char *pr = strstr(buf, "Pragma: "); if (pr) r.pragma = atoi(pr + 8);
  char *b = strstr(buf, "\r\n\r\n"); if (b) snprintf(r.body, sizeof r.body, "%s", b + 4);
  return r;
}

int main(void) {
  QT_BEGIN("webrtc");
  const char *bins[] = { getenv("QNX_PORT_PCSERVER"), "/usr/local/lib/webrtc/peerconnection_server", "/data/oss/usr/local/lib/webrtc/peerconnection_server" };
  const char *bin = NULL;
  for (int i = 0; i < 3 && !bin; i++) if (bins[i] && access(bins[i], X_OK) == 0) bin = bins[i];
  QT_CHECK(bin != NULL, "lib/webrtc/peerconnection_server installed");
  if (!bin) QT_END();
  port = 30000 + (int)(getpid() % 20000);
  char parg[32]; snprintf(parg, sizeof parg, "--port=%d", port);
  printf("  info %s %s\n", bin, parg);
  pid_t pid = fork();
  if (pid == 0) { int fd = open("/dev/null", 1); if (fd >= 0) { dup2(fd, 1); } execl(bin, "peerconnection_server", parg, (char *)NULL); _exit(127); }
  struct resp a; memset(&a, 0, sizeof a);
  for (int i = 0; i < 50 && a.status != 200; i++) { a = http("GET", "/sign_in?alice", NULL); if (a.status != 200) usleep(100000); }
  int A = a.pragma;
  char want[128]; snprintf(want, sizeof want, "alice,%d,1\n", A);
  QT_CHECK(a.status == 200 && A > 0 && !strcmp(a.body, want), "alice signs in: 200, her id in Pragma and the peer list");
  if (a.status == 200) {
    struct resp b = http("GET", "/sign_in?bob", NULL);
    int B = b.pragma;
    char wb[128]; snprintf(wb, sizeof wb, "bob,%d,1\nalice,%d,1\n", B, A);
    QT_CHECK(b.status == 200 && B > 0 && B != A && !strcmp(b.body, wb), "bob signs in: the list has bob and alice");
    char path[128]; snprintf(path, sizeof path, "/wait?peer_id=%d", A);
    struct resp w = http("GET", path, NULL);
    snprintf(wb, sizeof wb, "bob,%d,1\n", B);
    QT_CHECK(w.status == 200 && !strcmp(w.body, wb), "alice's /wait: bob is online");
    snprintf(path, sizeof path, "/message?peer_id=%d&to=%d", A, B);
    struct resp m = http("POST", path, "{\"type\":\"offer\",\"sdp\":\"v=0\"}");
    QT_CHECK(m.status == 200, "alice posts a message to bob");
    snprintf(path, sizeof path, "/wait?peer_id=%d", B);
    struct resp bw = http("GET", path, NULL);
    printf("  info bob's /wait: %d, from %d, \"%s\"\n", bw.status, bw.pragma, bw.body);
    QT_CHECK(bw.status == 200 && bw.pragma == A && !strcmp(bw.body, "{\"type\":\"offer\",\"sdp\":\"v=0\"}"), "bob's /wait delivers it, from alice's id");
    snprintf(path, sizeof path, "/sign_out?peer_id=%d", B);
    QT_CHECK(http("GET", path, NULL).status == 200, "bob signs out");
    snprintf(path, sizeof path, "/wait?peer_id=%d", A);
    w = http("GET", path, NULL);
    snprintf(wb, sizeof wb, "bob,%d,0\n", B);
    QT_CHECK(w.status == 200 && !strcmp(w.body, wb), "alice is told bob went offline");
    QT_CHECK(http("GET", "/wait?peer_id=999999", NULL).status == 500, "an unknown peer id is refused (500)");
  }
  if (pid > 0) { kill(pid, SIGTERM); int st; for (int i = 0; i < 30 && waitpid(pid, &st, WNOHANG) == 0; i++) usleep(100000); }
  QT_END();
}
