/* OpenJPEG: encode an RGB image to a JPEG 2000 codestream and decode it back. */
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <openjpeg-2.5/openjpeg.h>
#define W 64
#define H 48
static void quiet(const char *msg, void *c) { (void)msg; (void)c; }
int main(void) {
  QT_BEGIN("openjpeg");
  printf("  info OpenJPEG %s\n", opj_version());
  opj_image_cmptparm_t parms[3];
  memset(parms, 0, sizeof parms);
  for (int i = 0; i < 3; ++i) {
    parms[i].dx = 1; parms[i].dy = 1; parms[i].w = W; parms[i].h = H;
    parms[i].prec = 8; parms[i].bpp = 8; parms[i].sgnd = 0;
  }
  opj_image_t *img = opj_image_create(3, parms, OPJ_CLRSPC_SRGB);
  QT_CHECK(img != NULL, "opj_image_create");
  if (!img) QT_END();
  img->x1 = W; img->y1 = H;
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
    int i = y * W + x;
    img->comps[0].data[i] = x * 4; img->comps[1].data[i] = y * 5;
    img->comps[2].data[i] = 0x40;
  }
  char path[] = "/tmp/qnxport-j2k-XXXXXX";
  int fd = mkstemp(path);
  QT_CHECK(fd >= 0, "create temp file");
  if (fd >= 0) close(fd);

  opj_cparameters_t cp;
  opj_set_default_encoder_parameters(&cp);
  cp.tcp_numlayers = 1; cp.cp_disto_alloc = 1; cp.tcp_rates[0] = 0;  /* lossless */
  opj_codec_t *enc = opj_create_compress(OPJ_CODEC_J2K);
  opj_set_error_handler(enc, quiet, NULL);
  opj_set_warning_handler(enc, quiet, NULL);
  QT_CHECK(opj_setup_encoder(enc, &cp, img), "opj_setup_encoder");
  opj_stream_t *os = opj_stream_create_default_file_stream(path, OPJ_FALSE);
  QT_CHECK(os != NULL, "create output stream");
  QT_CHECK(opj_start_compress(enc, img, os) && opj_encode(enc, os) &&
           opj_end_compress(enc, os), "encode");
  opj_stream_destroy(os);
  opj_destroy_codec(enc);
  opj_image_destroy(img);

  opj_stream_t *is = opj_stream_create_default_file_stream(path, OPJ_TRUE);
  QT_CHECK(is != NULL, "create input stream");
  opj_codec_t *dec = opj_create_decompress(OPJ_CODEC_J2K);
  opj_set_error_handler(dec, quiet, NULL);
  opj_set_warning_handler(dec, quiet, NULL);
  opj_dparameters_t dp;
  opj_set_default_decoder_parameters(&dp);
  QT_CHECK(opj_setup_decoder(dec, &dp), "opj_setup_decoder");
  opj_image_t *back = NULL;
  QT_CHECK(opj_read_header(is, dec, &back), "opj_read_header");
  QT_CHECK(back && back->numcomps == 3, "three components");
  QT_CHECK(back && back->comps[0].w == W && back->comps[0].h == H,
           "decoded dimensions match");
  QT_CHECK(opj_decode(dec, is, back) && opj_end_decompress(dec, is), "decode");
  if (back) {
    QT_CHECK(back->comps[0].data[W + 1] == 4 && back->comps[2].data[0] == 0x40,
             "pixels survive the lossless round trip");
    opj_image_destroy(back);
  }
  opj_stream_destroy(is);
  opj_destroy_codec(dec);
  unlink(path);
  QT_END();
}
