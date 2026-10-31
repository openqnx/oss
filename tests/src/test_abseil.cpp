/* abseil: strings, formatting, base64, CRC32C, hash containers, int128, time
 * and Status, each checked against a known answer. */
#include "common.h"
#include <string>
#include <vector>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_replace.h>
#include <absl/strings/escaping.h>
#include <absl/strings/numbers.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/crc/crc32c.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/btree_map.h>
#include <absl/numeric/int128.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/time/time.h>
#include <absl/time/civil_time.h>
#include <absl/hash/hash.h>
#include <absl/base/config.h>

static absl::StatusOr<int> parse_pos(const std::string &s) {
  int v;
  if (!absl::SimpleAtoi(s, &v)) return absl::InvalidArgumentError("not a number: " + s);
  if (v < 0) return absl::OutOfRangeError("negative");
  return v;
}

int main() {
  QT_BEGIN("abseil");
  printf("  info abseil LTS %d\n", (int)ABSL_LTS_RELEASE_VERSION);

  std::string cat = absl::StrCat("qnx", 8, ".", 0, "-", 3.5);
  QT_CHECK(cat == "qnx8.0-3.5", "StrCat mixes strings and numbers");
  std::vector<std::string> parts = absl::StrSplit("a,b,,c", ',', absl::SkipEmpty());
  QT_CHECK(parts.size() == 3 && parts[2] == "c", "StrSplit with SkipEmpty");
  QT_CHECK(absl::StrJoin(parts, "|") == "a|b|c", "StrJoin");
  QT_CHECK(absl::StrReplaceAll("one two two", {{"two", "2"}}) == "one 2 2", "StrReplaceAll");
  QT_CHECK(absl::StrFormat("%05d|%-4s|%.3f|%x", 42, "ab", 3.14159, 255) == "00042|ab  |3.142|ff",
           "StrFormat");
  QT_CHECK(absl::StartsWith("libabsl", "lib") && absl::EndsWithIgnoreCase("X.SO", ".so"), "StartsWith / EndsWithIgnoreCase");

  std::string b64 = absl::Base64Escape("Hello, QNX!");
  QT_CHECK(b64 == "SGVsbG8sIFFOWCE=", "Base64Escape known answer");
  std::string back;
  QT_CHECK(absl::Base64Unescape(b64, &back) && back == "Hello, QNX!", "Base64Unescape round trip");
  QT_CHECK(absl::BytesToHexString("\x01\xab\xff") == "01abff", "BytesToHexString");
  QT_CHECK(absl::CEscape("a\nb\"") == "a\\nb\\\"", "CEscape");

  uint32_t crc = static_cast<uint32_t>(absl::ComputeCrc32c("123456789"));
  printf("  info crc32c(\"123456789\") = 0x%08x\n", crc);
  QT_CHECK(crc == 0xE3069283u, "CRC32C check value 0xE3069283");
  absl::crc32c_t c1 = absl::ComputeCrc32c("12345"), c2 = absl::ComputeCrc32c("6789");
  QT_CHECK(static_cast<uint32_t>(absl::ConcatCrc32c(c1, c2, 4)) == 0xE3069283u, "ConcatCrc32c");

  absl::flat_hash_map<std::string, int> m;
  for (int i = 0; i < 1000; ++i) m[absl::StrCat("k", i)] = i * i;
  QT_CHECK(m.size() == 1000 && m["k31"] == 961 && !m.contains("k1000"), "flat_hash_map 1000 entries");
  m.erase("k31");
  QT_CHECK(!m.contains("k31") && m.size() == 999, "flat_hash_map erase");
  absl::btree_map<int, std::string> bt = {{3, "c"}, {1, "a"}, {2, "b"}};
  std::string order;
  for (auto &kv : bt) order += kv.second;
  QT_CHECK(order == "abc", "btree_map iterates in key order");
  QT_CHECK(absl::Hash<std::string>{}("same") == absl::Hash<std::string>{}("same"), "absl::Hash is stable in-process");

  absl::uint128 big = absl::MakeUint128(1, 0);          // 2^64
  absl::uint128 prod = big * 3 + 7;
  QT_CHECK(absl::Uint128High64(prod) == 3 && absl::Uint128Low64(prod) == 7, "uint128 arithmetic");
  absl::uint128 q = absl::Uint128Max() / absl::uint128(10);
  QT_CHECK(absl::Uint128Low64(q) == 0x9999999999999999ULL && absl::Uint128High64(q) == 0x1999999999999999ULL,
           "uint128 division (max/10)");

  auto ok = parse_pos("123"), bad = parse_pos("x1"), neg = parse_pos("-4");
  QT_CHECK(ok.ok() && *ok == 123, "StatusOr value");
  QT_CHECK(!bad.ok() && absl::IsInvalidArgument(bad.status()), "StatusOr InvalidArgument");
  QT_CHECK(neg.status().code() == absl::StatusCode::kOutOfRange, "StatusOr OutOfRange");

  absl::Time t = absl::FromUnixSeconds(1700000000);
  std::string ts = absl::FormatTime("%Y-%m-%d %H:%M:%S", t, absl::UTCTimeZone());
  printf("  info %s\n", ts.c_str());
  QT_CHECK(ts == "2023-11-14 22:13:20", "FormatTime in UTC");
  absl::Time parsed;
  std::string perr;
  QT_CHECK(absl::ParseTime(absl::RFC3339_full, "2023-11-14T22:13:20Z", &parsed, &perr) && parsed == t,
           "ParseTime RFC3339");
  QT_CHECK(absl::FormatDuration(absl::Minutes(90) + absl::Milliseconds(5)) == "1h30m0.005s", "FormatDuration");
  absl::CivilDay d(2024, 2, 28);
  d += 1;
  QT_CHECK(d.month() == 2 && d.day() == 29 && absl::GetWeekday(d) == absl::Weekday::thursday,
           "CivilDay handles the leap day");
  QT_END();
}
