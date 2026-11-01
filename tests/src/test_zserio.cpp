// zserio C++ runtime: the bit stream API that generated code is built on,
// with known answers. (The zserio code generator is a Java program; the
// build machine has no JRE, so no schema is compiled here and the runtime is
// exercised directly.) Checks: bit fields packed MSB first across byte
// boundaries (exact bytes), signed fields, the variable-length integers with
// their zserio wire format and bit sizes, varsize-prefixed strings and bytes,
// float16/32/64 including IEEE half-precision bits, bools, alignment, bit
// buffers written and read back (also through a file), the bit position API,
// and reading past the end throwing CppRuntimeException.
#include "common.h"
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <unistd.h>
#include <zserio/BitBuffer.h>
#include <zserio/BitSizeOfCalculator.h>
#include <zserio/BitStreamReader.h>
#include <zserio/BitStreamWriter.h>
#include <zserio/CppRuntimeException.h>
#include <zserio/CppRuntimeVersion.h>
#include <zserio/FileUtil.h>
#include <zserio/FloatUtil.h>
#include <zserio/HashCodeUtil.h>

static bool bytes_are(const uint8_t *b, std::vector<uint8_t> want) {
  for (size_t i = 0; i < want.size(); i++)
    if (b[i] != want[i]) { printf("  info byte %zu: %02x, wanted %02x\n", i, b[i], want[i]); return false; }
  return true;
}

