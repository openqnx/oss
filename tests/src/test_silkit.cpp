// Vector SIL Kit: a registry process (sil-kit-registry, on loopback) and two
// participants that find each other through it. Checked: publish/subscribe
// -- a subscriber whose topic, media type and mandatory label match gets
// every message, in order and byte for byte, the last value of a publisher
// with history 1 reaches it even though it may subscribe after the first
// publish, and a subscriber with another label value gets nothing -- and an
// RPC call (the server adds two numbers; the client gets 42 back). The
// registry is stopped on every exit path.
// SILKIT_REGISTRY overrides the registry program (default
// /usr/local/bin/sil-kit-registry).
#include "common.h"
#include "silkit/SilKit.hpp"
#include "silkit/SilKitVersion.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/rpc/all.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std::chrono_literals;
using namespace SilKit::Services;

static pid_t registry = -1;
static char tmpdir[256];

static void stop_registry() {
  if (registry > 0) {
    kill(registry, SIGTERM);
    for (int i = 0; i < 100; ++i) {
      if (waitpid(registry, nullptr, WNOHANG) == registry) { registry = -1; return; }
      usleep(100000);
    }
    kill(registry, SIGKILL); waitpid(registry, nullptr, 0); registry = -1;
  }
}
static void cleanup() {
  stop_registry();
  if (tmpdir[0]) { std::string c = std::string("rm -rf '") + tmpdir + "'"; if (system(c.c_str())) {} tmpdir[0] = 0; }
}
static void on_signal(int) { cleanup(); _exit(2); }

static std::string slurp(const std::string &p) {
  std::ifstream f(p); std::stringstream s; s << f.rdbuf(); return s.str();
}

