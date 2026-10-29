/* msgpack-c: pack a map, unpack it, check the values came back. */
#include "common.h"
#include <msgpack.h>
int main(void) {
  QT_BEGIN("msgpack-c");
  msgpack_sbuffer sbuf;
  msgpack_sbuffer_init(&sbuf);
  msgpack_packer pk;
  msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
  msgpack_pack_map(&pk, 2);
  msgpack_pack_str_with_body(&pk, "sdp", 3);
  msgpack_pack_int(&pk, 8);
  msgpack_pack_str_with_body(&pk, "arch", 4);
  msgpack_pack_str_with_body(&pk, "aarch64le", 9);
  QT_CHECK(sbuf.size > 0, "packed bytes produced");
  msgpack_unpacked res;
  msgpack_unpacked_init(&res);
  msgpack_unpack_return ret = msgpack_unpack_next(&res, sbuf.data, sbuf.size, NULL);
  QT_CHECK(ret == MSGPACK_UNPACK_SUCCESS, "msgpack_unpack_next");
  msgpack_object obj = res.data;
  QT_CHECK(obj.type == MSGPACK_OBJECT_MAP && obj.via.map.size == 2, "map with 2 entries");
  int sdp = -1;
  for (uint32_t i = 0; i < obj.via.map.size; ++i) {
    msgpack_object_kv *kv = &obj.via.map.ptr[i];
    if (kv->key.via.str.size == 3 && memcmp(kv->key.via.str.ptr, "sdp", 3) == 0)
      sdp = (int)kv->val.via.i64;
  }
  QT_CHECK(sdp == 8, "integer value survives");
  msgpack_unpacked_destroy(&res);
  msgpack_sbuffer_destroy(&sbuf);
  QT_END();
}
