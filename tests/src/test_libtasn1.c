/* libtasn1: DER tag/length encoding and decoding of an OCTET STRING.
 *
 * asn1_encode_simple_der() emits only the tag and length octets -- the caller
 * concatenates the value itself -- while asn1_decode_simple_der() consumes a
 * complete TLV. */
#include "common.h"
#include <libtasn1.h>
int main(void) {
  QT_BEGIN("libtasn1");
  printf("  info libtasn1 %s\n", asn1_check_version(NULL));
  static const unsigned char val[2] = { 0x12, 0x34 };
  unsigned char tl[16];
  unsigned int tl_len = (unsigned int)sizeof tl;
  QT_CHECK(asn1_encode_simple_der(ASN1_ETYPE_OCTET_STRING, val, sizeof val, tl, &tl_len)
           == ASN1_SUCCESS, "asn1_encode_simple_der");
  QT_CHECK(tl_len == 2 && tl[0] == 0x04 && tl[1] == 0x02,
           "tag/length octets are 04 02");
  unsigned char der[16];
  memcpy(der, tl, tl_len);
  memcpy(der + tl_len, val, sizeof val);
  unsigned int der_len = tl_len + (unsigned int)sizeof val;
  QT_CHECK(der_len == 4 && der[2] == 0x12 && der[3] == 0x34, "full TLV is 04 02 12 34");

  const unsigned char *out = NULL;
  unsigned int olen = 0;
  QT_CHECK(asn1_decode_simple_der(ASN1_ETYPE_OCTET_STRING, der, der_len, &out, &olen)
           == ASN1_SUCCESS, "asn1_decode_simple_der");
  QT_CHECK(olen == 2 && out && out[0] == 0x12 && out[1] == 0x34, "decoded value");

  /* The low-level length codec used throughout the library. */
  /* asn1_get_length_der() refuses a length that would run past the buffer it
   * is given, so the buffer has to be big enough to hold the value too. */
  unsigned char lenbuf[3 + 300];
  int len_len = 0;
  asn1_length_der(300, lenbuf, &len_len);
  QT_CHECK(len_len == 3 && lenbuf[0] == 0x82, "long-form length for 300");
  int read = 0;
  QT_CHECK(asn1_get_length_der(lenbuf, (int)sizeof lenbuf, &read) == 300 &&
           read == len_len, "asn1_get_length_der round trip");

  QT_CHECK(asn1_strerror(ASN1_SUCCESS) != NULL, "asn1_strerror");
  QT_END();
}
