// zenoh-cpp (on zenoh-c): put/subscribe and get/queryable between two
// sessions over loopback TCP, in one process.
//
// One session listens on 127.0.0.1, the other connects to it; multicast
// scouting is off, so they find each other only by the endpoint. The second
// session subscribes and declares a queryable; the first publishes until the
// sample arrives, then queries and checks the reply. Also: ZBytes
// serialization round trips (zenoh::ext) with known values.
#include "common.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <zenoh.hxx>

using namespace zenoh;
using namespace std::chrono_literals;

static Session open_session(const char *listen, const char *connect) {
  Config cfg = Config::create_default();
  cfg.insert_json5("scouting/multicast/enabled", "false");
  // A peer listens on tcp/[::]:0 by default, which fails without IPv6.
  cfg.insert_json5("listen/endpoints", listen);
  if (connect) cfg.insert_json5("connect/endpoints", connect);
  return Session::open(std::move(cfg));
}

int main() {
  QT_BEGIN("zenoh-cpp");
  init_log_from_env_or("error");
  try {
    // ---- serialization (zenoh::ext), no network
    auto b = ext::serialize(std::make_pair(std::string("answer"), int32_t(42)));
    auto p = ext::deserialize<std::pair<std::string, int32_t>>(b);
    QT_CHECK(p.first == "answer" && p.second == 42, "ext::serialize/deserialize pair<string,int32>");
    QT_CHECK(Bytes("hello").as_string() == "hello", "Bytes from a string and back");
    QT_CHECK(KeyExpr("qnx/a/b").intersects(KeyExpr("qnx/*/b")), "key expression qnx/a/b intersects qnx/*/b");
    QT_CHECK(!KeyExpr("qnx/a/b").intersects(KeyExpr("qnx/c/*")), "qnx/a/b does not intersect qnx/c/*");

    Session pub = open_session("[\"tcp/127.0.0.1:17449\"]", nullptr);
    QT_CHECK(true, "open a session listening on TCP 127.0.0.1:17449");
    Session sub = open_session("[\"tcp/127.0.0.1:0\"]", "[\"tcp/127.0.0.1:17449\"]");
    QT_CHECK(true, "open a second session connecting to it");
    QT_CHECK(pub.get_zid().bytes() != sub.get_zid().bytes(), "the sessions have different ids");

    std::mutex m;
    std::string got;
    std::atomic<int> received{0};
    auto subscriber = sub.declare_subscriber(
        KeyExpr("qnx/port-test/cpp"),
        [&](const Sample &s) {
          std::lock_guard<std::mutex> l(m);
          got = s.get_payload().as_string();
          received++;
        },
        closures::none);
    auto queryable = sub.declare_queryable(
        KeyExpr("qnx/port-test/calc"),
        [](const Query &q) {
          // Replies with the sum of the parameters "a" and "b".
          std::string params(q.get_parameters());
          int a = 0, b = 0;
          sscanf(params.c_str(), "a=%d;b=%d", &a, &b);
          q.reply(KeyExpr("qnx/port-test/calc"), std::to_string(a + b));
        },
        closures::none);

    for (int i = 0; i < 100 && !received; i++) {
      pub.put(KeyExpr("qnx/port-test/cpp"), "hello from zenoh-cpp");
      std::this_thread::sleep_for(100ms);
    }
    QT_CHECK(received > 0, "the subscriber received a sample over TCP");
    {
      std::lock_guard<std::mutex> l(m);
      QT_CHECK(got == "hello from zenoh-cpp", "with the payload intact");
    }

    // The queryable is declared on the other session: query until it answers.
    std::string reply;
    for (int i = 0; i < 20 && reply.empty(); i++) {
      std::mutex dm;
      std::condition_variable cv;
      bool done = false;
      Session::GetOptions opts;
      opts.timeout_ms = 1000;
      pub.get(KeyExpr("qnx/port-test/calc"), "a=19;b=23",
              [&](const Reply &r) {
                if (r.is_ok()) { std::lock_guard<std::mutex> l(m); reply = r.get_ok().get_payload().as_string(); }
              },
              [&]() { std::lock_guard<std::mutex> l(dm); done = true; cv.notify_all(); }, std::move(opts));
      std::unique_lock<std::mutex> l(dm);
      cv.wait_for(l, 3s, [&] { return done; });
      if (reply.empty()) std::this_thread::sleep_for(200ms);
    }
    QT_CHECK(reply == "42", "get qnx/port-test/calc?a=19;b=23 replies 42");
  } catch (const ZException &e) {
    printf("  info zenoh exception: %s\n", e.what());
    QT_CHECK(false, "no zenoh exception");
  }
  QT_END();
}
