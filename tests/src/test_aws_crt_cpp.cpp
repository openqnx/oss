/* aws-crt-cpp: the C++ wrapper over the AWS C runtime -- ApiHandle, hashes and
 * HMAC (known vectors), Base64, JSON build/parse, UUID, URI, DateTime, an
 * event loop group and the host resolver resolving a loopback literal. */
#include "common.h"
#include <chrono>
#include <future>
#include <string>
#include <aws/crt/Api.h>
#include <aws/crt/Types.h>
#include <aws/crt/JsonObject.h>
#include <aws/crt/UUID.h>
#include <aws/crt/DateTime.h>
#include <aws/crt/crypto/Hash.h>
#include <aws/crt/crypto/HMAC.h>
#include <aws/crt/io/EventLoopGroup.h>
#include <aws/crt/io/HostResolver.h>
#include <aws/crt/io/Uri.h>

using namespace Aws::Crt;
static std::string hex(const ByteBuf &b) {
  static const char *d = "0123456789abcdef";
  std::string s;
  for (size_t i = 0; i < b.len; ++i) { s += d[b.buffer[i] >> 4]; s += d[b.buffer[i] & 15]; }
  return s;
}
static std::string str(const ByteCursor &c) { return std::string((const char *)c.ptr, c.len); }

int main() {
  QT_BEGIN("aws-crt-cpp");
  {
    ApiHandle api;
    uint8_t dig[64];
    ByteBuf out = ByteBufFromArray(dig, sizeof dig); out.len = 0;
    QT_CHECK(Crypto::ComputeSHA256(ByteCursorFromCString("abc"), out) &&
             hex(out) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA-256(\"abc\")");
    out.len = 0;
    QT_CHECK(Crypto::ComputeMD5(ByteCursorFromCString(""), out) && hex(out) == "d41d8cd98f00b204e9800998ecf8427e", "MD5(\"\")");
    out.len = 0;
    QT_CHECK(Crypto::ComputeSHA256HMAC(ByteCursorFromCString("Jefe"), ByteCursorFromCString("what do ya want for nothing?"), out) &&
             hex(out) == "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843", "HMAC-SHA256 (RFC 4231 #2)");
    auto h = Crypto::Hash::CreateSHA1();
    h.Update(ByteCursorFromCString("ab")); h.Update(ByteCursorFromCString("c"));
    out.len = 0;
    QT_CHECK(h.Digest(out) && hex(out) == "a9993e364706816aba3e25717850c26c9cd0d89d", "streaming SHA-1 object");

    Vector<uint8_t> raw = {'Q', 'N', 'X', 0, 0xff};
    String b64 = Base64Encode(raw);
    QT_CHECK(b64 == "UU5YAP8=", "Base64Encode");
    QT_CHECK(Base64Decode(b64) == raw, "Base64Decode round trip");

    JsonObject j;
    j.WithString("name", "qnx").WithInteger("major", 8).WithBool("rt", true)
     .WithArray("arches", Vector<String>{"x86_64", "aarch64le"});
    String compact = j.View().WriteCompact();
    printf("  info json %s\n", compact.c_str());
    JsonObject parsed(compact);
    QT_CHECK(parsed.WasParseSuccessful(), "JSON written then parsed");
    JsonView v = parsed.View();
    QT_CHECK(v.GetString("name") == "qnx" && v.GetInteger("major") == 8 && v.GetBool("rt"), "JSON members read back");
    auto arr = v.GetArray("arches");
    QT_CHECK(arr.size() == 2 && arr[1].AsString() == "aarch64le", "JSON array read back");
    JsonObject bad("{\"unterminated\": ");
    QT_CHECK(!bad.WasParseSuccessful(), "malformed JSON rejected");

    UUID u("01234567-89ab-cdef-0123-456789abcdef");
    QT_CHECK(u.ToString() == "01234567-89ab-cdef-0123-456789abcdef", "UUID parse/format");
    QT_CHECK(UUID().ToString() != UUID().ToString(), "random UUIDs differ");

    Io::Uri uri(ByteCursorFromCString("https://bucket.s3.amazonaws.com:8443/path/obj.txt?versionId=3"));
    QT_CHECK(str(uri.GetScheme()) == "https" && str(uri.GetHostName()) == "bucket.s3.amazonaws.com" && uri.GetPort() == 8443,
             "URI scheme, host and port");
    QT_CHECK(str(uri.GetPath()) == "/path/obj.txt" && str(uri.GetQueryString()) == "versionId=3", "URI path and query");

    DateTime dt("Tue, 14 Nov 2023 22:13:20 GMT", DateFormat::RFC822);
    QT_CHECK(dt && dt.Millis() == 1700000000000ULL && dt.GetYear() == 2023, "DateTime parses RFC 822");

    Io::EventLoopGroup elg(1);
    QT_CHECK(bool(elg), "EventLoopGroup with one thread");
    Io::DefaultHostResolver res(elg, 8, 30);
    std::promise<std::pair<int, std::string>> pr;
    auto fut = pr.get_future();
    bool started = res.ResolveHost("127.0.0.1", [&](Io::HostResolver &, const Vector<Io::HostAddress> &a, int err) {
      std::string first = a.empty() ? "" : std::string(aws_string_c_str(a[0].address));
      pr.set_value({err, first});
    });
    QT_CHECK(started, "ResolveHost started");
    if (started && fut.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
      auto r = fut.get();
      printf("  info resolved to %s (err %d)\n", r.second.c_str(), r.first);
      QT_CHECK(r.first == 0 && r.second == "127.0.0.1", "loopback literal resolves to itself");
    } else {
      QT_CHECK(false, "resolver answered within 10 s");
    }
  }
  QT_END();
}
