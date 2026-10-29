/* libcbor: encode a map and decode it again. */
#include "common.h"
#include <stdlib.h>
#include <cbor.h>
int main(void) {
  QT_BEGIN("libcbor");
  cbor_item_t *m = cbor_new_definite_map(1);
  cbor_map_add(m, (struct cbor_pair){ cbor_move(cbor_build_string("sdp")), cbor_move(cbor_build_uint8(8)) });
  unsigned char *buf; size_t sz; size_t len = cbor_serialize_alloc(m, &buf, &sz);
  QT_CHECK(len > 0, "serialises");
  struct cbor_load_result r; cbor_item_t *b = cbor_load(buf, len, &r);
  QT_CHECK(b && r.error.code == CBOR_ERR_NONE, "decodes");
  QT_CHECK(b && cbor_map_size(b) == 1 && cbor_get_uint8(cbor_map_handle(b)[0].value) == 8, "value survives");
  free(buf); cbor_decref(&m); cbor_decref(&b);
  QT_END();
}
