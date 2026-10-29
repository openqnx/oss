/* dav1d: open a decoder and feed it an OBU sequence header. A full AV1 frame
 * needs an encoder; what a port has to prove is that the decoder initialises
 * (its threading and CPU-feature detection run here) and parses valid input. */
#include "common.h"
#include <stdlib.h>
#include <dav1d/dav1d.h>
static void nofree(const uint8_t *p, void *c) { (void)p; (void)c; }
int main(void) {
  QT_BEGIN("dav1d");
  printf("  info dav1d %s\n", dav1d_version());
  Dav1dSettings s;
  dav1d_default_settings(&s);
  s.n_threads = 1;
  QT_CHECK(s.n_threads == 1, "settings are writable");
  Dav1dContext *c = NULL;
  int rc = dav1d_open(&c, &s);
  QT_CHECK(rc == 0 && c != NULL, "dav1d_open");
  if (rc != 0) QT_END();
  /* A minimal AV1 temporal delimiter OBU: header 0x12, size 0x00. */
  static const uint8_t td[] = { 0x12, 0x00 };
  Dav1dData data;
  memset(&data, 0, sizeof data);
  QT_CHECK(dav1d_data_wrap(&data, td, sizeof td, nofree, NULL) == 0, "dav1d_data_wrap");
  rc = dav1d_send_data(c, &data);
  QT_CHECK(rc == 0, "decoder accepts a temporal delimiter OBU");
  Dav1dPicture pic;
  memset(&pic, 0, sizeof pic);
  rc = dav1d_get_picture(c, &pic);
  QT_CHECK(rc == DAV1D_ERR(EAGAIN), "no picture yet (more data needed)");
  /* Garbage must be rejected rather than crash. */
  static const uint8_t junk[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  Dav1dData bad;
  memset(&bad, 0, sizeof bad);
  dav1d_data_wrap(&bad, junk, sizeof junk, nofree, NULL);
  dav1d_send_data(c, &bad);
  dav1d_data_unref(&bad);
  dav1d_flush(c);
  dav1d_close(&c);
  QT_CHECK(c == NULL, "dav1d_close cleared the context");
  QT_END();
}
