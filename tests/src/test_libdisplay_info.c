/* libdisplay-info: parse a hand-built EDID 1.4 block with known contents
 * (vendor "QNX", product 0x1234, 1920x1080@60 preferred timing at 148.5 MHz,
 * product name and serial strings) through the high-level and EDID APIs,
 * compute a CVT reduced-blanking timing with a known pixel clock, and decode
 * the same block with the di-edid-decode program. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <libdisplay-info/info.h>
#include <libdisplay-info/edid.h>
#include <libdisplay-info/cvt.h>

static uint8_t edid[128];
static void put_string_desc(uint8_t *d, uint8_t tag, const char *s) {
  memset(d, 0, 18);
  d[3] = tag;
  size_t n = strlen(s);
  for (size_t i = 0; i < 13; i++) d[5 + i] = i < n ? (uint8_t)s[i] : (i == n ? 0x0a : 0x20);
}
static void build_edid(void) {
  static const uint8_t head[54] = {
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x45, 0xd8,             /* "QNX": 10001 01110 11000 */
    0x34, 0x12,             /* product 0x1234 */
    0x01, 0x00, 0x00, 0x00, /* serial 1 */
    0x01, 0x22,             /* week 1, 1990+34 = 2024 */
    0x01, 0x04,             /* EDID 1.4 */
    0xa5,                   /* digital, 8 bpc, DisplayPort */
    0x34, 0x1d,             /* 52 x 29 cm */
    0x78,                   /* gamma 2.2 */
    0x06,                   /* sRGB default, preferred timing is native */
    0xee, 0x91, 0xa3, 0x54, 0x4c, 0x99, 0x26, 0x0f, 0x50, 0x54, /* chromaticity */
    0x00, 0x00, 0x00,       /* no established timings */
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, /* standard timings unused */
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  };
  /* 1920x1080@60, 148.5 MHz: the CEA-861 VIC 16 timing. */
  static const uint8_t dtd[18] = { 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58,
                                   0x2c, 0x45, 0x00, 0x0f, 0x28, 0x21, 0x00, 0x00, 0x1e };
  memcpy(edid, head, sizeof head);
  memcpy(edid + 54, dtd, sizeof dtd);
  put_string_desc(edid + 72, 0xfc, "QNX Test");
  put_string_desc(edid + 90, 0xff, "QNX12345");
  memset(edid + 108, 0, 18); edid[108 + 3] = 0x10; /* dummy descriptor */
  edid[126] = 0;                                   /* no extensions */
  uint8_t sum = 0;
  for (int i = 0; i < 127; i++) sum += edid[i];
  edid[127] = (uint8_t)(0x100 - sum);
}
static int streq_free(char *s, const char *want, const char *what) {
  int ok = s && strcmp(s, want) == 0;
  if (!ok) printf("  info %s: got \"%s\", want \"%s\"\n", what, s ? s : "(null)", want);
  free(s);
  return ok;
}
static int run(const char *cmd, char *out, size_t n) {
  out[0] = 0;
  FILE *p = popen(cmd, "r");
  if (!p) return -1;
  size_t len = fread(out, 1, n - 1, p);
  out[len] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main(void) {
  QT_BEGIN("libdisplay-info");
  build_edid();
  struct di_info *info = di_info_parse_edid(edid, sizeof edid);
  QT_CHECK(info != NULL, "di_info_parse_edid accepts the block");
  if (!info) QT_END();
  const char *fail = di_info_get_failure_msg(info);
  if (fail) printf("  info failures: %s\n", fail);
  QT_CHECK(fail == NULL, "no conformity failures");
  /* "QNX" is not in the PNP ID registry: libdisplay-info reports it as such. */
  char *make = di_info_get_make(info);
  printf("  info make \"%s\"\n", make ? make : "(null)");
  QT_CHECK(make && (strcmp(make, "PNP(QNX)") == 0 || strstr(make, "QNX")), "di_info_get_make names QNX");
  free(make);
  QT_CHECK(streq_free(di_info_get_model(info), "QNX Test", "model"), "di_info_get_model = \"QNX Test\"");
  QT_CHECK(streq_free(di_info_get_serial(info), "QNX12345", "serial"), "di_info_get_serial = QNX12345");

  const struct di_edid *e = di_info_get_edid(info);
  QT_CHECK(e && di_edid_get_version(e) == 1 && di_edid_get_revision(e) == 4, "EDID version 1.4");
  const struct di_edid_vendor_product *vp = e ? di_edid_get_vendor_product(e) : NULL;
  QT_CHECK(vp && memcmp(vp->manufacturer, "QNX", 3) == 0 && vp->product == 0x1234 && vp->serial == 1,
           "vendor/product: QNX 0x1234 serial 1");
  QT_CHECK(vp && vp->manufacture_week == 1 && vp->manufacture_year == 2024, "manufactured week 1 of 2024");
  const struct di_edid_screen_size *ss = e ? di_edid_get_screen_size(e) : NULL;
  QT_CHECK(ss && ss->width_cm == 52 && ss->height_cm == 29, "screen size 52 x 29 cm");
  const struct di_edid_detailed_timing_def *const *dt = e ? di_edid_get_detailed_timing_defs(e) : NULL;
  int ndt = 0;
  while (dt && dt[ndt]) ndt++;
  QT_CHECK(ndt == 1, "one detailed timing definition");
  if (ndt >= 1) {
    const struct di_edid_detailed_timing_def *d = dt[0];
    printf("  info DTD %dx%d %d Hz pixel clock, blank %d/%d, image %dx%d mm\n", d->horiz_video,
           d->vert_video, d->pixel_clock_hz, d->horiz_blank, d->vert_blank, d->horiz_image_mm,
           d->vert_image_mm);
    QT_CHECK(d->horiz_video == 1920 && d->vert_video == 1080 && d->pixel_clock_hz == 148500000,
             "DTD: 1920x1080 at 148.5 MHz");
    QT_CHECK(d->horiz_blank == 280 && d->vert_blank == 45 && d->horiz_front_porch == 88 &&
             d->horiz_sync_pulse == 44 && d->vert_front_porch == 4 && d->vert_sync_pulse == 5,
             "DTD: blanking, porches and sync widths");
    QT_CHECK(d->horiz_image_mm == 527 && d->vert_image_mm == 296 && !d->interlaced,
             "DTD: 527 x 296 mm, progressive");
  }
  const struct di_edid_display_descriptor *const *dd = e ? di_edid_get_display_descriptors(e) : NULL;
  int name_found = 0, ndd = 0;
  for (; dd && dd[ndd]; ndd++)
    if (di_edid_display_descriptor_get_tag(dd[ndd]) == DI_EDID_DISPLAY_DESCRIPTOR_PRODUCT_NAME) {
      const char *s = di_edid_display_descriptor_get_string(dd[ndd]);
      name_found = s && strcmp(s, "QNX Test") == 0;
    }
  QT_CHECK(ndd == 3 && name_found, "3 display descriptors, product name \"QNX Test\"");
  di_info_destroy(info);

  /* A corrupted checksum is detected. */
  edid[127] ^= 0x55;
  info = di_info_parse_edid(edid, sizeof edid);
  fail = info ? di_info_get_failure_msg(info) : "rejected";
  QT_CHECK(fail != NULL, "a bad checksum is reported");
  if (info) di_info_destroy(info);
  edid[127] ^= 0x55;

  /* CVT reduced blanking v1, 1920x1080@60: 138.5 MHz, sync 32 px. */
  struct di_cvt_options o;
  memset(&o, 0, sizeof o);
  o.red_blank_ver = DI_CVT_REDUCED_BLANKING_V1;
  o.h_pixels = 1920; o.v_lines = 1080; o.ip_freq_rqd = 60;
  struct di_cvt_timing t;
  memset(&t, 0, sizeof t);
  di_cvt_compute(&t, &o);
  printf("  info CVT-RB 1920x1080@60: %.3f MHz, hfp %.0f hsync %.0f hbp %.0f\n", t.act_pixel_freq,
         t.h_front_porch, t.h_sync, t.h_back_porch);
  QT_CHECK(fabs(t.act_pixel_freq - 138.5) < 0.01 && t.h_sync == 32 && t.h_front_porch == 48 &&
           t.h_back_porch == 80, "di_cvt_compute: CVT-RB 1920x1080@60 = 138.5 MHz, 48/32/80");

  /* The program. */
  char dir[] = "/tmp/qnxport-displayinfo.XXXXXX", path[256], cmd[512];
  static char out[16384];
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  snprintf(path, sizeof path, "%s/edid.bin", dir);
  FILE *f = fopen(path, "wb");
  if (f) { fwrite(edid, 1, sizeof edid, f); fclose(f); }
  snprintf(cmd, sizeof cmd, "di-edid-decode %s 2>&1", path);
  int rc = run(cmd, out, sizeof out);
  int ok = rc == 0 && strstr(out, "Manufacturer: QNX") && strstr(out, "'QNX Test'") &&
           strstr(out, "1920x1080") && strstr(out, "148.500000 MHz") && strstr(out, "EDID conformity: PASS");
  if (!ok) printf("  info di-edid-decode rc=%d output:\n%s\n", rc, out);
  QT_CHECK(ok, "di-edid-decode: manufacturer, product name, DTD 1920x1080 148.5 MHz, conformity PASS");
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  if (system(cmd) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
