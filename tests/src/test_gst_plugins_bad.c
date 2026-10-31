/* gst-plugins-bad: H.264 with OpenH264 (openh264enc -> h264parse) checked at
 * the bitstream level with the set's own codecparsers library (SPS: 160x120,
 * a known profile), then carried through the MPEG-TS muxer and demuxer and
 * decoded by openh264dec (10 frames of 160x120 back); an audio effect
 * (freeverb) produces stereo output from mono. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>
#include <gst/codecparsers/gsth264parser.h>

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
  QT_BEGIN("gst-plugins-bad");
  alarm(120);
  setenv("GST_PLUGIN_PATH", "/usr/local/lib/gstreamer-1.0", 0);
  setenv("GST_REGISTRY", "/tmp/qnxport-gst-bad.registry", 0);
  setenv("GST_REGISTRY_FORK", "no", 0);
  gst_init(&argc, &argv);
  QT_CHECK(have("openh264enc") && have("openh264dec"), "openh264 elements");
  QT_CHECK(have("h264parse") && have("mpegtsmux") && have("tsdemux"), "videoparsersbad and MPEG-TS elements");
  QT_CHECK(have("freeverb") && have("y4mdec") && have("x265enc"), "freeverb, y4mdec, x265enc");

  struct run e = run("videotestsrc num-buffers=10 ! video/x-raw,format=I420,width=160,height=120,framerate=25/1"
                     " ! openh264enc ! h264parse ! video/x-h264,stream-format=byte-stream,alignment=au"
                     " ! appsink name=sink sync=false", 0);
  printf("  info encoded: %d access unit(s), %llu bytes\n", e.bufs, (unsigned long long)e.bytes);
  QT_CHECK(e.bufs == 10, "10 H.264 access units");
  if (e.mid) {
    GstMapInfo mi;
    gst_buffer_map(e.mid, &mi, GST_MAP_READ);
    GstH264NalParser *np = gst_h264_nal_parser_new();
    GstH264NalUnit nalu;
    guint off = 0;
    int got_sps = 0, idr = 0;
    GstH264SPS sps;
    GstH264ParserResult pr;
    while ((pr = gst_h264_parser_identify_nalu(np, mi.data, off, mi.size, &nalu)) == GST_H264_PARSER_OK ||
           pr == GST_H264_PARSER_NO_NAL_END) {
      if (nalu.type == GST_H264_NAL_SPS && gst_h264_parser_parse_sps(np, &nalu, &sps) == GST_H264_PARSER_OK) got_sps = 1;
      if (nalu.type == GST_H264_NAL_SLICE_IDR) idr = 1;
      if (pr == GST_H264_PARSER_NO_NAL_END || nalu.offset + nalu.size >= mi.size) break;
      off = nalu.offset + nalu.size;
    }
    int w = got_sps ? (sps.frame_cropping_flag ? sps.crop_rect_width : sps.width) : 0;
    int h = got_sps ? (sps.frame_cropping_flag ? sps.crop_rect_height : sps.height) : 0;
    printf("  info SPS: profile_idc %d, %dx%d\n", got_sps ? sps.profile_idc : -1, w, h);
    QT_CHECK(got_sps && w == 160 && h == 120, "codecparsers reads the SPS: 160x120");
    QT_CHECK(got_sps && (sps.profile_idc == 66 || sps.profile_idc == 77 || sps.profile_idc == 100), "baseline/main/high profile");
    QT_CHECK(idr, "first access unit holds an IDR slice");
    gst_h264_nal_parser_free(np);
    gst_buffer_unmap(e.mid, &mi);
    gst_buffer_unref(e.mid);
  } else QT_CHECK(0, "first access unit captured");
  if (e.caps) gst_caps_unref(e.caps);

  struct run d = run("videotestsrc num-buffers=10 ! video/x-raw,format=I420,width=160,height=120,framerate=25/1"
                     " ! openh264enc ! h264parse ! mpegtsmux ! tsdemux ! h264parse ! openh264dec"
                     " ! appsink name=sink sync=false", 3);
  GstVideoInfo vi;
  printf("  info decoded after TS: %d frame(s)\n", d.bufs);
  QT_CHECK(d.bufs == 10 && d.eos_ok, "10 frames through mpegtsmux/tsdemux/openh264dec");
  QT_CHECK(d.caps && gst_video_info_from_caps(&vi, d.caps) && vi.width == 160 && vi.height == 120, "decoded 160x120");
  if (d.caps) gst_caps_unref(d.caps);
  if (d.mid) gst_buffer_unref(d.mid);

  struct run a = run("audiotestsrc num-buffers=20 ! audio/x-raw,format=F32LE,channels=1,rate=48000 ! freeverb"
                     " ! audio/x-raw,channels=2 ! appsink name=sink sync=false", 5);
  GstAudioInfo ai;
  QT_CHECK(a.bufs == 20 && a.caps && gst_audio_info_from_caps(&ai, a.caps) && ai.channels == 2, "freeverb: mono in, stereo out");
  if (a.caps) gst_caps_unref(a.caps);
  if (a.mid) gst_buffer_unref(a.mid);
  unlink("/tmp/qnxport-gst-bad.registry");
  QT_END();
}
