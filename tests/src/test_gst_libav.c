/* gst-libav: FFmpeg's codecs as GStreamer elements. Video: videotestsrc ->
 * avenc_mpeg4 -> avdec_mpeg4 -> appsink, 10 frames of a white picture come
 * back as 10 decoded 160x120 frames, still white. Audio: a 440 Hz tone
 * through avenc_aac -> avdec_aac keeps its sample rate and returns about the
 * same number of samples. Also avdemux/avmux element registration. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>

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
  QT_BEGIN("gst-libav");
  alarm(120);
  setenv("GST_PLUGIN_PATH", "/usr/local/lib/gstreamer-1.0", 0);
  setenv("GST_REGISTRY", "/tmp/qnxport-gst-libav.registry", 0);
  setenv("GST_REGISTRY_FORK", "no", 0);
  gst_init(&argc, &argv);
  GstPlugin *pl = gst_registry_find_plugin(gst_registry_get(), "libav");
  QT_CHECK(pl != NULL, "libav plugin registered");
  if (pl) { printf("  info gst-libav %s\n", gst_plugin_get_version(pl)); gst_object_unref(pl); }
  QT_CHECK(have("avenc_mpeg4") && have("avdec_mpeg4"), "MPEG-4 Part 2 encoder and decoder");
  QT_CHECK(have("avdec_h264") && have("avenc_aac") && have("avdec_aac"), "H.264 decoder, AAC encoder/decoder");
  QT_CHECK(have("avmux_matroska") || have("avmux_mp4"), "a libavformat muxer element");

  struct run v = run("videotestsrc num-buffers=10 pattern=white ! video/x-raw,format=I420,width=160,height=120,framerate=25/1"
                     " ! avenc_mpeg4 bitrate=800000 ! avdec_mpeg4 ! video/x-raw,format=I420 ! appsink name=sink sync=false", 5);
  printf("  info video: %d frame(s)\n", v.bufs);
  QT_CHECK(v.bufs == 10 && v.eos_ok, "10 frames encoded, decoded, EOS reached");
  GstVideoInfo vi;
  QT_CHECK(v.caps && gst_video_info_from_caps(&vi, v.caps) && vi.width == 160 && vi.height == 120, "decoded caps 160x120");
  if (v.mid && v.caps) {
    GstMapInfo mi;
    gst_buffer_map(v.mid, &mi, GST_MAP_READ);
    unsigned long sum = 0;
    for (int i = 0; i < 160 * 120; ++i) sum += mi.data[i];
    double mean = (double)sum / (160 * 120);
    printf("  info mean luma %.1f\n", mean);
    QT_CHECK(mean > 225 && mean < 245, "decoded picture is white (Y ~ 235)");
    gst_buffer_unmap(v.mid, &mi);
  }
  if (v.caps) gst_caps_unref(v.caps);
  if (v.mid) gst_buffer_unref(v.mid);

  /* 1 s of 44.1 kHz mono in 1024-sample buffers */
  struct run a = run("audiotestsrc num-buffers=43 samplesperbuffer=1024 freq=440 ! audio/x-raw,rate=44100,channels=1"
                     " ! audioconvert ! avenc_aac ! avdec_aac ! audioconvert ! audio/x-raw,format=S16LE"
                     " ! appsink name=sink sync=false", 20);
  GstAudioInfo ai;
  int have_ai = a.caps && gst_audio_info_from_caps(&ai, a.caps);
  guint64 samples = have_ai && ai.bpf ? a.bytes / ai.bpf : 0;
  printf("  info audio: %d buffer(s), %llu samples\n", a.bufs, (unsigned long long)samples);
  QT_CHECK(have_ai && ai.rate == 44100, "AAC round trip keeps 44.1 kHz");
  QT_CHECK(samples >= 43 * 1024 - 2048 && samples <= 43 * 1024 + 2048, "about 44032 samples come back");
  if (a.mid && have_ai) {
    GstMapInfo mi;
    gst_buffer_map(a.mid, &mi, GST_MAP_READ);
    const gint16 *s = (const gint16 *)mi.data;
    int peak = 0;
    for (gsize i = 0; i < mi.size / 2; ++i) if (abs(s[i]) > peak) peak = abs(s[i]);
    printf("  info audio peak %d\n", peak);
    QT_CHECK(peak > 8000, "decoded audio is a real signal, not silence");
    gst_buffer_unmap(a.mid, &mi);
  }
  if (a.caps) gst_caps_unref(a.caps);
  if (a.mid) gst_buffer_unref(a.mid);
  unlink("/tmp/qnxport-gst-libav.registry");
  QT_END();
}