int main() {
  QT_BEGIN("zserio");
  printf("  info C++ runtime %s\n", CPP_EXTENSION_RUNTIME_VERSION_STRING);
  try {
    // bit fields: 3 + 5 + 4 + 12 bits = 3 bytes, MSB first
    {
      uint8_t buf[8] = { 0 };
      zserio::BitStreamWriter w(buf, sizeof buf);
      w.writeBits(0x5, 3);      // 101
      w.writeBits(0x1F, 5);     // 11111      -> 1011 1111 = 0xBF
      w.writeBits(0xA, 4);      // 1010
      w.writeBits(0x123, 12);   // 0001 0010 0011 -> 1010 0001 | 0010 0011 = 0xA1 0x23
      QT_CHECK(w.getBitPosition() == 24, "24 bits written");
      QT_CHECK(bytes_are(buf, { 0xBF, 0xA1, 0x23 }), "bit fields packed MSB first: BF A1 23");
      zserio::BitStreamReader r(buf, sizeof buf);
      QT_CHECK(r.readBits(3) == 0x5 && r.readBits(5) == 0x1F && r.readBits(4) == 0xA && r.readBits(12) == 0x123, "bit fields read back");
    }
    // signed fields and 64-bit values
    {
      uint8_t buf[32] = { 0 };
      zserio::BitStreamWriter w(buf, sizeof buf);
      w.writeSignedBits(-1, 4); w.writeSignedBits(-8, 4); w.writeSignedBits(7, 4);
      w.writeBits64(0x0123456789ABCDEFULL, 64); w.writeSignedBits64(-1234567890123LL, 48);
      zserio::BitStreamReader r(buf, sizeof buf);
      QT_CHECK(r.readSignedBits(4) == -1 && r.readSignedBits(4) == -8 && r.readSignedBits(4) == 7, "signed 4-bit fields: -1, -8, 7");
      QT_CHECK(r.readBits64(64) == 0x0123456789ABCDEFULL && r.readSignedBits64(48) == -1234567890123LL, "64-bit and signed 48-bit fields");
    }
    // variable-length integers: wire format and sizes
    {
      uint8_t buf[64] = { 0 };
      zserio::BitStreamWriter w(buf, sizeof buf);
      w.writeVarUInt16(1);        // 01
      w.writeVarUInt16(300);      // 1|0000001 00101100 -> 81 2C
      w.writeVarInt16(-1);        // sign, has-next, 6 bits: 1 0 000001 -> 81
      w.writeVarSize(127);        // 7F
      QT_CHECK(bytes_are(buf, { 0x01, 0x81, 0x2C, 0x81, 0x7F }), "varuint16 1, 300; varint16 -1; varsize 127: 01 81 2C 81 7F");
      QT_CHECK(zserio::bitSizeOfVarUInt16(300) == 16 && zserio::bitSizeOfVarUInt16(1) == 8 && zserio::bitSizeOfVarInt16(-1) == 8,
               "bitSizeOfVarUInt16/VarInt16");
      // varuint64 has at most 8 bytes, the last one with all 8 bits (a 57-bit range)
      QT_CHECK(zserio::bitSizeOfVarUInt64(UINT64_C(1) << 56) == 64 && zserio::bitSizeOfVarUInt64(127) == 8 &&
               zserio::bitSizeOfVarSize(1u << 20) == 24, "bitSizeOf for varuint64 (8 bytes at 2^56) and varsize");
      w.writeVarUInt64(UINT64_C(0xFFFFFFFFFFFFFFFF) >> 8); w.writeVarInt(INT64_MIN); w.writeVarUInt(0); w.writeVarInt64(-123456789);
      w.writeVarUInt32(268435455); w.writeVarInt32(-268435455);
      zserio::BitStreamReader r(buf, sizeof buf);
      QT_CHECK(r.readVarUInt16() == 1 && r.readVarUInt16() == 300 && r.readVarInt16() == -1 && r.readVarSize() == 127, "varints read back");
      QT_CHECK(r.readVarUInt64() == (UINT64_C(0xFFFFFFFFFFFFFFFF) >> 8) && r.readVarInt() == INT64_MIN && r.readVarUInt() == 0 &&
               r.readVarInt64() == -123456789 && r.readVarUInt32() == 268435455 && r.readVarInt32() == -268435455,
               "varuint64 (56 bits), varint INT64_MIN, varint64/varuint32/varint32 extremes");
      bool threw = false;
      try { w.writeVarUInt16(1u << 15); } catch (const zserio::CppRuntimeException &e) { threw = true; printf("  info out of range: %s\n", e.what()); }
      QT_CHECK(threw, "varuint16 out of range throws CppRuntimeException");
    }
    // strings, bytes, floats, bools, alignment
    {
      uint8_t buf[128] = { 0 };
      zserio::BitStreamWriter w(buf, sizeof buf);
      w.writeString("QNX");
      QT_CHECK(bytes_are(buf, { 0x03, 'Q', 'N', 'X' }), "string: varsize length then UTF-8 bytes");
      w.writeFloat16(1.0f);
      QT_CHECK(bytes_are(buf + 4, { 0x3C, 0x00 }), "float16 1.0 is 3C00");
      QT_CHECK(zserio::convertFloatToUInt16(-2.0f) == 0xC000 && zserio::convertUInt16ToFloat(0x7BFF) == 65504.0f, "half-precision conversions (-2.0, max 65504)");
      w.writeFloat32(3.25f); w.writeFloat64(M_PI);
      w.writeBool(true); w.writeBool(false); w.writeBool(true);
      w.alignTo(8);
      QT_CHECK(w.getBitPosition() % 8 == 0, "alignTo(8)");
      const uint8_t raw[] = { 0xDE, 0xAD, 0xBE, 0xEF };
      w.writeBytes(zserio::Span<const uint8_t>(raw, 4));
      w.writeString(std::string("gr\xc3\xbc\xc3\x9f" "e"));   // UTF-8, 7 bytes
      size_t end = w.getBitPosition();
      zserio::BitStreamReader r(buf, (end + 7) / 8);
      QT_CHECK(r.readString() == "QNX" && r.readFloat16() == 1.0f && r.readFloat32() == 3.25f && r.readFloat64() == M_PI, "string and floats read back");
      QT_CHECK(r.readBool() && !r.readBool() && r.readBool(), "bools read back");
      r.alignTo(8);
      auto b = r.readBytes();
      QT_CHECK(b.size() == 4 && b[0] == 0xDE && b[3] == 0xEF, "bytes read back");
      QT_CHECK(r.readString() == "gr\xc3\xbc\xc3\x9f" "e" && r.getBitPosition() == end, "UTF-8 string read back, position at the end");
      bool threw = false;
      try { r.readBits(8); } catch (const zserio::CppRuntimeException &e) { threw = true; printf("  info past the end: %s\n", e.what()); }
      QT_CHECK(threw, "reading past the end throws CppRuntimeException");
      r.setBitPosition(8);
      QT_CHECK(r.readBits(8) == 'Q', "setBitPosition and read again");
    }
    // bit buffers: odd bit size, embedded in a stream, through a file
    {
      const uint8_t data[] = { 0xAB, 0xC0 };
      zserio::BitBuffer bb(data, 10);       // 1010 1011 11
      QT_CHECK(bb.getBitSize() == 10 && bb.getByteSize() == 2, "BitBuffer of 10 bits");
      zserio::BitBuffer same(std::vector<uint8_t>{ 0xAB, 0xFF }, 10);   // differs only beyond bit 10
      QT_CHECK(bb == same && bb.hashCode() == same.hashCode(), "BitBuffer equality ignores bits past the size");
      zserio::BitBuffer out(256);
      zserio::BitStreamWriter w(out);
      w.writeBits(1, 1); w.writeBitBuffer(bb); w.writeVarUInt(1000000);
      size_t bits = w.getBitPosition();
      zserio::BitStreamReader r(out.getBuffer(), bits, zserio::BitsTag());
      QT_CHECK(r.readBits(1) == 1 && r.readBitBuffer() == bb && r.readVarUInt() == 1000000, "bit buffer embedded at an odd bit offset");
      char path[96]; snprintf(path, sizeof path, "/tmp/qnxport-zserio-%d.bin", (int)getpid());
      zserio::writeBufferToFile(w, path);
      zserio::BitBuffer back = zserio::readBufferFromFile(path);
      unlink(path);
      QT_CHECK(back.getByteSize() == (bits + 7) / 8 && memcmp(back.getBuffer(), out.getBuffer(), back.getByteSize()) == 0,
               "writeBufferToFile / readBufferFromFile");
    }
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "no unexpected exception");
  }
  QT_END();
}
