#include "common.h"
#include <string.h>
#include <unistd.h>
#include <rfb/rfb.h>
#include <gvnc.h>
/* gtk-vnc's client library (gvnc, under the gvncviewer window) against a
 * LibVNCServer in this process on the loopback: it must authenticate, read
 * the desktop's name and size, and receive the known image exactly (ZRLE),
 * driven by GLib's main loop as gvncviewer is. */
#define W 192
#define H 96
static unsigned char *srv_fb, *cli_fb;
static long covered; static int inited; static char name[64];
static GMainLoop *loop;
static void on_init(VncConnection *c, gpointer d) {
  (void)d; inited = 1; snprintf(name, sizeof name, "%s", vnc_connection_get_name(c));
  const VncPixelFormat *rf = vnc_connection_get_pixel_format(c);
  VncBaseFramebuffer *fb = vnc_base_framebuffer_new(cli_fb, vnc_connection_get_width(c), vnc_connection_get_height(c), W * 4, rf, rf);
  vnc_connection_set_framebuffer(c, VNC_FRAMEBUFFER(fb));
  g_object_unref(fb);
  gint32 enc[] = { VNC_CONNECTION_ENCODING_ZRLE, VNC_CONNECTION_ENCODING_RAW };
  vnc_connection_set_encodings(c, 2, enc);
  vnc_connection_framebuffer_update_request(c, FALSE, 0, 0, W, H);
}
static void on_update(VncConnection *c, guint16 x, guint16 y, guint16 w, guint16 h, gpointer d) {
  (void)c; (void)x; (void)y; (void)d; covered += (long)w * h;
  if (covered >= (long)W * H) g_main_loop_quit(loop);
}
static void on_auth(VncConnection *c, GValueArray *types, gpointer d) { (void)types; (void)d; vnc_connection_set_auth_type(c, VNC_CONNECTION_AUTH_NONE); }
static gboolean on_timeout(gpointer d) { (void)d; g_main_loop_quit(loop); return FALSE; }
int main(void) {
  QT_BEGIN("gtk-vnc");
  int port = 5900 + 100 + getpid() % 60;
  rfbScreenInfoPtr s = rfbGetScreen(NULL, NULL, W, H, 8, 3, 4);
  srv_fb = malloc(W * H * 4); cli_fb = calloc(W * H, 4);
  for (int i = 0; i < W * H; i++) { srv_fb[i*4] = i % W; srv_fb[i*4+1] = (i / W) * 2; srv_fb[i*4+2] = 0x5a; srv_fb[i*4+3] = 0; }
  s->frameBuffer = (char *)srv_fb; s->port = port; s->ipv6port = 0; s->listenInterface = htonl(INADDR_LOOPBACK);
  s->cursor = NULL; s->desktopName = "QNX test desktop"; rfbLogEnable(0);
  rfbInitServer(s); rfbRunEventLoop(s, -1, TRUE);
  VncConnection *c = vnc_connection_new();
  g_signal_connect(c, "vnc-initialized", G_CALLBACK(on_init), NULL);
  g_signal_connect(c, "vnc-framebuffer-update", G_CALLBACK(on_update), NULL);
  g_signal_connect(c, "vnc-auth-choose-type", G_CALLBACK(on_auth), NULL);
  char ps[8]; snprintf(ps, sizeof ps, "%d", port);
  loop = g_main_loop_new(NULL, FALSE);
  QT_CHECK(vnc_connection_open_host(c, "127.0.0.1", ps), "vnc_connection_open_host");
  g_timeout_add_seconds(10, on_timeout, NULL);
  g_main_loop_run(loop);
  QT_CHECK(inited, "connected and initialised");
  printf("  info desktop \"%s\" %dx%d\n", name, vnc_connection_get_width(c), vnc_connection_get_height(c));
  QT_CHECK(!strcmp(name, "QNX test desktop") && vnc_connection_get_width(c) == W && vnc_connection_get_height(c) == H, "name and size");
  QT_CHECK(covered >= (long)W * H, "whole screen updated");
  int diff = 0; for (int i = 0; i < W * H * 4; i++) if (i % 4 != 3 && cli_fb[i] != srv_fb[i]) diff++;
  printf("  info %d differing bytes\n", diff);
  QT_CHECK(diff == 0, "the image, exactly");
  vnc_connection_shutdown(c); g_object_unref(c);
  rfbShutdownServer(s, TRUE); rfbScreenCleanup(s);
  QT_END();
}
