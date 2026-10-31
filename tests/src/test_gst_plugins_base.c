/* gst-plugins-base: real pipelines through base elements with known results.
 *  - videotestsrc (red, 64x48 I420) -> videoconvert -> RGB appsink: 10 frames,
 *    the negotiated caps, the buffer size and the pixel colour;
 *  - audiotestsrc 8 kHz mono -> volume 0.5 -> audioconvert -> audioresample ->
 *    16 kHz stereo F32 appsink: the sample count doubles, the peak halves;
 *  - audiotestsrc -> vorbisenc -> oggmux -> file, then decodebin (oggdemux,
 *    vorbisdec, typefind) back to raw audio, gst-discoverer-1.0 on the file,
 *    and the pbutils codec description API. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>

static int have(const char *name) {
  GstElementFactory *f = gst_element_factory_find(name);
  if (f) { gst_object_unref(f); return 1; }
  printf("  info element \"%s\" is not registered\n", name);
  return 0;
}
/* Run a pipeline to EOS; returns 1 on EOS, 0 on error/timeout. */
static int run_to_eos(GstElement *p, int secs) {
  if (gst_element_set_state(p, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) return 0;
  GstBus *bus = gst_element_get_bus(p);
  GstMessage *m = gst_bus_timed_pop_filtered(bus, (GstClockTime)secs * GST_SECOND,
                                             GST_MESSAGE_EOS | GST_MESSAGE_ERROR);
  int ok = m && GST_MESSAGE_TYPE(m) == GST_MESSAGE_EOS;
  if (m && GST_MESSAGE_TYPE(m) == GST_MESSAGE_ERROR) {
    GError *e = NULL; gchar *dbg = NULL;
    gst_message_parse_error(m, &e, &dbg);
    printf("  info bus error: %s\n", e && e->message ? e->message : "?");
    g_clear_error(&e); g_free(dbg);
  } else if (!m) printf("  info timed out after %d s\n", secs);
  if (m) gst_message_unref(m);
  gst_object_unref(bus);
  return ok;
}

int main(int argc, char **argv) {
  QT_BEGIN("gst-plugins-base");
  gst_init(&argc, &argv);
  guint maj, min, mic, nano;
  gst_plugins_base_version(&maj, &min, &mic, &nano);
  printf("  info gst-plugins-base %u.%u.%u\n", maj, min, mic);
  const char *els[] = { "videotestsrc", "videoconvert", "audiotestsrc", "audioconvert", "audioresample",
                        "volume", "appsink", "vorbisenc", "vorbisdec", "oggmux", "oggdemux",
                        "decodebin", "typefind" };
  int all = 1;
  for (size_t i = 0; i < sizeof els / sizeof *els; i++) all &= have(els[i]);
  QT_CHECK(all, "all base elements used here are registered");

  /* ---- video ---- */
  GError *err = NULL;
  GstElement *vp = gst_parse_launch(
      "videotestsrc num-buffers=10 pattern=red ! video/x-raw,format=I420,width=64,height=48,framerate=30/1 "
      "! videoconvert ! video/x-raw,format=RGB ! appsink name=s sync=false", &err);
  QT_CHECK(vp != NULL, "video pipeline built");
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  if (vp) {
    GstElement *s = gst_bin_get_by_name(GST_BIN(vp), "s");
    gst_element_set_state(vp, GST_STATE_PLAYING);
    int frames = 0, caps_ok = 0, size_ok = 0, red_ok = 0;
    GstSample *smp;
    while ((smp = gst_app_sink_try_pull_sample(GST_APP_SINK(s), 10 * GST_SECOND)) != NULL) {
      if (frames == 0) {
        GstCaps *c = gst_sample_get_caps(smp);
        GstStructure *st = gst_caps_get_structure(c, 0);
        int w = 0, h = 0;
        gst_structure_get_int(st, "width", &w); gst_structure_get_int(st, "height", &h);
        const gchar *fmt = gst_structure_get_string(st, "format");
        printf("  info caps: %s %dx%d\n", fmt ? fmt : "(none)", w, h);
        caps_ok = fmt && !strcmp(fmt, "RGB") && w == 64 && h == 48;
        GstBuffer *b = gst_sample_get_buffer(smp);
        GstMapInfo mi;
        if (gst_buffer_map(b, &mi, GST_MAP_READ)) {
          printf("  info buffer %zu bytes, pixel(10,10) = %u,%u,%u\n", (size_t)mi.size,
                 mi.data[10 * 192 + 30], mi.data[10 * 192 + 31], mi.data[10 * 192 + 32]);
          size_ok = mi.size == 192 * 48;  /* RGB rows are 64*3 = 192 bytes, already 4-aligned */
          const guint8 *px = mi.data + 10 * 192 + 30;
          red_ok = px[0] > 230 && px[1] < 25 && px[2] < 25;
          gst_buffer_unmap(b, &mi);
        }
      }
      frames++;
      gst_sample_unref(smp);
    }
    printf("  info %d frame(s) pulled\n", frames);
    QT_CHECK(frames == 10, "exactly num-buffers=10 frames arrive");
    QT_CHECK(caps_ok, "negotiated RGB 64x48");
    QT_CHECK(size_ok, "frame size 64*3*48 = 9216 bytes");
    QT_CHECK(red_ok, "pattern=red converts to RGB red");
    QT_CHECK(gst_app_sink_is_eos(GST_APP_SINK(s)), "appsink reached EOS");
    gst_element_set_state(vp, GST_STATE_NULL);
    gst_object_unref(s); gst_object_unref(vp);
  }

  /* ---- audio: convert + resample + volume ---- */
  GstElement *ap = gst_parse_launch(
      "audiotestsrc num-buffers=8 samplesperbuffer=1000 wave=sine freq=200 volume=0.8 "
      "! audio/x-raw,format=S16LE,rate=8000,channels=1 ! volume volume=0.5 ! audioconvert ! audioresample "
      "! audio/x-raw,format=F32LE,rate=16000,channels=2,layout=interleaved ! appsink name=a sync=false", &err);
  QT_CHECK(ap != NULL, "audio pipeline built");
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  if (ap) {
    GstElement *a = gst_bin_get_by_name(GST_BIN(ap), "a");
    gst_element_set_state(ap, GST_STATE_PLAYING);
    size_t bytes = 0; float peak = 0; int bufs = 0, lr_same = 1;
    GstSample *smp;
    while ((smp = gst_app_sink_try_pull_sample(GST_APP_SINK(a), 10 * GST_SECOND)) != NULL) {
      GstBuffer *b = gst_sample_get_buffer(smp);
      GstMapInfo mi;
      if (gst_buffer_map(b, &mi, GST_MAP_READ)) {
        const float *f = (const float *)mi.data;
        for (size_t i = 0; i + 1 < mi.size / 4; i += 2) {
          if (fabsf(f[i]) > peak) peak = fabsf(f[i]);
          if (f[i] != f[i + 1]) lr_same = 0;
        }
        bytes += mi.size; gst_buffer_unmap(b, &mi);
      }
      bufs++; gst_sample_unref(smp);
    }
    size_t frames = bytes / 8;
    printf("  info %d buffer(s), %zu stereo frames at 16 kHz, peak %.3f\n", bufs, frames, peak);
    /* 8000 mono samples at 8 kHz = 1 s -> 16000 frames; the resampler may
       hold back a few frames of filter latency at the end. */
    QT_CHECK(frames >= 15900 && frames <= 16100, "8000 samples @ 8 kHz resample to ~16000 @ 16 kHz");
    QT_CHECK(peak > 0.37f && peak < 0.43f, "volume 0.8 * 0.5 -> peak ~0.4");
    QT_CHECK(lr_same, "mono upmixed to identical left/right");
    gst_element_set_state(ap, GST_STATE_NULL);
    gst_object_unref(a); gst_object_unref(ap);
  }

  /* ---- Ogg/Vorbis encode, decode, discover ---- */
  char dir[64] = "/tmp/qnxport-gstbase.XXXXXX";
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char path[128], desc[512];
  snprintf(path, sizeof path, "%s/tone.ogg", dir);
  snprintf(desc, sizeof desc,
           "audiotestsrc num-buffers=20 samplesperbuffer=2205 ! audio/x-raw,rate=44100,channels=1 "
           "! audioconvert ! vorbisenc ! oggmux ! filesink location=%s", path);
  GstElement *ep = gst_parse_launch(desc, &err);
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  QT_CHECK(ep && run_to_eos(ep, 30), "encoded 1 s of audio to Ogg/Vorbis");
  if (ep) { gst_element_set_state(ep, GST_STATE_NULL); gst_object_unref(ep); }
  FILE *f = fopen(path, "rb"); char magic[4] = {0};
  if (f) { if (fread(magic, 1, 4, f) != 4) magic[0] = 0; fclose(f); }
  QT_CHECK(!memcmp(magic, "OggS", 4), "file starts with OggS");

  snprintf(desc, sizeof desc,
           "filesrc location=%s ! decodebin ! audioconvert ! audio/x-raw,format=S16LE,channels=1 "
           "! appsink name=d sync=false", path);
  GstElement *dp = gst_parse_launch(desc, &err);
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  if (dp) {
    GstElement *d = gst_bin_get_by_name(GST_BIN(dp), "d");
    gst_element_set_state(dp, GST_STATE_PLAYING);
    size_t samples = 0; GstSample *smp;
    while ((smp = gst_app_sink_try_pull_sample(GST_APP_SINK(d), 10 * GST_SECOND)) != NULL) {
      samples += gst_buffer_get_size(gst_sample_get_buffer(smp)) / 2;
      gst_sample_unref(smp);
    }
    printf("  info decoded %zu samples\n", samples);
    QT_CHECK(samples >= 44100 - 1024 && samples <= 44100 + 1024,
             "decodebin (oggdemux + vorbisdec) returns ~44100 samples (1 s)");
    gst_element_set_state(dp, GST_STATE_NULL);
    gst_object_unref(d); gst_object_unref(dp);
  } else QT_CHECK(0, "decode pipeline built");

  gchar *uri = g_filename_to_uri(path, NULL, NULL);
  GstDiscoverer *disc = gst_discoverer_new(15 * GST_SECOND, &err);
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  GstDiscovererInfo *info = disc && uri ? gst_discoverer_discover_uri(disc, uri, &err) : NULL;
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  if (info) {
    GstClockTime dur = gst_discoverer_info_get_duration(info);
    GList *as = gst_discoverer_info_get_audio_streams(info);
    guint rate = as ? gst_discoverer_audio_info_get_sample_rate(as->data) : 0;
    printf("  info discoverer: %" GST_TIME_FORMAT ", %u audio stream(s), %u Hz\n", GST_TIME_ARGS(dur),
           g_list_length(as), rate);
    QT_CHECK(dur > 990 * GST_MSECOND && dur < 1010 * GST_MSECOND, "GstDiscoverer: duration 1.0 s");
    QT_CHECK(g_list_length(as) == 1 && rate == 44100, "GstDiscoverer: one 44100 Hz audio stream");
    gst_discoverer_stream_info_list_free(as);
    gst_discoverer_info_unref(info);
  } else QT_CHECK(0, "GstDiscoverer analysed the file");
  if (disc) g_object_unref(disc);
  g_free(uri);

  char cmd[256];
  snprintf(cmd, sizeof cmd, "gst-discoverer-1.0 %s > %s/disc.txt 2>&1", path, dir);
  int rc = system(cmd);
  printf("  info $ gst-discoverer-1.0 tone.ogg -> %d\n", rc);
  snprintf(cmd, sizeof cmd, "%s/disc.txt", dir);
  char out[8192] = {0};
  f = fopen(cmd, "r");
  if (f) { size_t n = fread(out, 1, sizeof out - 1, f); out[n] = 0; fclose(f); }
  QT_CHECK(rc == 0 && (strstr(out, "Duration: 0:00:01.0") || strstr(out, "Duration: 0:00:00.99")),
           "gst-discoverer-1.0 prints a 1 s duration");
  QT_CHECK(strstr(out, "Vorbis") || strstr(out, "vorbis"), "gst-discoverer-1.0 names the Vorbis stream");

  GstCaps *vc = gst_caps_from_string("audio/x-vorbis");
  gchar *cd = gst_pb_utils_get_codec_description(vc);
  printf("  info codec description of audio/x-vorbis: %s\n", cd ? cd : "(null)");
  QT_CHECK(cd && strstr(cd, "Vorbis"), "pbutils codec description");
  g_free(cd); gst_caps_unref(vc);

  snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd);
  QT_END();
}
