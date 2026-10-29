/* gst-plugins-good: the LGPL plugin set. test_gstreamer covers base, ugly, bad
 * and libav through one pipeline but never names a "good" element, so this
 * test registers the set explicitly and runs audio through two of them. */
#include "common.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
static int have(const char *name) {
  GstElementFactory *f = gst_element_factory_find(name);
  if (f) { gst_object_unref(f); return 1; }
  printf("  info element \"%s\" is not registered\n", name);
  return 0;
}
int main(int argc, char **argv) {
  QT_BEGIN("gst-plugins-good");
  gst_init(&argc, &argv);
  QT_CHECK(have("level"),         "level (good: audio level meter)");
  QT_CHECK(have("audiopanorama"), "audiopanorama (good: audiofx)");
  QT_CHECK(have("videoflip"),     "videoflip (good: videofilter)");
  QT_CHECK(have("videocrop"),     "videocrop (good: videocrop)");
  QT_CHECK(have("wavparse"),      "wavparse (good: wavparse)");
  QT_CHECK(have("qtmux") || have("mp4mux"), "mp4 muxer (good: isomp4)");

  /* Run audio through two good elements and make sure buffers come out. */
  GError *err = NULL;
  GstElement *pipe = gst_parse_launch(
      "audiotestsrc num-buffers=10 ! audioconvert ! audiopanorama panorama=0.5"
      " ! level ! audioconvert ! appsink name=sink sync=false", &err);
  QT_CHECK(pipe != NULL && err == NULL, "build pipeline through good elements");
  if (err) { printf("  info %s\n", err->message); g_error_free(err); }
  if (pipe) {
    GstElement *sink = gst_bin_get_by_name(GST_BIN(pipe), "sink");
    QT_CHECK(gst_element_set_state(pipe, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE,
             "pipeline reaches PLAYING");
    int got = 0;
    for (int i = 0; i < 10; ++i) {
      GstSample *s = gst_app_sink_pull_sample(GST_APP_SINK(sink));
      if (!s) break;
      ++got; gst_sample_unref(s);
    }
    printf("  info pulled %d buffer(s)\n", got);
    QT_CHECK(got > 0, "audio flows through audiopanorama and level");
    gst_element_set_state(pipe, GST_STATE_NULL);
    gst_object_unref(sink); gst_object_unref(pipe);
  }
  QT_END();
}
