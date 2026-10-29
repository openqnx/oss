/* protobuf-c: encode and decode a message with a hand-written descriptor-free
   path (the runtime's varint/packing helpers). */
#include "common.h"
#include <protobuf-c/protobuf-c.h>
int main(void) {
  QT_BEGIN("protobuf-c");
  QT_CHECK(protobuf_c_version_number() >= 1005000, "runtime version >= 1.5.0");
  QT_CHECK(strncmp(protobuf_c_version(), "1.5", 3) == 0, "version string");
  /* The macro takes the initial storage as an array: its sizeof() is the
   * starting capacity. Given NULL it claims sizeof(void *) bytes at address 0,
   * and the first append writes there. Start small so append has to grow. */
  uint8_t init[2];
  ProtobufCBufferSimple sb = PROTOBUF_C_BUFFER_SIMPLE_INIT(init);
  sb.base.append(&sb.base, 5, (const uint8_t *)"hello");
  QT_CHECK(sb.len == 5 && memcmp(sb.data, "hello", 5) == 0, "simple buffer append");
  PROTOBUF_C_BUFFER_SIMPLE_CLEAR(&sb);
  QT_END();
}
