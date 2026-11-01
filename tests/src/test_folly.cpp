// folly: containers (fbstring, small_vector, F14 maps, sorted_vector_map,
// ConcurrentHashMap), conversions (to<>, with double-conversion for
// floating point), string utilities, dynamic and JSON, Expected/Optional,
// Synchronized, futures on a CPU thread pool (thenValue chains, collectAll,
// exceptions, timeouts), an EventBase timer and cross-thread
// runInEventBaseThread, IOBuf chains, the compression codecs (zlib, zstd,
// lz4, lzma, bzip2) round-tripping data, and a coroutine task.
#include "common.h"
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <folly/Conv.h>
#include <folly/Expected.h>
#include <folly/FBString.h>
#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <folly/compression/Compression.h>
#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/container/F14Map.h>
#include <folly/container/small_vector.h>
#include <folly/sorted_vector_types.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseThread.h>
#include <folly/json/dynamic.h>
#include <folly/json/json.h>

static folly::coro::Task<int> add_later(int a, int b) { co_return a + b; }
static folly::coro::Task<int> sum_three() {
  int x = co_await add_later(1, 2);
  int y = co_await add_later(x, 4);
  co_return y * 6;
}

int main() {
  QT_BEGIN("folly");
  try {
    // containers
    folly::fbstring fs("hello");
    fs += ", QNX";
    QT_CHECK(fs == "hello, QNX" && fs.size() == 10 && fs.find("QNX") == 7, "fbstring");
    folly::small_vector<int, 4> sv{1, 2, 3};
    const int *inline_data = sv.data();
    bool inside = (const char *)inline_data >= (const char *)&sv && (const char *)inline_data < (const char *)&sv + sizeof sv;
    for (int i = 4; i <= 10; i++) sv.push_back(i);
    bool moved_out = !((const char *)sv.data() >= (const char *)&sv && (const char *)sv.data() < (const char *)&sv + sizeof sv);
    QT_CHECK(inside && moved_out && sv.size() == 10 && sv[9] == 10 && sv.capacity() >= 10, "small_vector: inline, then on the heap");
    folly::F14FastMap<std::string, int> fm;
    for (int i = 0; i < 1000; i++) fm[folly::to<std::string>("k", i)] = i * i;
    QT_CHECK(fm.size() == 1000 && fm.at("k31") == 961 && fm.count("k1000") == 0, "F14FastMap: 1000 entries");
    folly::sorted_vector_map<int, std::string> sm{{3, "c"}, {1, "a"}, {2, "b"}};
    QT_CHECK(sm.begin()->second == "a" && sm.rbegin()->first == 3 && sm.find(2)->second == "b", "sorted_vector_map keeps order");

    // conversions and strings
    QT_CHECK(folly::to<int>("12345") == 12345 && folly::to<std::string>(-42) == "-42", "to<int>, to<string>");
    QT_CHECK(folly::to<std::string>(0.1) == "0.1" && folly::to<double>("2.5e-3") == 0.0025, "double conversions (shortest round trip)");
    auto bad = folly::tryTo<int>("12x");
    QT_CHECK(bad.hasError(), "tryTo<int>(\"12x\") reports an error");
    bool threw = false;
    try { folly::to<uint8_t>(300); } catch (const folly::ConversionError &) { threw = true; }
    QT_CHECK(threw, "to<uint8_t>(300) throws ConversionError (overflow)");
    std::vector<folly::StringPiece> parts;
    folly::split(',', "a,b,,c", parts);
    QT_CHECK(parts.size() == 4 && parts[3] == "c" && folly::join("-", parts) == "a-b--c", "split and join");
    QT_CHECK(folly::stringPrintf("%05d|%s", 42, "x") == "00042|x" && folly::trimWhitespace("  t  ") == "t", "stringPrintf, trimWhitespace");
    QT_CHECK(folly::hexlify(std::string("\x01\xab", 2)) == "01ab", "hexlify");

    // dynamic / JSON
    folly::dynamic d = folly::parseJson(R"({"name":"qnx","n":[1,2,3],"pi":3.25,"ok":true,"nested":{"x":null}})");
    QT_CHECK(d["name"] == "qnx" && d["n"].size() == 3 && d["n"][2] == 3 && d["pi"].asDouble() == 3.25 && d["ok"].asBool() &&
             d["nested"]["x"].isNull(), "parseJson");
    d["added"] = folly::dynamic::array(1, "two");
    folly::json::serialization_opts so; so.sort_keys = true;
    std::string js = folly::json::serialize(d, so);
    QT_CHECK(js == R"({"added":[1,"two"],"n":[1,2,3],"name":"qnx","nested":{"x":null},"ok":true,"pi":3.25})", "toJson with sorted keys");
    QT_CHECK(folly::parseJson(js) == d, "JSON round trip");

    // Expected / Optional / Synchronized
    auto half = [](int x) -> folly::Expected<int, std::string> {
      if (x % 2) return folly::makeUnexpected(std::string("odd"));
      return x / 2;
    };
    QT_CHECK(half(8).value() == 4 && half(7).error() == "odd", "Expected");
    folly::Optional<int> o; QT_CHECK(!o && o.value_or(5) == 5, "Optional");
    folly::Synchronized<std::vector<int>> sync;
    {
      std::vector<std::thread> ts;
      for (int t = 0; t < 4; t++) ts.emplace_back([&] { for (int i = 0; i < 1000; i++) sync.wlock()->push_back(i); });
      for (auto &t : ts) t.join();
    }
    QT_CHECK(sync.rlock()->size() == 4000, "Synchronized vector: 4 threads x 1000 pushes");
    folly::ConcurrentHashMap<int, int> chm;
    {
      std::vector<std::thread> ts;
      for (int t = 0; t < 4; t++) ts.emplace_back([&, t] { for (int i = 0; i < 500; i++) chm.insert_or_assign(t * 500 + i, i); });
      for (auto &t : ts) t.join();
    }
    QT_CHECK(chm.size() == 2000 && chm.find(1999)->second == 499, "ConcurrentHashMap: 2000 entries from 4 threads");

    // futures on a thread pool
    {
      folly::CPUThreadPoolExecutor pool(4);
      auto f = folly::via(&pool, [] { return 6; }).thenValue([](int v) { return v * 7; }).thenValue([](int v) { return folly::to<std::string>(v); });
      QT_CHECK(std::move(f).get() == "42", "via(pool).thenValue chain: 42");
      std::vector<folly::Future<int>> fs2;
      for (int i = 1; i <= 10; i++) fs2.push_back(folly::via(&pool, [i] { return i * i; }));
      auto all = folly::collectAll(std::move(fs2)).get();
      int s = 0; for (auto &t : all) s += t.value();
      QT_CHECK(all.size() == 10 && s == 385, "collectAll of 10 futures: sum of squares 385");
      auto e = folly::via(&pool, []() -> int { throw std::runtime_error("boom"); })
                   .thenError(folly::tag_t<std::runtime_error>{}, [](const std::runtime_error &ex) { return int(strlen(ex.what())); });
      QT_CHECK(std::move(e).get() == 4, "exception caught by thenError");
      folly::Promise<int> never;
      bool timed_out = false;
      try { never.getFuture().get(std::chrono::milliseconds(50)); } catch (const folly::FutureTimeout &) { timed_out = true; }
      QT_CHECK(timed_out, "get() with a 50 ms timeout throws FutureTimeout");
      QT_CHECK(folly::coro::blockingWait(sum_three()) == 42, "coroutine Task: ((1+2)+4)*6 = 42");
      QT_CHECK(folly::coro::blockingWait(folly::coro::co_withExecutor(&pool, add_later(20, 22))) == 42, "coroutine scheduled on the pool");
    }

    // EventBase
    {
      folly::EventBaseThread ebt;
      folly::EventBase *eb = ebt.getEventBase();
      folly::Promise<std::chrono::steady_clock::time_point> fired;
      auto fut = fired.getSemiFuture();
      auto t0 = std::chrono::steady_clock::now();
      eb->runInEventBaseThread([&] { eb->scheduleAt([&] { fired.setValue(std::chrono::steady_clock::now()); },
                                                    eb->now() + std::chrono::milliseconds(30)); });
      auto t1 = std::move(fut).get(std::chrono::seconds(5));
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
      printf("  info EventBase timer fired after %lld ms\n", (long long)ms);
      QT_CHECK(ms >= 25 && ms < 2000, "EventBase timer (30 ms) on its own thread");
      std::atomic<bool> in_loop{false};
      eb->runInEventBaseThreadAndWait([&] { in_loop = eb->isInEventBaseThread(); });
      QT_CHECK(in_loop, "runInEventBaseThreadAndWait runs on the loop thread");
    }

    // IOBuf
    {
      auto a = folly::IOBuf::copyBuffer("Hello, ");
      a->appendToChain(folly::IOBuf::copyBuffer("IOBuf "));
      a->appendToChain(folly::IOBuf::copyBuffer("world"));
      QT_CHECK(a->countChainElements() == 3 && a->computeChainDataLength() == 18, "IOBuf chain of 3, 18 bytes");
      folly::io::Cursor c(a.get());
      QT_CHECK(c.readFixedString(18) == "Hello, IOBuf world", "Cursor reads across the chain");
      a->coalesce();
      QT_CHECK(!a->isChained() && a->moveToFbString() == "Hello, IOBuf world", "coalesce");
    }

    // compression codecs
    {
      std::string data;
      for (int i = 0; i < 2000; i++) data += folly::to<std::string>("line ", i % 50, " of the qnx folly test\n");
      using folly::compression::CodecType;
      struct { CodecType t; const char *n; } codecs[] = {
        { CodecType::ZLIB, "zlib" }, { CodecType::GZIP, "gzip" }, { CodecType::ZSTD, "zstd" }, { CodecType::LZ4, "lz4" },
        { CodecType::LZ4_FRAME, "lz4 frame" }, { CodecType::LZMA2, "lzma2" }, { CodecType::BZIP2, "bzip2" } };
      int good = 0; std::string sizes;
      for (auto &cc : codecs) {
        if (!folly::compression::hasCodec(cc.t)) { printf("  info codec %s not built in\n", cc.n); continue; }
        auto codec = folly::compression::getCodec(cc.t);
        auto in = folly::IOBuf::copyBuffer(data);
        auto z = codec->compress(in.get());
        auto back = codec->uncompress(z.get(), data.size());
        std::string out = back->moveToFbString().toStdString();
        if (out == data && z->computeChainDataLength() < data.size() / 4) good++;
        sizes += folly::to<std::string>(" ", cc.n, "=", z->computeChainDataLength());
      }
      printf("  info %zu bytes compressed:%s\n", data.size(), sizes.c_str());
      QT_CHECK(good == 7, "7 codecs compress to under a quarter and uncompress exactly");
    }
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "no unexpected exception");
  }
  QT_END();
}