int main() {
  QT_BEGIN("silkit");
  printf("  info SIL Kit %s\n", SilKit::Version::String());
  const char *reg = getenv("SILKIT_REGISTRY") ? getenv("SILKIT_REGISTRY") : "/usr/local/bin/sil-kit-registry";
  // SIL Kit puts its local-domain sockets in TMPDIR: a long one overflows
  // sun_path ("File name too long").
  if (getenv("TMPDIR") && strlen(getenv("TMPDIR")) > 60) setenv("TMPDIR", "/tmp", 1);
  snprintf(tmpdir, sizeof tmpdir, "%s/qnxport-silkit.XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  if (!mkdtemp(tmpdir)) { tmpdir[0] = 0; QT_CHECK(false, "temporary directory"); QT_END(); }
  atexit(cleanup);
  signal(SIGINT, on_signal); signal(SIGTERM, on_signal); signal(SIGHUP, on_signal);

  const int port = 18500 + (int)(getpid() % 1000);
  const std::string uri = "silkit://127.0.0.1:" + std::to_string(port);
  const std::string status = std::string(tmpdir) + "/registry.status", log = std::string(tmpdir) + "/registry.log";
  registry = fork();
  if (registry == 0) {
    int fd = open(log.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); }
    execl(reg, reg, "--listen-uri", uri.c_str(), "--use-signal-handler", "--x-status-file", status.c_str(), (char *)nullptr);
    _exit(127);
  }
  bool running = false;
  for (int i = 0; i < 600 && !running; ++i) {      // 60 s
    if (waitpid(registry, nullptr, WNOHANG) == registry) { registry = -1; break; }
    running = slurp(status).find("running") != std::string::npos;
    if (!running) usleep(100000);
  }
  QT_CHECK(running, ("sil-kit-registry is running on " + uri).c_str());
  if (!running) { printf("%s", slurp(log).c_str()); QT_END(); }

  try {
    auto config = SilKit::Config::ParticipantConfigurationFromString(
        "Logging:\n  Sinks:\n    - Type: Stdout\n      Level: Warn\n");
    auto pubP = SilKit::CreateParticipant(config, "Publisher", uri);
    auto subP = SilKit::CreateParticipant(config, "Subscriber", uri);
    QT_CHECK(pubP && subP, "two participants joined through the registry");

    PubSub::PubSubSpec spec{"Telemetry", "application/x-qnx-test"};
    spec.AddLabel("vehicle", "qnx", MatchingLabel::Kind::Mandatory);
    PubSub::PubSubSpec other{"Telemetry", "application/x-qnx-test"};
    other.AddLabel("vehicle", "other", MatchingLabel::Kind::Mandatory);

    std::mutex mu; std::condition_variable cv;
    std::vector<std::vector<uint8_t>> got; std::atomic<int> wrong{0};
    auto *pub = pubP->CreateDataPublisher("TelemetryPub", spec, 1);
    pub->Publish(std::vector<uint8_t>{'h', 'i'});    // before anyone subscribes: history 1 keeps it
    subP->CreateDataSubscriber("TelemetrySub", spec, [&](PubSub::IDataSubscriber *, const PubSub::DataMessageEvent &e) {
      std::lock_guard<std::mutex> l(mu); got.emplace_back(e.data.begin(), e.data.end()); cv.notify_all();
    });
    subP->CreateDataSubscriber("OtherSub", other, [&](PubSub::IDataSubscriber *, const PubSub::DataMessageEvent &) { ++wrong; });
    {
      std::unique_lock<std::mutex> l(mu);
      cv.wait_for(l, 30s, [&] { return !got.empty(); });
      QT_CHECK(!got.empty() && got[0] == std::vector<uint8_t>({'h', 'i'}),
               "a late subscriber gets the publisher's history (the last value)");
    }
    const int N = 50;
    for (int i = 0; i < N; ++i) {
      std::vector<uint8_t> m(4 + i);
      m[0] = (uint8_t)i; m[1] = 0xA5; m[2] = (uint8_t)(255 - i); m[3] = 0x5A;
      for (int k = 4; k < 4 + i; ++k) m[k] = (uint8_t)(k * 7 + i);
      pub->Publish(m);
    }
    {
      std::unique_lock<std::mutex> l(mu);
      cv.wait_for(l, 30s, [&] { return (int)got.size() >= N + 1; });
      bool ok = (int)got.size() == N + 1;
      for (int i = 0; ok && i < N; ++i) {
        const auto &m = got[i + 1];
        ok = (int)m.size() == 4 + i && m[0] == i && m[1] == 0xA5 && m[2] == 255 - i && m[3] == 0x5A;
        for (int k = 4; ok && k < 4 + i; ++k) ok = m[k] == (uint8_t)(k * 7 + i);
      }
      printf("  info received %zu of %d messages\n", got.size() - (got.empty() ? 0 : 1), N);
      QT_CHECK(ok, "50 messages arrive in order, byte for byte");
    }
    std::this_thread::sleep_for(300ms);
    QT_CHECK(wrong == 0, "a subscriber with a different mandatory label gets nothing");

    // ---- RPC ----
    Rpc::RpcSpec rspec{"Add", "application/x-qnx-add"};
    pubP->CreateRpcServer("Adder", rspec, [](Rpc::IRpcServer *srv, const Rpc::RpcCallEvent &e) {
      int sum = 0;
      for (auto b : e.argumentData) sum += b;
      std::vector<uint8_t> r{(uint8_t)sum};
      srv->SubmitResult(e.callHandle, r);
    });
    std::atomic<int> result{-1}; std::atomic<bool> done{false}, success{false};
    auto *cli = subP->CreateRpcClient("AddClient", rspec, [&](Rpc::IRpcClient *, const Rpc::RpcCallResultEvent &e) {
      if (e.callStatus == Rpc::RpcCallStatus::Success && e.resultData.size() == 1) { result = e.resultData[0]; success = true; }
      done = true;
    });
    for (int i = 0; i < 300 && !success; ++i) {      // until the server is discovered
      done = false;
      cli->Call(std::vector<uint8_t>{40, 2});
      for (int k = 0; k < 100 && !done; ++k) std::this_thread::sleep_for(10ms);
      if (!success) std::this_thread::sleep_for(100ms);
    }
    QT_CHECK(success && result == 42, "RPC: the server adds 40 + 2 and the client gets 42");
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "SIL Kit calls do not throw");
  }
  stop_registry();
  QT_END();
}
