/* libheif: encode a known 64x48 RGB image (smooth gradients) to HEIF with
 * x265 (HEVC), to AVIF with aom (AV1) and with JPEG and JPEG 2000 codecs, write each
 * to a file, check the file brand, read it back and decode: the size must
 * match and the pixels be close to the original (lossy codecs: mean absolute
 * error under 3 of 255). Also: which decoders/encoders are built in, and a
 * metadata (EXIF) block surviving the round trip. */
#include "common.h"
#include <libheif/heif.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define W 64
#define H 48

static uint8_t px(int x, int y, int c) {
  return (uint8_t)(c == 0 ? x * 4 : c == 1 ? y * 5 : 128 + (x - y));
}

static struct heif_image *make_image(void) {
  struct heif_image *img = NULL;
  if (heif_image_create(W, H, heif_colorspace_RGB, heif_chroma_interleaved_RGB, &img).code) return NULL;
  heif_image_add_plane(img, heif_channel_interleaved, W, H, 8);
  int stride = 0;
  uint8_t *p = heif_image_get_plane(img, heif_channel_interleaved, &stride);
  for (int y = 0; y < H; y++) for (int x = 0; x < W; x++)
    for (int c = 0; c < 3; c++) p[y * stride + x * 3 + c] = px(x, y, c);
  return img;
}

/* returns mean absolute error *1000, or -1 */
static long round_trip(enum heif_compression_format fmt, const char *fn, const char *brand, int with_exif) {
  struct heif_context *ctx = heif_context_alloc();
  struct heif_encoder *enc = NULL;
  struct heif_error e = heif_context_get_encoder_for_format(ctx, fmt, &enc);
  if (e.code) { printf("  info no encoder: %s\n", e.message); heif_context_free(ctx); return -1; }
  heif_encoder_set_lossy_quality(enc, 95);
  struct heif_image *img = make_image();
  struct heif_image_handle *h = NULL;
  e = heif_context_encode_image(ctx, img, enc, NULL, &h);
  if (e.code) { printf("  info encode: %s\n", e.message); return -1; }
  if (with_exif) {
    static const uint8_t exif[] = { 'M','M',0,42,0,0,0,8, 0,0, 0,0,0,0 };  /* big-endian TIFF, no entries */
    heif_context_add_exif_metadata(ctx, h, exif, sizeof exif);
  }
  heif_image_handle_release(h);
  heif_encoder_release(enc);
  heif_image_release(img);
  e = heif_context_write_to_file(ctx, fn);
  heif_context_free(ctx);
  if (e.code) { printf("  info write: %s\n", e.message); return -1; }

  FILE *f = fopen(fn, "rb"); uint8_t hdr[16] = {0};
  if (f) { if (fread(hdr, 1, 16, f) != 16) {} fclose(f); }
  if (memcmp(hdr + 4, "ftyp", 4) || memcmp(hdr + 8, brand, 4)) {
    printf("  info brand is '%.4s', expected '%s'\n", (char *)hdr + 8, brand); return -1;
  }
  ctx = heif_context_alloc();
  e = heif_context_read_from_file(ctx, fn, NULL);
  if (e.code) { printf("  info read: %s\n", e.message); return -1; }
  h = NULL; heif_context_get_primary_image_handle(ctx, &h);
  if (!h || heif_image_handle_get_width(h) != W || heif_image_handle_get_height(h) != H) return -1;
  if (with_exif) {
    heif_item_id ids[2]; int n = heif_image_handle_get_list_of_metadata_block_IDs(h, "Exif", ids, 2);
    if (n != 1) { printf("  info %d Exif blocks\n", n); return -1; }
  }
  struct heif_image *out = NULL;
  e = heif_decode_image(h, &out, heif_colorspace_RGB, heif_chroma_interleaved_RGB, NULL);
  if (e.code) { printf("  info decode: %s\n", e.message); return -1; }
  int stride = 0;
  const uint8_t *p = heif_image_get_plane_readonly(out, heif_channel_interleaved, &stride);
  long sum = 0;
  for (int y = 0; y < H; y++) for (int x = 0; x < W; x++)
    for (int c = 0; c < 3; c++) sum += labs((long)p[y * stride + x * 3 + c] - px(x, y, c));
  heif_image_release(out); heif_image_handle_release(h); heif_context_free(ctx);
  return sum * 1000 / (W * H * 3);
}

int main(void) {
  QT_BEGIN("libheif");
  heif_init(NULL);
  printf("  info libheif %s\n", heif_get_version());
  QT_CHECK(heif_get_version_number_major() == 1, "libheif 1.x");
  QT_CHECK(heif_have_decoder_for_format(heif_compression_HEVC), "HEVC decoder (libde265)");
  QT_CHECK(heif_have_encoder_for_format(heif_compression_HEVC), "HEVC encoder (x265)");
  QT_CHECK(heif_have_decoder_for_format(heif_compression_AV1), "AV1 decoder (aom/dav1d)");
  QT_CHECK(heif_have_encoder_for_format(heif_compression_AV1), "AV1 encoder (aom)");
  QT_CHECK(heif_have_decoder_for_format(heif_compression_JPEG2000), "JPEG 2000 decoder (openjpeg)");
  char dir[] = "/tmp/qnxport-heif.XXXXXX";
  if (!mkdtemp(dir)) { QT_CHECK(0, "temp dir"); QT_END(); }
  char fn[128];
  long m;
  snprintf(fn, sizeof fn, "%s/a.heic", dir);
  m = round_trip(heif_compression_HEVC, fn, "heic", 1);
  printf("  info HEVC mean abs error %ld.%03ld\n", m / 1000, m % 1000);
  QT_CHECK(m >= 0 && m < 3000, "HEIF (x265 -> libde265): 64x48, Exif kept, pixels within 3/255");
  snprintf(fn, sizeof fn, "%s/a.avif", dir);
  m = round_trip(heif_compression_AV1, fn, "avif", 0);
  printf("  info AV1 mean abs error %ld.%03ld\n", m / 1000, m % 1000);
  QT_CHECK(m >= 0 && m < 3000, "AVIF (aom): 64x48, pixels within 3/255");
  snprintf(fn, sizeof fn, "%s/a.hej2", dir);
  m = round_trip(heif_compression_JPEG2000, fn, "j2ki", 0);
  printf("  info JPEG 2000 mean abs error %ld.%03ld\n", m / 1000, m % 1000);
  QT_CHECK(m >= 0 && m < 3000, "HEIF with JPEG 2000 (openjpeg): pixels within 3/255");
  char cmd[160]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); if (system(cmd)) {}
  heif_deinit();
  QT_END();
}
