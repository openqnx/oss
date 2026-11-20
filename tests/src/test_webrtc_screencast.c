/* webrtc-screencast: the desktop in a browser. labwc is started on wlroots'
 * headless backend (no display needed) and webrtc-screencast on a loopback
 * port; then what a browser does, by hand over HTTP: the page, the viewer
 * count, and the SDP exchange -- a recvonly VP8 offer posted to /offer must
 * come back as a sendonly VP8 answer carrying DTLS and ICE (its candidates
 * gathered, since the page does not trickle). A second run with --token must
 * refuse a request without the token and take one with it; a third with
 * --input must find the compositor's virtual devices and accept the page's
 * data channel (the mouse and the keyboard go through it). */
#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

static int port;
static char body[16384];

static int http(const char *method, const char *path, const char *type, const char *data) {
  int s = socket(AF_INET, SOCK_STREAM, 0), status = 0;
  struct timeval tv = { 20, 0 };
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  struct sockaddr_in sa; memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET; sa.sin_port = htons((unsigned short)port); sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  body[0] = 0;
  if (connect(s, (struct sockaddr *)&sa, sizeof sa)) { close(s); return 0; }
  static char req[8192], buf[20000];
  int n = snprintf(req, sizeof req, "%s %s HTTP/1.1\r\nHost: localhost\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n%s",
                   method, path, type, data ? (int)strlen(data) : 0, data ? data : "");
  if (write(s, req, n) != n) { close(s); return 0; }
  int got = 0, k;
  while (got < (int)sizeof buf - 1 && (k = (int)read(s, buf + got, sizeof buf - 1 - got)) > 0) got += k;
  close(s);
  buf[got] = 0;
  sscanf(buf, "HTTP/1.%*d %d", &status);
  char *b = strstr(buf, "\r\n\r\n"); if (b) snprintf(body, sizeof body, "%s", b + 4);
  return status;
}

static pid_t spawn(char *const argv[]) {
  pid_t p = fork();
  if (p == 0) { execvp(argv[0], argv); _exit(127); }
  return p;
}
static void stop(pid_t p) {
  int st;
  if (p <= 0) return;
  kill(p, SIGTERM);
  for (int i = 0; i < 50; i++) { if (waitpid(p, &st, WNOHANG) == p) return; usleep(100000); }
  kill(p, SIGKILL); waitpid(p, &st, 0);
}

/* What a browser offers for "receive one video": VP8 only, DTLS, bundle. */
static const char offer_fmt[] =
  "v=0\r\n"
  "o=- 4611731400430051336 2 IN IP4 127.0.0.1\r\n"
  "s=-\r\n"
  "t=0 0\r\n"
  "a=group:BUNDLE %s\r\n"
  "a=msid-semantic: WMS\r\n"
  "m=video 9 UDP/TLS/RTP/SAVPF 96\r\n"
  "c=IN IP4 0.0.0.0\r\n"
  "a=rtcp:9 IN IP4 0.0.0.0\r\n"
  "a=ice-ufrag:qnxT\r\n"
  "a=ice-pwd:qnxporttestqnxporttest01\r\n"
  "a=ice-options:trickle\r\n"
  "a=fingerprint:sha-256 4A:AD:B9:B1:3F:82:18:3B:54:02:12:DF:3E:5D:49:6B:19:E5:7C:AB:4A:AD:B9:B1:3F:82:18:3B:54:02:12:DF\r\n"
  "a=setup:actpass\r\n"
  "a=mid:0\r\n"
  "a=recvonly\r\n"
  "a=rtcp-mux\r\n"
  "a=rtpmap:96 VP8/90000\r\n"
  "a=rtcp-fb:96 nack\r\n"
  "a=rtcp-fb:96 nack pli\r\n"
  "a=rtcp-fb:96 goog-remb\r\n%s";
static char offer[4096], offer2[4096];

