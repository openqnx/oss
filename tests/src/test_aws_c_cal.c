/* aws-c-cal: the AWS crypto abstraction (OpenSSL underneath) -- hashes and
 * HMAC against published test vectors, AES-256-CBC against NIST SP 800-38A,
 * HKDF-SHA512, and an ECDSA P-256 sign/verify round trip. */
#include "common.h"
#include <aws/cal/cal.h>
#include <aws/cal/hash.h>
#include <aws/cal/hmac.h>
#include <aws/cal/ecc.h>
#include <aws/cal/hkdf.h>
#include <aws/cal/symmetric_cipher.h>
#include <aws/common/encoding.h>

static struct aws_allocator *al;
static int hex_is(const struct aws_byte_buf *b, const char *hex) {
  struct aws_byte_cursor c = aws_byte_cursor_from_buf(b);
  struct aws_byte_buf h; aws_byte_buf_init(&h, al, b->len * 2 + 2);
  int ok = aws_hex_encode(&c, &h) == AWS_OP_SUCCESS && h.len >= strlen(hex) &&
           memcmp(h.buffer, hex, strlen(hex)) == 0;
  if (!ok) printf("  info got %.*s\n", (int)h.len, (const char *)h.buffer);
  aws_byte_buf_clean_up(&h);
  return ok;
}
static struct aws_byte_buf unhex(const char *hex) {
  struct aws_byte_cursor c = aws_byte_cursor_from_c_str(hex);
  struct aws_byte_buf b; aws_byte_buf_init(&b, al, strlen(hex) / 2 + 1);
  aws_hex_decode(&c, &b);
  return b;
}

