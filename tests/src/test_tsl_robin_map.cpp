// tsl-robin-map: robin_map / robin_set behave like the std unordered
// containers on known data -- insert, lookup, overwrite, erase with backward
// shift, rehash and growth, heterogeneous lookup, a colliding hash, the
// power-of-two and prime growth policies, and serialization round trip.
#include "common.h"
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
struct BadHash { std::size_t operator()(int k) const { return static_cast<std::size_t>(k % 4); } };   // 4 buckets' worth of collisions

struct SvHash {
  using is_transparent = void;
  std::size_t operator()(std::string_view s) const { return std::hash<std::string_view>()(s); }
};
struct SvEq {
  using is_transparent = void;
  bool operator()(std::string_view a, std::string_view b) const { return a == b; }
};

// serializer / deserializer over a byte vector, as the library's interface asks for
struct Out {
  std::vector<unsigned char> buf;
  template <class T> void put(const T &v) { const unsigned char *p = reinterpret_cast<const unsigned char *>(&v); buf.insert(buf.end(), p, p + sizeof v); }
  void operator()(const std::uint64_t &v) { put(v); }
  void operator()(const float &v) { put(v); }
  void operator()(const std::int16_t &v) { put(v); }
  void operator()(const std::uint32_t &v) { put(v); }
  void operator()(const bool &v) { put(v); }
  void operator()(const std::pair<int, long long> &v) { put(v.first); put(v.second); }
};
struct In {
  const std::vector<unsigned char> &buf; std::size_t pos = 0;
  template <class T> T get() { T v; memcpy(&v, &buf.at(pos), sizeof v); pos += sizeof v; return v; }
  template <class T> T operator()() { return read(static_cast<T *>(nullptr)); }
  template <class T> T read(T *) { return get<T>(); }
  std::pair<int, long long> read(std::pair<int, long long> *) { int a = get<int>(); long long b = get<long long>(); return {a, b}; }
};
}  // namespace

