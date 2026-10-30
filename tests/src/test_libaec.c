#include "common.h"
#include <string.h>
#include <libaec.h>
#include <szlib.h>
/* libaec: a smooth 16-bit signal encoded (with preprocessing) and decoded
 * back exactly, through the native API and the SZIP-compatible one. */
int main(void) {
  QT_BEGIN("libaec");
  static unsigned short in[8192], out[8192]; static unsigned char enc[40000];
  for (int i = 0; i < 8192; i++) in[i] = (unsigned short)(1000 + (i % 512) * 3);
  struct aec_stream s; memset(&s, 0, sizeof s);
  s.bits_per_sample = 16; s.block_size = 16; s.rsi = 128; s.flags = AEC_DATA_PREPROCESS;
  s.next_in = (unsigned char *)in; s.avail_in = sizeof in; s.next_out = enc; s.avail_out = sizeof enc;
  QT_CHECK(aec_buffer_encode(&s) == AEC_OK, "aec_buffer_encode");
  size_t el = s.total_out; printf("  info %zu -> %zu bytes\n", sizeof in, el);
  QT_CHECK(el < sizeof in / 2, "compressed to less than half");
  memset(&s, 0, sizeof s);
  s.bits_per_sample = 16; s.block_size = 16; s.rsi = 128; s.flags = AEC_DATA_PREPROCESS;
  s.next_in = enc; s.avail_in = el; s.next_out = (unsigned char *)out; s.avail_out = sizeof out;
  QT_CHECK(aec_buffer_decode(&s) == AEC_OK && s.total_out == sizeof in && memcmp(in, out, sizeof in) == 0, "decoded exactly");
  SZ_com_t p = { SZ_MSB_OPTION_MASK | SZ_NN_OPTION_MASK, 16, 16, 128 };
  size_t dl = sizeof enc, ol = sizeof out;
  QT_CHECK(SZ_BufftoBuffCompress(enc, &dl, in, sizeof in, &p) == SZ_OK, "SZ_BufftoBuffCompress");
  QT_CHECK(SZ_BufftoBuffDecompress(out, &ol, enc, dl, &p) == SZ_OK && ol == sizeof in && memcmp(in, out, ol) == 0, "SZIP API round trip");
  QT_END();
}
