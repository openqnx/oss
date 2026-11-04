/* libgme: a VGM file built here -- the Sega Master System's SN76489 sound chip
 * playing a 440 Hz square wave (tone period 254 at 3.579545 MHz) at full
 * volume for one second, then silence for half a second -- is identified,
 * opened and played into 16-bit stereo PCM at 44.1 kHz: the track length
 * comes from the header (1.5 s), the first second is loud with ~440 cycles
 * per second, the rest is silent, and gme_tell follows playback. */
#include "common.h"
#include <gme/gme.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

static void le32(uint8_t *p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }

int main(void) {
  QT_BEGIN("libgme");
  uint8_t vgm[0x40 + 32];
  memset(vgm, 0, sizeof vgm);
  memcpy(vgm, "Vgm ", 4);
  uint8_t *d = vgm + 0x40; int n = 0;
  d[n++] = 0x50; d[n++] = 0x8E;   /* latch ch0 tone, low 4 bits of 254 */
  d[n++] = 0x50; d[n++] = 0x0F;   /* high 6 bits */
  d[n++] = 0x50; d[n++] = 0x90;   /* ch0 attenuation 0: full volume */
  d[n++] = 0x61; d[n++] = 0x44; d[n++] = 0xAC;   /* wait 44100 samples */
  d[n++] = 0x50; d[n++] = 0x9F;   /* ch0 off */
  d[n++] = 0x61; d[n++] = 0x22; d[n++] = 0x56;   /* wait 22050 */
  d[n++] = 0x66;                  /* end */
  int size = 0x40 + n;
  le32(vgm + 0x04, size - 4);
  le32(vgm + 0x08, 0x150);        /* version 1.50 */
  le32(vgm + 0x0C, 3579545);      /* SN76489 clock */
  le32(vgm + 0x18, 66150);        /* total samples */
  vgm[0x28] = 0x09; vgm[0x2A] = 16;   /* SMS noise feedback, shift width */
  le32(vgm + 0x34, 0x40 - 0x34);  /* data offset */

  QT_CHECK(!strcmp(gme_identify_header(vgm), "VGM"), "header identified as VGM");
  Music_Emu *emu = NULL;
  gme_err_t err = gme_open_data(vgm, size, &emu, 44100);
  if (err) printf("  info %s\n", err);
  QT_CHECK(!err && emu, "gme_open_data opens it at 44.1 kHz");
  if (!emu) QT_END();
  QT_CHECK(gme_track_count(emu) == 1, "one track");
  gme_info_t *info = NULL;
  gme_track_info(emu, &info, 0);
  printf("  info length %d ms, system '%s'\n", info ? info->length : -1, info ? info->system : "");
  QT_CHECK(info && info->length == 1500, "track length 1500 ms from the header");
  gme_free_info(info);
  QT_CHECK(gme_start_track(emu, 0) == NULL, "start track 0");
  gme_ignore_silence(emu, 1);
  enum { N = 44100 * 3 / 2 };
  short *buf = malloc(N * 2 * sizeof(short));
  QT_CHECK(gme_play(emu, N * 2, buf) == NULL, "1.5 s played");
  /* the left channel: loud part 0.1..0.9 s, quiet part 1.1..1.4 s */
  double mean = 0; for (int i = 4410; i < 39690; i++) mean += buf[2 * i]; mean /= (39690 - 4410);
  double rms = 0; int cross = 0;
  for (int i = 4410; i < 39690; i++) {
    double v = buf[2 * i] - mean; rms += v * v;
    if ((buf[2 * i] - mean) * (buf[2 * i - 2] - mean) < 0) cross++;
  }
  rms = sqrt(rms / (39690 - 4410));
  double hz = cross / 2.0 / 0.8;
  double qrms = 0; for (int i = 48510; i < 61740; i++) qrms += (double)buf[2 * i] * buf[2 * i];
  qrms = sqrt(qrms / (61740 - 48510));
  printf("  info tone rms %.0f, %.1f Hz; after it rms %.0f\n", rms, hz, qrms);
  QT_CHECK(rms > 1000, "the first second is loud");
  QT_CHECK(hz > 420 && hz < 460, "at 440 Hz (420-460)");
  QT_CHECK(qrms < rms / 20, "silent after the channel is switched off");
  QT_CHECK(gme_tell(emu) >= 1490 && gme_tell(emu) <= 1510, "gme_tell is 1500 ms");
  free(buf);
  gme_delete(emu);
  QT_END();
}
