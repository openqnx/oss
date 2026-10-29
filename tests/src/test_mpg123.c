/* libmpg123: library init, decoder list, and a feed-mode handle. */
#include "common.h"
#include <mpg123.h>
int main(void) {
  QT_BEGIN("mpg123");
  int err; mpg123_handle *h = mpg123_new(NULL, &err);
  QT_CHECK(h != NULL, "handle");
  const char **d = mpg123_decoders();
  QT_CHECK(d && d[0], "has a decoder");
  QT_CHECK(mpg123_open_feed(h) == MPG123_OK, "open feed");
  size_t done; unsigned char junk[64] = {0}, out[1024];
  QT_CHECK(mpg123_decode(h, junk, sizeof junk, out, sizeof out, &done) == MPG123_NEED_MORE, "needs more data for garbage input");
  mpg123_delete(h);
  QT_END();
}