int main(void) {
  QT_BEGIN("aws-c-cal");
  al = aws_default_allocator();
  aws_cal_library_init(al);
  struct aws_byte_cursor abc = aws_byte_cursor_from_c_str("abc");
  struct aws_byte_buf out; aws_byte_buf_init(&out, al, 128);

  QT_CHECK(aws_sha256_compute(al, &abc, &out, 0) == AWS_OP_SUCCESS &&
           hex_is(&out, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), "SHA-256(\"abc\")");
  aws_byte_buf_reset(&out, true);
  QT_CHECK(aws_sha1_compute(al, &abc, &out, 0) == AWS_OP_SUCCESS &&
           hex_is(&out, "a9993e364706816aba3e25717850c26c9cd0d89d"), "SHA-1(\"abc\")");
  aws_byte_buf_reset(&out, true);
  QT_CHECK(aws_md5_compute(al, &abc, &out, 0) == AWS_OP_SUCCESS &&
           hex_is(&out, "900150983cd24fb0d6963f7d28e17f72"), "MD5(\"abc\")");
  aws_byte_buf_reset(&out, true);
  /* Streaming: the same digest when fed in pieces. */
  struct aws_hash *h = aws_sha256_new(al);
  struct aws_byte_cursor a = aws_byte_cursor_from_c_str("a"), bc = aws_byte_cursor_from_c_str("bc");
  aws_hash_update(h, &a); aws_hash_update(h, &bc);
  QT_CHECK(aws_hash_finalize(h, &out, 0) == AWS_OP_SUCCESS &&
           hex_is(&out, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), "streaming SHA-256");
  aws_hash_destroy(h);
  aws_byte_buf_reset(&out, true);

  /* RFC 4231 test case 2 */
  struct aws_byte_cursor key = aws_byte_cursor_from_c_str("Jefe");
  struct aws_byte_cursor msg = aws_byte_cursor_from_c_str("what do ya want for nothing?");
  QT_CHECK(aws_sha256_hmac_compute(al, &key, &msg, &out, 0) == AWS_OP_SUCCESS &&
           hex_is(&out, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"), "HMAC-SHA256 (RFC 4231 #2)");
  aws_byte_buf_reset(&out, true);

  /* NIST SP 800-38A F.2.5, first block (aws-c-cal adds PKCS#7 padding after it). */
  struct aws_byte_buf k = unhex("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4");
  struct aws_byte_buf iv = unhex("000102030405060708090a0b0c0d0e0f");
  struct aws_byte_buf pt = unhex("6bc1bee22e409f96e93d7e117393172a");
  struct aws_byte_cursor kc = aws_byte_cursor_from_buf(&k), ivc = aws_byte_cursor_from_buf(&iv);
  struct aws_symmetric_cipher *c = aws_aes_cbc_256_new(al, &kc, &ivc);
  QT_CHECK(c != NULL, "AES-256-CBC cipher created");
  if (c) {
    struct aws_byte_buf ct; aws_byte_buf_init(&ct, al, 64);
    aws_symmetric_cipher_encrypt(c, aws_byte_cursor_from_buf(&pt), &ct);
    aws_symmetric_cipher_finalize_encryption(c, &ct);
    QT_CHECK(ct.len == 32 && hex_is(&ct, "f58c4c04d6e5f1ba779eabfb5f7bfbd6"), "AES-256-CBC NIST vector");
    aws_symmetric_cipher_destroy(c);
    c = aws_aes_cbc_256_new(al, &kc, &ivc);
    struct aws_byte_buf back; aws_byte_buf_init(&back, al, 64);
    aws_symmetric_cipher_decrypt(c, aws_byte_cursor_from_buf(&ct), &back);
    aws_symmetric_cipher_finalize_decryption(c, &back);
    QT_CHECK(aws_byte_buf_eq(&back, &pt), "AES-256-CBC decrypts back to the plaintext");
    aws_byte_buf_clean_up(&back); aws_byte_buf_clean_up(&ct);
    aws_symmetric_cipher_destroy(c);
  }

  /* RFC 5869 test case 1 inputs; aws-c-cal only offers SHA-512 */
  struct aws_byte_buf ikm = unhex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");
  struct aws_byte_buf salt = unhex("000102030405060708090a0b0c");
  struct aws_byte_buf info = unhex("f0f1f2f3f4f5f6f7f8f9");
  struct aws_byte_buf okm; aws_byte_buf_init(&okm, al, 64);
  QT_CHECK(aws_hkdf_derive(al, HKDF_HMAC_SHA512, aws_byte_cursor_from_buf(&ikm),
                           aws_byte_cursor_from_buf(&salt), aws_byte_cursor_from_buf(&info), &okm, 42) == AWS_OP_SUCCESS &&
           okm.len == 42 && hex_is(&okm, "832390086cda71fb47625bb5ceb168e4c8e26a1a16ed34d9fc7fe92c1481579338da362cb8d9f925d7cb"),
           "HKDF-SHA512 (RFC 5869 case 1 inputs, Python-computed answer)");
  aws_byte_buf_clean_up(&okm);

  struct aws_ecc_key_pair *kp = aws_ecc_key_pair_new_generate_random(al, AWS_CAL_ECDSA_P256);
  QT_CHECK(kp != NULL, "generate an ECDSA P-256 key pair");
  if (kp) {
    struct aws_byte_buf digest; aws_byte_buf_init(&digest, al, 32);
    struct aws_byte_cursor m = aws_byte_cursor_from_c_str("sign me on QNX");
    aws_sha256_compute(al, &m, &digest, 0);
    struct aws_byte_buf sig; aws_byte_buf_init(&sig, al, aws_ecc_key_pair_signature_length(kp));
    struct aws_byte_cursor dc = aws_byte_cursor_from_buf(&digest);
    QT_CHECK(aws_ecc_key_pair_sign_message(kp, &dc, &sig) == AWS_OP_SUCCESS && sig.len > 8, "ECDSA sign");
    struct aws_byte_cursor sc = aws_byte_cursor_from_buf(&sig);
    QT_CHECK(aws_ecc_key_pair_verify_signature(kp, &dc, &sc) == AWS_OP_SUCCESS, "ECDSA signature verifies");
    digest.buffer[0] ^= 1;
    QT_CHECK(aws_ecc_key_pair_verify_signature(kp, &dc, &sc) != AWS_OP_SUCCESS, "tampered digest is rejected");
    struct aws_byte_cursor px, py;
    aws_ecc_key_pair_get_public_key(kp, &px, &py);
    QT_CHECK(px.len == 32 && py.len == 32, "P-256 public key coordinates are 32 bytes");
    aws_byte_buf_clean_up(&sig); aws_byte_buf_clean_up(&digest);
    aws_ecc_key_pair_release(kp);
  }
  aws_byte_buf_clean_up(&out); aws_byte_buf_clean_up(&k); aws_byte_buf_clean_up(&iv); aws_byte_buf_clean_up(&pt);
  aws_byte_buf_clean_up(&ikm); aws_byte_buf_clean_up(&salt); aws_byte_buf_clean_up(&info);
  aws_cal_library_clean_up();
  QT_END();
}
