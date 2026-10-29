/* GStreamer: build and run a real pipeline. videotestsrc -> videoconvert ->
 * x264enc -> avdec_h264 -> appsink covers the core, the base/good/ugly plugin
 * sets and gst-libav in one go; the test falls back to a core-only pipeline
 * if an element is missing so the failure says which plugin is absent. */
#include "common.h"
#include <stdlib.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
static int have(const char *name) {
  GstElementFactory *f = gst_element_factory_find(name);
  if (f) { gst_object_unref(f); return 1; }
  printf("  info element \"%s\" is not registered\n", name);
  return 0;
}
int main(int argc, char **argv) {
  QT_BEGIN("gstreamer");
  gst_init(&argc, &argv);
  guint maj, min, mic, nano;
  gst_version(&maj, &min, &mic, &nano);
  printf("  info GStreamer %u.%u.%u\n", maj, min, mic);
  QT_CHECK(gst_is_initialized(), "gst_init");

  GstRegistry *reg = gst_registry_get();
  GList *plugins = gst_registry_get_plugin_list(reg);
  printf("  info %d plugin(s) in the registry\n", g_list_length(plugins));
  QT_CHECK(g_list_length(plugins) > 5, "plugins were found and loaded");
  gst_plugin_list_free(plugins);

  QT_CHECK(have("videotestsrc"), "videotestsrc (gst-plugins-base)");
  QT_CHECK(have("videoconvert"), "videoconvert (gst-plugins-base)");
  QT_CHECK(have("appsink"), "appsink (gst-plugins-base)");
  int h264 = have("x264enc") && have("avdec_h264");
  QT_CHECK(h264, "x264enc (gst-plugins-ugly) and avdec_h264 (gst-libav)");
  QT_CHECK(have("x265enc"), "x265enc (gst-plugins-bad)");
  QT_CHECK(have("av1enc") || have("svtav1enc"), "an AV1 encoder (gst-plugins-bad)");
  QT_CHECK(have("openh264dec"), "openh264dec (gst-plugins-bad)");

  const char *desc = h264
    ? "videotestsrc num-buffers=10 ! video/x-raw,width=176,height=144,framerate=25/1"
      " ! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency"
      " ! h264parse ! avdec_h264 ! videoconvert ! appsink name=sink sync=false"
    : "videotestsrc num-buffers=10 ! video/x-raw,width=176,height=144,framerate=25/1"
      " ! videoconvert ! appsink name=sink sync=false";
  GError *err = NULL;
  GstElement *pipe = gst_parse_launch(desc, &err);
  QT_CHECK(pipe != NULL, "gst_parse_launch");
  if (!pipe) { printf("  info %s\n", err ? err->message : "?"); QT_END(); }
  GstElement *sink = gst_bin_get_by_name(GST_BIN(pipe), "sink");
  QT_CHECK(sink != NULL, "appsink found in the pipeline");

  GstStateChangeReturn sr = gst_element_set_state(pipe, GST_STATE_PLAYING);
  QT_CHECK(sr != GST_STATE_CHANGE_FAILURE, "pipeline reached PLAYING");
  int frames = 0;
  for (;;) {
    GstSample *s = gst_app_sink_try_pull_sample(GST_APP_SINK(sink), 5 * GST_SECOND);
    if (!s) break;
    GstBuffer *buf = gst_sample_get_buffer(s);
    if (buf && gst_buffer_get_size(buf) > 0) ++frames;
    gst_sample_unref(s);
    if (gst_app_sink_is_eos(GST_APP_SINK(sink))) break;
  }
  printf("  info %d frame(s) reached appsink\n", frames);
  QT_CHECK(frames >= 5, "frames flowed through the pipeline");

  GstBus *bus = gst_element_get_bus(pipe);
  GstMessage *m = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
  QT_CHECK(m == NULL, "no error message on the bus");
  if (m) {
    GError *e = NULL; gchar *dbg = NULL;
    gst_message_parse_error(m, &e, &dbg);
    printf("  info %s (%s)\n", e ? e->message : "?", dbg ? dbg : "");
    gst_message_unref(m);
  }
  gst_object_unref(bus);
  gst_element_set_state(pipe, GST_STATE_NULL);
  gst_object_unref(sink);
  gst_object_unref(pipe);
  QT_END();
}
