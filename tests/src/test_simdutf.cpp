/* simdutf: SIMD UTF validation and transcoding, with a runtime-dispatched
 * implementation chosen for the CPU the test is actually running on. */
#include "common.h"
#include <simdutf.h>
#include <string>
#include <vector>
int main() {
  QT_BEGIN("simdutf");
  printf("  info implementation = %s\n",
         simdutf::get_active_implementation()->name().c_str());
  QT_CHECK(simdutf::get_active_implementation() != nullptr, "an implementation was selected");

  const std::string good = "QNX SDP 8.0 über 中文 \U0001F600";
  QT_CHECK(simdutf::validate_utf8(good.data(), good.size()), "valid UTF-8 accepted");
  const char bad[] = { (char)0xC3, (char)0x28 };     /* truncated 2-byte sequence */
  QT_CHECK(!simdutf::validate_utf8(bad, sizeof bad), "invalid UTF-8 rejected");

  size_t n16 = simdutf::utf16_length_from_utf8(good.data(), good.size());
  std::vector<char16_t> u16(n16);
  size_t w = simdutf::convert_utf8_to_utf16le(good.data(), good.size(), u16.data());
  QT_CHECK(w == n16 && w > 0, "UTF-8 -> UTF-16LE");
  size_t n8 = simdutf::utf8_length_from_utf16le(u16.data(), w);
  std::string back(n8, '\0');
  size_t b = simdutf::convert_utf16le_to_utf8(u16.data(), w, back.data());
  QT_CHECK(b == good.size() && back == good, "UTF-16LE -> UTF-8 round trip");

  size_t n32 = simdutf::utf32_length_from_utf8(good.data(), good.size());
  std::vector<char32_t> u32(n32);
  QT_CHECK(simdutf::convert_utf8_to_utf32(good.data(), good.size(), u32.data()) == n32,
           "UTF-8 -> UTF-32");
  QT_CHECK(u32.back() == 0x1F600, "the astral code point survived");
  QT_CHECK(simdutf::count_utf8(good.data(), good.size()) == n32,
           "code point count agrees with the UTF-32 length");
  QT_END();
}
