#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rfb/rfb.h>
#include <rfb/rfbclient.h>
/* LibVNCServer and LibVNCClient on the loopback: a server shows a known
 * 256x128 image, a client connects and must receive it -- exactly with raw
 * and with lossless Tight encoding, and within JPEG error with Tight+JPEG,
 * the encoding a TurboVNC server (VirtualGL's usual partner) sends. */
#define W 256
#define H 128
static int updates; static long covered;
/* The first updates may carry only protocol extensions (no pixels): count
 * the area the rectangles cover. */
static void done(rfbClient *c, int x, int y, int w, int h) { (void)c; (void)x; (void)y; covered += (long)w * h; }
static void finished(rfbClient *c) { (void)c; updates++; }
static int fetch(int port, const char *enc, int jpeg, const unsigned char *want, double *err) {
  rfbClient *c = rfbGetClient(8, 3, 4);
  c->serverHost = strdup("127.0.0.1"); c->serverPort = port;
  c->appData.encodingsString = enc; c->appData.enableJPEG = jpeg ? TRUE : FALSE;
  c->appData.qualityLevel = 9; c->appData.compressLevel = 6;
  c->GotFrameBufferUpdate = done; c->FinishedFrameBufferUpdate = finished;
  if (!rfbInitClient(c, NULL, NULL)) return -1;
  updates = 0; covered = 0;
  for (int i = 0; i < 200 && covered < (long)W * H; i++) {
    int r = WaitForMessage(c, 50000);
    if (r < 0 || (r > 0 && !HandleRFBServerMessage(c))) break;
  }
  /* Pseudo-rectangles (desktop size) count too: read what else comes. */
  for (int i = 0; i < 10; i++) {
    int r = WaitForMessage(c, 50000);
    if (r < 0 || (r > 0 && !HandleRFBServerMessage(c))) break;
  }
  int ok = covered >= (long)W * H && c->width == W && c->height == H;
  double e = 0;
  if (ok) for (int i = 0; i < W * H * 4; i++) if (i % 4 != 3) e += abs(c->frameBuffer[i] - want[i]);
  *err = ok ? e / (W * H * 3) : 1e9;
  rfbClientCleanup(c);
  return ok;
}
int main(void) {
  QT_BEGIN("libvncserver");
  int port = 5900 + 30 + getpid() % 60;
  /* The library logs from its own threads, between this test's lines: quiet. */
  rfbLogEnable(0);
  rfbScreenInfoPtr s = rfbGetScreen(NULL, NULL, W, H, 8, 3, 4);
  QT_CHECK(s != NULL, "rfbGetScreen");
  unsigned char *fb = malloc(W * H * 4);
  for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {   /* smooth gradients: JPEG-friendly */
    unsigned char *p = fb + (y * W + x) * 4; p[0] = x; p[1] = y * 2; p[2] = (x + y) / 2; p[3] = 0;
  }
  s->frameBuffer = (char *)fb; s->port = port; s->ipv6port = 0; s->listenInterface = htonl(INADDR_LOOPBACK);
  s->alwaysShared = TRUE;
  s->cursor = NULL;   /* no cursor drawn into the image */
  rfbInitServer(s);
  QT_CHECK(rfbIsActive(s), "server listening on 127.0.0.1");
  rfbRunEventLoop(s, -1, TRUE);   /* its own thread */
  double e;
  int r = fetch(port, "raw", 0, fb, &e);
  printf("  info raw: mean error %.3f\n", e);
  QT_CHECK(r == 1 && e == 0, "raw: the image, exactly");
  r = fetch(port, "tight", 0, fb, &e);
  printf("  info tight (lossless): mean error %.3f\n", e);
  QT_CHECK(r == 1 && e == 0, "Tight, lossless: the image, exactly");
  r = fetch(port, "tight", 1, fb, &e);
  printf("  info tight+jpeg q9: mean error %.3f\n", e);
  QT_CHECK(r == 1 && e < 4.0, "Tight+JPEG (TurboVNC's encoding): within JPEG error");
  r = fetch(port, "zrle", 0, fb, &e);
  QT_CHECK(r == 1 && e == 0, "ZRLE: the image, exactly");
  rfbShutdownServer(s, TRUE); rfbScreenCleanup(s);
  QT_END();
}