int main() {
  QT_BEGIN("tsl-robin-map");

  // ---- robin_map against known contents ----
  tsl::robin_map<int, long long> sq;
  QT_CHECK(sq.empty() && sq.find(1) == sq.end(), "a new map is empty");
  for (int i = 0; i < 10000; i++) sq[i] = (long long)i * i;
  long long sum = 0; std::size_t n = 0;
  for (const auto &kv : sq) { sum += kv.second; ++n; }
  // sum of squares 0..9999 = 9999*10000*19999/6
  QT_CHECK(sq.size() == 10000 && n == 10000 && sum == 333283335000LL, "10000 squares inserted; iteration visits each once");
  QT_CHECK(sq.at(123) == 15129 && sq.count(9999) == 1 && sq.count(10000) == 0 && sq.contains(0), "lookups hit and miss correctly");
  bool threw = false;
  try { sq.at(-5); } catch (const std::out_of_range &) { threw = true; }
  QT_CHECK(threw, "at() on a missing key throws std::out_of_range");
  QT_CHECK(sq.load_factor() <= sq.max_load_factor() && sq.bucket_count() >= sq.size(), "load factor within its maximum after growth");

  auto ins = sq.insert({123, -1});
  QT_CHECK(!ins.second && ins.first->second == 15129, "insert of an existing key keeps the old value");
  auto ia = sq.insert_or_assign(123, -1);
  QT_CHECK(!ia.second && sq[123] == -1, "insert_or_assign overwrites");
  auto te = sq.try_emplace(20000, 7);
  QT_CHECK(te.second && sq.size() == 10001 && sq.at(20000) == 7, "try_emplace adds a new key");

  // erase every odd key: backward shift deletion must keep all even ones findable
  std::size_t erased = 0;
  for (int i = 1; i < 10000; i += 2) erased += sq.erase(i);
  bool evens = true, odds = true;
  for (int i = 0; i < 10000; i += 2) evens = evens && sq.count(i) == 1 && (i == 123 || sq.at(i) == (long long)i * i);
  for (int i = 1; i < 10000; i += 2) odds = odds && sq.find(i) == sq.end();
  QT_CHECK(erased == 5000 && sq.size() == 5001 && evens && odds, "5000 odd keys erased, all even keys still found");
  for (auto it = sq.begin(); it != sq.end();) { if (it->first % 4 == 0) it = sq.erase(it); else ++it; }
  QT_CHECK(sq.size() == 2500 && sq.count(4) == 0 && sq.count(20000) == 0 && sq.count(6) == 1, "erase through iterators removes the 2501 multiples of 4");
  sq.rehash(1 << 16);
  QT_CHECK(sq.bucket_count() >= (1u << 16) && sq.size() == 2500 && sq.at(9998) == 9998LL * 9998, "rehash keeps the contents");
  sq.clear();
  QT_CHECK(sq.empty() && sq.begin() == sq.end(), "clear empties the map");

  // ---- same answers as std::unordered_map under a deterministic workload ----
  {
    tsl::robin_map<std::uint32_t, std::uint32_t> a;
    std::unordered_map<std::uint32_t, std::uint32_t> b;
    std::uint32_t x = 2463534242u;   // xorshift32
    for (int i = 0; i < 50000; i++) {
      x ^= x << 13; x ^= x >> 17; x ^= x << 5;
      std::uint32_t k = x % 4096;
      if (x & 0x10000) { a.erase(k); b.erase(k); } else { a[k] += x; b[k] += x; }
    }
    bool same = a.size() == b.size();
    for (const auto &kv : b) { auto it = a.find(kv.first); same = same && it != a.end() && it->second == kv.second; }
    QT_CHECK(same && !b.empty(), "50000 mixed operations leave the same contents as std::unordered_map");
  }

  // ---- heavy collisions ----
  {
    tsl::robin_map<int, int, BadHash> c;
    for (int i = 0; i < 300; i++) c.emplace(i, i + 1000);
    bool ok = c.size() == 300;
    for (int i = 0; i < 300; i++) ok = ok && c.at(i) == i + 1000;
    for (int i = 0; i < 300; i += 3) c.erase(i);
    for (int i = 0; i < 300; i++) ok = ok && (c.count(i) == (i % 3 == 0 ? 0u : 1u));
    QT_CHECK(ok && c.size() == 200, "a hash with only 4 distinct values still stores and erases correctly");
  }

  // ---- strings, heterogeneous lookup, mutable values ----
  {
    tsl::robin_map<std::string, int, SvHash, SvEq> m = {{"lidar", 1}, {"radar", 2}, {"camera", 3}};
    std::string_view key = "radar";
    QT_CHECK(m.find(key) != m.end() && m.find(key)->second == 2 && m.count(std::string_view("sonar")) == 0,
             "lookup by string_view without building a std::string");
    m.find(key).value() = 20;   // values are changed through value()
    for (auto it = m.begin(); it != m.end(); ++it) it.value() += 1;
    QT_CHECK(m.at("radar") == 21 && m.at("lidar") == 2 && m.at("camera") == 4, "it.value() modifies in place");
    tsl::robin_map<std::string, int, SvHash, SvEq> copy = m, moved = std::move(m);
    QT_CHECK(copy == moved && copy.size() == 3 && m.empty(), "copy equals the moved-to map; the moved-from map is empty");
  }

  // ---- robin_set and growth policies ----
  {
    tsl::robin_set<int> s;
    for (int i = 0; i < 1000; i++) s.insert(i % 100);
    QT_CHECK(s.size() == 100 && s.count(99) == 1 && s.count(100) == 0, "robin_set keeps 100 distinct values of 1000 inserts");
    std::size_t b = s.bucket_count();
    QT_CHECK(b != 0 && (b & (b - 1)) == 0, "default growth policy: bucket count is a power of two");
    tsl::robin_pg_set<int> p(s.begin(), s.end());
    tsl::robin_pg_map<int, int> pm;
    for (int i = 0; i < 5000; i++) pm[i * 64] = i;   // keys that all collide modulo a power of two
    bool ok = pm.size() == 5000;
    for (int i = 0; i < 5000; i++) ok = ok && pm.at(i * 64) == i;
    QT_CHECK(p.size() == 100 && p.count(42) == 1 && ok && (pm.bucket_count() & 1) == 1, "prime growth policy: odd bucket count, contents intact");
  }

  // ---- serialization ----
  {
    tsl::robin_map<int, long long> src;
    for (int i = -50; i < 50; i++) src[i] = (long long)i * 1000000007LL;
    Out out; src.serialize(out);
    In in{out.buf};
    auto back = tsl::robin_map<int, long long>::deserialize(in, true);
    QT_CHECK(!out.buf.empty() && in.pos == out.buf.size() && back == src && back.at(-50) == -50000000350LL,
             "serialize / deserialize restores 100 entries");
  }
  QT_END();
}