int main(void) {
  QT_BEGIN("webrtc-screencast");
  snprintf(offer, sizeof offer, offer_fmt, "0", "");
  char rt[] = "/tmp/screencast-rt-XXXXXX";
  QT_CHECK(mkdtemp(rt) != NULL, "XDG_RUNTIME_DIR");
  chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1);
  setenv("WLR_BACKENDS", "headless", 1);
  setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY");
  char *lab[] = { "labwc", NULL };
  pid_t lp = spawn(lab);
  char sock[300]; snprintf(sock, sizeof sock, "%s/wayland-0", rt);
  struct stat sb; int up = 0;
  for (int i = 0; i < 150 && !up; i++) { if (stat(sock, &sb) == 0) up = 1; else usleep(100000); }
  QT_CHECK(up, "labwc (headless) is up: the compositor whose picture is sent");
  if (up) {
    setenv("WAYLAND_DISPLAY", "wayland-0", 1);
    port = 30000 + (int)(getpid() % 20000);
    char parg[16]; snprintf(parg, sizeof parg, "%d", port);
    char *sc[] = { "webrtc-screencast", "--bind", "127.0.0.1", "--port", parg, NULL };
    pid_t sp = spawn(sc);
    int st = 0;
    for (int i = 0; i < 100 && st != 200; i++) { st = http("GET", "/status", "text/plain", NULL); if (st != 200) usleep(100000); }
    QT_CHECK(st == 200 && !strcmp(body, "{\"viewers\":0}\n"), "GET /status: 200, nobody is watching yet");
    st = http("GET", "/", "text/plain", NULL);
    QT_CHECK(st == 200 && strstr(body, "<video") && strstr(body, "RTCPeerConnection") && strstr(body, "iceServers:[]"),
             "GET /: the page with the <video> and the WebRTC script, no STUN server by default");
    QT_CHECK(strstr(body, "const INPUT=false;") != NULL, "without --input the page is view only");
    QT_CHECK(http("GET", "/nothing", "text/plain", NULL) == 404, "an unknown path: 404");
    QT_CHECK(http("POST", "/offer", "application/sdp", "this is not SDP") == 503, "a broken offer is refused (503)");
    st = http("POST", "/offer", "application/sdp", offer);
    printf("  info answer: %d, %d bytes\n", st, (int)strlen(body));
    QT_CHECK(st == 200 && !strncmp(body, "v=0", 3), "POST /offer: 200 and an SDP answer");
    QT_CHECK(strstr(body, "m=video") && strstr(body, "a=sendonly"), "the answer sends video only");
    QT_CHECK(strstr(body, "VP8/90000") != NULL, "the answer's codec is VP8");
    QT_CHECK(strstr(body, "a=fingerprint:sha-256") && strstr(body, "a=setup:"), "the answer has its DTLS fingerprint and role");
    QT_CHECK(strstr(body, "a=ice-ufrag:") && strstr(body, "a=candidate:"), "the answer has ICE credentials and gathered candidates");
    QT_CHECK(strstr(body, "a=msid:") != NULL, "the answer names the track (msid)");
    st = http("GET", "/status", "text/plain", NULL);
    QT_CHECK(st == 200 && !strcmp(body, "{\"viewers\":1}\n"), "GET /status: one viewer now");
    QT_CHECK(waitpid(sp, &st, WNOHANG) == 0, "webrtc-screencast still running");
    stop(sp);

    char *tk[] = { "webrtc-screencast", "--bind", "127.0.0.1", "--port", parg, "--token", "s3cret", NULL };
    sp = spawn(tk);
    st = 0;
    for (int i = 0; i < 100 && st == 0; i++) { st = http("GET", "/status", "text/plain", NULL); if (st == 0) usleep(100000); }
    QT_CHECK(st == 403, "with --token: a request without the token is refused (403)");
    QT_CHECK(http("GET", "/?token=wrong", "text/plain", NULL) == 403, "with --token: a wrong token is refused (403)");
    QT_CHECK(http("GET", "/?token=s3cret", "text/plain", NULL) == 200, "with --token: the right token is let in (200)");
    stop(sp);

    /* --input: the compositor's virtual pointer and keyboard are found (labwc
       has both), the page turns its mouse and keyboard handlers on, and the
       answer to an offer with a data channel takes the channel. */
    char *ip[] = { "webrtc-screencast", "--bind", "127.0.0.1", "--port", parg, "--input", NULL };
    sp = spawn(ip);
    st = 0;
    for (int i = 0; i < 100 && st != 200; i++) { st = http("GET", "/", "text/plain", NULL); if (st != 200) usleep(100000); }
    QT_CHECK(st == 200 && strstr(body, "const INPUT=true;") && strstr(body, "createDataChannel"),
             "with --input: the page sends the mouse and the keyboard (data channel)");
    snprintf(offer2, sizeof offer2, offer_fmt, "0 1",
      "m=application 9 UDP/DTLS/SCTP webrtc-datachannel\r\n"
      "c=IN IP4 0.0.0.0\r\n"
      "a=ice-ufrag:qnxT\r\n"
      "a=ice-pwd:qnxporttestqnxporttest01\r\n"
      "a=ice-options:trickle\r\n"
      "a=fingerprint:sha-256 4A:AD:B9:B1:3F:82:18:3B:54:02:12:DF:3E:5D:49:6B:19:E5:7C:AB:4A:AD:B9:B1:3F:82:18:3B:54:02:12:DF\r\n"
      "a=setup:actpass\r\n"
      "a=mid:1\r\n"
      "a=sctp-port:5000\r\n"
      "a=max-message-size:262144\r\n");
    st = http("POST", "/offer", "application/sdp", offer2);
    printf("  info answer with a data channel: %d, %d bytes\n", st, (int)strlen(body));
    QT_CHECK(st == 200 && strstr(body, "m=application") && strstr(body, "a=sctp-port:") && !strstr(body, "m=application 0 "),
             "with --input: the answer accepts the data channel");
    QT_CHECK(waitpid(sp, &st, WNOHANG) == 0, "webrtc-screencast --input still running");
    stop(sp);
  }
  stop(lp);
  unlink(sock); char lk[310]; snprintf(lk, sizeof lk, "%s.lock", sock); unlink(lk); rmdir(rt);
  QT_END();
}
