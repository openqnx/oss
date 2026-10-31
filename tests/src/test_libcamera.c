/* libcamera (Raspberry Pi IPA for QNX, librpi_camera_ipa): the Raspberry Pi
 * auto-exposure / auto-white-balance algorithms, run on synthetic ISP
 * statistics. A minimal VC4 (Pi 4) tuning file is written to /tmp, the IPA
 * is started for an imx708 at 1920x1080, and every frame is fed bcm2835 ISP
 * statistics of a scene whose red channel is half and blue channel two
 * thirds of green. Grey-world AWB must converge to gains of about 2.0 (red)
 * and 1.5 (blue) relative to green, and AGC must return a usable exposure
 * and gain. No camera hardware is involved. (aarch64 only, like the port.)
 * The library logs through slog2, so slogger2 must be running. */
#include "common.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rpi_camera_ipa.h>

/* struct bcm2835_isp_stats, from include/linux/bcm2835-isp.h */
#define NUM_HISTOGRAM_BINS 128
struct hist { uint32_t r[NUM_HISTOGRAM_BINS], g[NUM_HISTOGRAM_BINS], b[NUM_HISTOGRAM_BINS]; };
struct region { uint32_t counted, notcounted; uint64_t r_sum, g_sum, b_sum; };
struct focus { uint64_t contrast_val[2][2]; uint32_t contrast_val_num[2][2]; };
struct bcm2835_isp_stats {
  uint32_t version, size;
  struct hist hist[2];
  struct region awb_stats[16 * 12];
  struct region floating_stats[16];
  struct region agc_stats[16];
  struct focus focus_stats[12];
};

/* rpi.agc and rpi.awb from src/ipa/rpi/vc4/data/uncalibrated.json, AWB made
   to react within a frame (speed 1). */
static const char *TUNING =
  "{ \"version\": 2.0, \"target\": \"bcm2835\", \"algorithms\": [\n"
  " { \"rpi.awb\": { \"use_derivatives\": 0, \"bayes\": 0, \"speed\": 1.0, \"frame_period\": 1, \"startup_frames\": 1 } },\n"
  " { \"rpi.agc\": {\n"
  "   \"metering_modes\": { \"centre-weighted\": { \"weights\": [ 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0 ] } },\n"
  "   \"exposure_modes\": { \"normal\": { \"shutter\": [ 100, 15000, 30000, 60000, 120000 ],\n"
  "                                     \"gain\": [ 1.0, 2.0, 3.0, 4.0, 6.0 ] } },\n"
  "   \"constraint_modes\": { \"normal\": [ { \"bound\": \"LOWER\", \"q_lo\": 0.98, \"q_hi\": 1.0,\n"
  "                                        \"y_target\": [ 0, 0.4, 1000, 0.4 ] } ] },\n"
  "   \"y_target\": [ 0, 0.16, 1000, 0.165, 10000, 0.17 ] } }\n"
  "] }\n";

int main(void) {
  QT_BEGIN("libcamera");
  alarm(60);
  const char *cfg = "/tmp/qnxport-libcamera-tuning.json";
  FILE *f = fopen(cfg, "w");
  if (f) { fputs(TUNING, f); fclose(f); }
  QT_CHECK(f != NULL, "tuning file written");

  QT_CHECK(rpiIpaStart(RPI4, "/tmp/qnxport-no-such-tuning.json", "imx708", 1920, 1080) == NULL, "missing tuning file rejected");
  QT_CHECK(rpiIpaStart(RPI4, cfg, "no-such-sensor", 1920, 1080) == NULL, "unknown sensor rejected");
  struct RpiIpaHandle_t *h = rpiIpaStart(RPI4, cfg, "imx708", 1920, 1080);
  QT_CHECK(h != NULL, "IPA started: Pi 4, imx708, 1920x1080");
  if (!h) { printf("  info (rpiIpaStart also fails when slog2 cannot register: is slogger2 running?)\n"); unlink(cfg); QT_END(); }

  struct bcm2835_isp_stats *st = calloc(1, sizeof *st);
  st->version = 1; st->size = sizeof *st;
  /* 13-bit pipeline: per-region means G = 2000, R = 1000, B = 1333 */
  for (int i = 0; i < 16 * 12; ++i) {
    st->awb_stats[i].counted = 20000;
    st->awb_stats[i].g_sum = 2000ULL * 20000; st->awb_stats[i].r_sum = 1000ULL * 20000; st->awb_stats[i].b_sum = 1333ULL * 20000;
  }
  for (int i = 0; i < 16; ++i) {
    st->agc_stats[i].counted = 20000;
    st->agc_stats[i].g_sum = 2000ULL * 20000; st->agc_stats[i].r_sum = 1000ULL * 20000; st->agc_stats[i].b_sum = 1333ULL * 20000;
  }
  for (int b = 0; b < NUM_HISTOGRAM_BINS; ++b) st->hist[0].g[b] = (b >= 30 && b < 50) ? 10000 : 10;
  uint8_t sensor_md[1];
  int32_t exp = 0, iso = 0, gr = 0, gg = 0, gb = 0;
  int rc = -1, frames = 0;
  /* The AWB runs on its own thread; give it up to 200 frames to report. */
  for (; frames < 200; ++frames) {
    rc = rpiIpaProcessData(h, sensor_md, 0, st, sizeof *st, &exp, &iso, &gr, &gg, &gb);
    if (rc != 0) break;
    if (frames > 5 && gr > 1800 && gr < 2200 && gb > 1300 && gb < 1700) break;
    usleep(5000);
  }
  printf("  info after %d frame(s): rc %d, exposure %d lines, gain code %d, gains R %d G %d B %d (x1000)\n",
         frames + 1, rc, exp, iso, gr, gg, gb);
  QT_CHECK(rc == 0, "rpiIpaProcessData succeeds");
  QT_CHECK(gr > 1800 && gr < 2200, "grey-world AWB: red gain ~2.0");
  QT_CHECK(gb > 1300 && gb < 1700, "grey-world AWB: blue gain ~1.5");
  QT_CHECK(gg >= 900 && gg <= 1100, "green gain ~1.0");
  /* outputs are sensor units: exposure in lines, analogue gain as the imx708
     gain code (112..960) */
  QT_CHECK(exp > 0 && iso >= 112 && iso <= 960, "AGC: exposure lines > 0, gain code within imx708's 112..960");
  QT_CHECK(rpiIpaProcessData(h, sensor_md, 0, NULL, 0, &exp, &iso, &gr, &gg, &gb) == EINVAL, "NULL statistics rejected");
  rpiIpaStop(h);
  free(st);
  unlink(cfg);
  QT_END();
}
