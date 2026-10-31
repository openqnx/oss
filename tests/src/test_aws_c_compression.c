/* aws-c-compression: the generic Huffman encoder/decoder (HPACK's engine),
 * driven by a small prefix code defined here -- a known bit pattern for
 * "abracadabra", EOS padding, literal escapes, streaming decode in chunks. */
#include "common.h"
#include <aws/common/common.h>
#include <aws/compression/compression.h>
#include <aws/compression/huffman.h>

/* a=0 b=10 r=110 c=1110 d=11110, anything else: 11111 + 8-bit literal. */
static struct aws_huffman_code enc(uint8_t s, void *ud) {
  (void)ud;
  struct aws_huffman_code c;
  switch (s) {
    case 'a': c.pattern = 0x0; c.num_bits = 1; break;
    case 'b': c.pattern = 0x2; c.num_bits = 2; break;
    case 'r': c.pattern = 0x6; c.num_bits = 3; break;
    case 'c': c.pattern = 0xE; c.num_bits = 4; break;
    case 'd': c.pattern = 0x1E; c.num_bits = 5; break;
    default: c.pattern = (0x1Fu << 8) | s; c.num_bits = 13; break;
  }
  return c;
}
static uint8_t dec(uint32_t bits, uint8_t *sym, void *ud) {
  (void)ud;
  if (!(bits & 0x80000000u)) { *sym = 'a'; return 1; }
  if ((bits >> 30) == 0x2) { *sym = 'b'; return 2; }
  if ((bits >> 29) == 0x6) { *sym = 'r'; return 3; }
  if ((bits >> 28) == 0xE) { *sym = 'c'; return 4; }
  if ((bits >> 27) == 0x1E) { *sym = 'd'; return 5; }
  *sym = (uint8_t)(bits >> 19); return 13;
}

int main(void) {
  QT_BEGIN("aws-c-compression");
  struct aws_allocator *al = aws_default_allocator();
  aws_compression_library_init(al);
  struct aws_huffman_symbol_coder coder = {enc, dec, NULL};
  struct aws_huffman_encoder e; aws_huffman_encoder_init(&e, &coder);
  struct aws_huffman_decoder d; aws_huffman_decoder_init(&d, &coder);

  struct aws_byte_cursor in = aws_byte_cursor_from_c_str("abracadabra");
  QT_CHECK(aws_huffman_get_encoded_length(&e, in) == 3, "encoded length of \"abracadabra\" is 3 bytes (24 bits)");
  struct aws_byte_buf out; aws_byte_buf_init(&out, al, 64);
  QT_CHECK(aws_huffman_encode(&e, &in, &out) == AWS_OP_SUCCESS, "encode");
  QT_CHECK(out.len == 3 && out.buffer[0] == 0x59 && out.buffer[1] == 0xCF && out.buffer[2] == 0x2C,
           "bit pattern 59 cf 2c");
  struct aws_byte_cursor ec = aws_byte_cursor_from_buf(&out);
  struct aws_byte_buf back; aws_byte_buf_init(&back, al, 64);
  QT_CHECK(aws_huffman_decode(&d, &ec, &back) == AWS_OP_SUCCESS && aws_byte_buf_eq_c_str(&back, "abracadabra"),
           "decode round trip");

  /* Literals and EOS padding: 13-bit escapes leave a partial last byte. */
  const char msg[] = "QNX\x00\xff abc d";
  struct aws_byte_cursor in2 = aws_byte_cursor_from_array(msg, sizeof msg - 1);
  aws_huffman_encoder_reset(&e); aws_byte_buf_reset(&out, true);
  size_t want = aws_huffman_get_encoded_length(&e, in2);
  QT_CHECK(aws_huffman_encode(&e, &in2, &out) == AWS_OP_SUCCESS && out.len == want, "encode with literals");
  QT_CHECK((out.buffer[out.len - 1] & 1) == 1, "last byte padded with EOS (1) bits");
  /* Stream the encoded bytes to the decoder one at a time. */
  aws_huffman_decoder_reset(&d); aws_byte_buf_reset(&back, true);
  int ok = 1;
  for (size_t i = 0; i < out.len; ++i) {
    struct aws_byte_cursor one = aws_byte_cursor_from_array(out.buffer + i, 1);
    if (aws_huffman_decode(&d, &one, &back) != AWS_OP_SUCCESS) ok = 0;
  }
  QT_CHECK(ok && back.len == sizeof msg - 1 && memcmp(back.buffer, msg, back.len) == 0,
           "byte-at-a-time streaming decode restores binary data");

  /* A short output buffer without growth is an error, with growth it works. */
  aws_huffman_decoder_reset(&d);
  struct aws_byte_buf small; aws_byte_buf_init(&small, al, 2);
  ec = aws_byte_cursor_from_array("\x59\xcf\x2c", 3);
  QT_CHECK(aws_huffman_decode(&d, &ec, &small) == AWS_OP_ERR && aws_last_error() == AWS_ERROR_SHORT_BUFFER,
           "short buffer reported");
  aws_huffman_decoder_reset(&d); aws_byte_buf_reset(&small, true);
  aws_huffman_decoder_allow_growth(&d, true);
  ec = aws_byte_cursor_from_array("\x59\xcf\x2c", 3);
  QT_CHECK(aws_huffman_decode(&d, &ec, &small) == AWS_OP_SUCCESS && aws_byte_buf_eq_c_str(&small, "abracadabra"),
           "decoder grows its output when allowed");
  aws_byte_buf_clean_up(&small); aws_byte_buf_clean_up(&out); aws_byte_buf_clean_up(&back);
  aws_compression_library_clean_up();
  QT_END();
}
