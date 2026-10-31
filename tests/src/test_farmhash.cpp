/* farmhash: the Fingerprint functions are specified to be identical on every
 * platform, so they are checked against values computed on the build host
 * from the same source (Fingerprint64("") is the k2 constant
 * 0x9ae16a3b2f90404f). The platform-tuned Hash functions are checked for
 * consistency: deterministic, seed-sensitive, string-overload equivalent. */
#include "common.h"
#include <string>
#include <farmhash.h>

struct KA { const char *s; uint32_t f32; uint64_t f64, lo128, hi128; };
static const KA ka[] = {
  {"", 0xdc56d17au, 0x9ae16a3b2f90404fULL, 0x3df09dfc64c09a2bULL, 0x3cb540c392e51e29ULL},
  {"a", 0x3c973d4du, 0xb3454265b6df75e3ULL, 0x6e97d6bbdfc0a0c4ULL, 0x52a71e38f43be561ULL},
  {"hello world", 0x19a7581au, 0x588fb7478bd6b01bULL, 0x28690d39700514edULL, 0x61196fad02431508ULL},
  {"The quick brown fox jumps over the lazy dog", 0xec998320u, 0xabbe83f33b1b5134ULL, 0xa7f9a86a2d60c968ULL, 0xbf1498f876dbe279ULL},
};

int main() {
  QT_BEGIN("farmhash");
  for (const KA &k : ka) {
    size_t n = strlen(k.s);
    util::uint128_t f = util::Fingerprint128(k.s, n);
    char what[96];
    snprintf(what, sizeof what, "Fingerprint32/64/128 of \"%.20s\" (len %zu)", k.s, n);
    bool ok = util::Fingerprint32(k.s, n) == k.f32 && util::Fingerprint64(k.s, n) == k.f64 &&
              util::Uint128Low64(f) == k.lo128 && util::Uint128High64(f) == k.hi128;
    if (!ok) printf("  info got %08x %016llx %016llx %016llx\n", util::Fingerprint32(k.s, n),
                    (unsigned long long)util::Fingerprint64(k.s, n), (unsigned long long)util::Uint128Low64(f),
                    (unsigned long long)util::Uint128High64(f));
    QT_CHECK(ok, what);
  }
  std::string big;
  for (int i = 0; i < 1000; ++i) big += char('a' + (i * 7) % 26);
  QT_CHECK(util::Fingerprint32(big) == 0x916e2a4fu && util::Fingerprint64(big) == 0xd06b2f1c71fdc6d0ULL,
           "Fingerprint32/64 of a 1000-byte string (long-input code paths)");
  QT_CHECK(util::Fingerprint(uint64_t(0x0123456789abcdefULL)) == 0x7502ffd76684b79eULL, "Fingerprint(uint64)");

  std::string s = "QNX Neutrino RTOS";
  QT_CHECK(util::Hash64(s) == util::Hash64(s.data(), s.size()) && util::Hash32(s) == util::Hash32(s.data(), s.size()),
           "Hash32/Hash64 deterministic, string and pointer overloads agree");
  QT_CHECK(util::Hash64WithSeed(s, 1) != util::Hash64WithSeed(s, 2) && util::Hash32WithSeed(s, 1) != util::Hash32WithSeed(s, 2),
           "seeded hashes depend on the seed");
  QT_CHECK(util::Hash64(s) != util::Hash64(std::string("QNX Neutrino RTOs")), "one-bit input change changes Hash64");
  int collisions = 0;
  for (int i = 0; i < 2000; ++i)
    for (int j = i + 1; j < i + 5 && j < 2000; ++j)
      if (util::Hash32(std::to_string(i)) == util::Hash32(std::to_string(j))) ++collisions;
  QT_CHECK(collisions == 0, "no Hash32 collisions among neighbouring small integers");
  QT_END();
}
