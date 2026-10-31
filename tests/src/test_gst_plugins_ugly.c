/* gst-plugins-ugly: x264enc. Ten 160x120 frames are encoded to an H.264
 * byte stream whose first access unit starts with a start code and carries
 * SPS/PPS/IDR NAL units (checked by hand); the stream is then decoded (by
 * avdec_h264 from gst-libav, or openh264dec from gst-plugins-bad) back to
 * ten 160x120 frames. Also: the set's other elements are registered. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>

static int have(const char *name) {
  GstElementFactory *f = gst_element_factory_find(name);
  if (f) { gst_object_unref(f); return 1; }
  printf("  info element \"%s\" is not registered\n", name);
  return 0;
}
/* Run a pipeline ending in appsink "sink"; return buffers seen, and the
   caps and a copy of the middle buffer for inspection. */
struct run { int bufs; guint64 bytes; GstCaps *caps; GstBuffer *mid; int eos_ok; };
static struct run run(const char *desc, int expect_mid) {
  struct run r = {0, 0, NULL, NULL, 0};
  GError *err = NULL;
  GstElement *p = gst_parse_launch(desc, &err);
  if (!p) { printf("  info parse error: %s\n", err ? err->message : "?"); if (err) g_error_free(err); return r; }
  GstElement *sink = gst_bin_get_by_name(GST_BIN(p), "sink");
  if (gst_element_set_state(p, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) printf("  info state change failed\n");
  for (;;) {
    GstSample *s = gst_app_sink_try_pull_sample(GST_APP_SINK(sink), 10 * GST_SECOND);
    if (!s) break;
    GstBuffer *b = gst_sample_get_buffer(s);
    if (!r.caps) r.caps = gst_caps_copy(gst_sample_get_caps(s));
    if (b) { r.bytes += gst_buffer_get_size(b); if (r.bufs == expect_mid && !r.mid) r.mid = gst_buffer_copy(b); }
    ++r.bufs;
    gst_sample_unref(s);
  }
  r.eos_ok = gst_app_sink_is_eos(GST_APP_SINK(sink));
  GstBus *bus = gst_element_get_bus(p);
  GstMessage *m = gst_bus_pop_filtered(bus, GST_MESSAGE_ERROR);
  if (m) { GError *e; gchar *dbg; gst_message_parse_error(m, &e, &dbg); printf("  info pipeline error: %s\n", e->message);
           g_error_free(e); g_free(dbg); gst_message_unref(m); }
  gst_object_unref(bus);
  gst_element_set_state(p, GST_STATE_NULL);
  gst_object_unref(sink); gst_object_unref(p);
  return r;
}

int main(int argc, char **argv) {
  QT_BEGIN("gst-plugins-ugly");
  alarm(120);
  setenv("GST_PLUGIN_PATH", "/usr/local/lib/gstreamer-1.0", 0);
  setenv("GST_REGISTRY", "/tmp/qnxport-gst-ugly.registry", 0);
  setenv("GST_REGISTRY_FORK", "no", 0);
  gst_init(&argc, &argv);
  QT_CHECK(have("x264enc"), "x264enc (x264)");
  QT_CHECK(have("asfdemux") && have("rmdemux") && have("dvdlpcmdec") && have("dvdsubdec"), "asf, realmedia, dvdlpcmdec, dvdsub");

  struct run e = run("videotestsrc num-buffers=10 ! video/x-raw,format=I420,width=160,height=120,framerate=25/1"
                     " ! x264enc speed-preset=ultrafast tune=zerolatency key-int-max=30"
                     " ! video/x-h264,stream-format=byte-stream,alignment=au ! appsink name=sink sync=false", 0);
  printf("  info %d access unit(s), %llu bytes\n", e.bufs, (unsigned long long)e.bytes);
  QT_CHECK(e.bufs == 10 && e.eos_ok, "10 access units, EOS");
  if (e.mid) {
    GstMapInfo mi;
    gst_buffer_map(e.mid, &mi, GST_MAP_READ);
    int sc = mi.size > 4 && mi.data[0] == 0 && mi.data[1] == 0 && (mi.data[2] == 1 || (mi.data[2] == 0 && mi.data[3] == 1));
    int types = 0;
    for (gsize i = 0; i + 3 < mi.size; ++i)
      if (mi.data[i] == 0 && mi.data[i + 1] == 0 && mi.data[i + 2] == 1) {
        int t = mi.data[i + 3] & 0x1f;
        types |= t == 7 ? 1 : t == 8 ? 2 : t == 5 ? 4 : 0;
      }
    QT_CHECK(sc, "byte stream begins with a start code");
    QT_CHECK(types == 7, "first access unit has SPS, PPS and an IDR slice");
    gst_buffer_unmap(e.mid, &mi);
    gst_buffer_unref(e.mid);
  }
  if (e.caps) gst_caps_unref(e.caps);

  const char *dec = have("avdec_h264") ? "avdec_h264" : have("openh264dec") ? "openh264dec" : NULL;
  QT_CHECK(dec != NULL, "an H.264 decoder is available");
  if (dec) {
    char desc[512];
    snprintf(desc, sizeof desc, "videotestsrc num-buffers=10 ! video/x-raw,format=I420,width=160,height=120,framerate=25/1"
             " ! x264enc speed-preset=ultrafast tune=zerolatency ! h264parse ! %s ! appsink name=sink sync=false", dec);
    struct run d = run(desc, 3);
    GstVideoInfo vi;
    printf("  info %s decoded %d frame(s)\n", dec, d.bufs);
    QT_CHECK(d.bufs == 10 && d.caps && gst_video_info_from_caps(&vi, d.caps) && vi.width == 160 && vi.height == 120,
             "x264 stream decodes to 10 frames of 160x120");
    if (d.caps) gst_caps_unref(d.caps);
    if (d.mid) gst_buffer_unref(d.mid);
  }
  unlink("/tmp/qnxport-gst-ugly.registry");
  QT_END();
}
