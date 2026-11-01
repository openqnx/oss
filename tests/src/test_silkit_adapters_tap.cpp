// SIL Kit Adapter TAP: bridges a host TAP interface (io-sock's tapN on QNX,
// /dev/tapN) to a SIL Kit Ethernet network. Without a TAP device the adapter
// must refuse with its "network interface ... exists" hint. With one (the
// test, as root, runs `ifconfig tapN create`; a target without tap support
// skips this part and says so), the test brings tapN up as 10.77.<n>.1/24,
// starts a registry and the adapter, and joins the same Ethernet network as
// a SIL Kit participant. Sending a UDP datagram to 10.77.<n>.2 makes the
// host stack ARP for it: the participant must see that ARP request arrive
// through the adapter, answer it, then receive the UDP datagram itself
// (payload intact), and finally inject a UDP datagram from 10.77.<n>.2 that
// a host socket bound to 10.77.<n>.1 must receive -- both directions through
// the TAP device. Registry, adapter and tap interface are removed on every
// exit path.
#include "common.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/ethernet/all.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std::chrono_literals;
using namespace SilKit::Services::Ethernet;

static pid_t registry = -1, adapter = -1;
static char tmpdir[256];
static std::string tapname;

static void stop(pid_t &p) {
  if (p > 0) {
    kill(p, SIGTERM);
    for (int i = 0; i < 100; ++i) {
      if (waitpid(p, nullptr, WNOHANG) == p) { p = -1; return; }
      usleep(100000);
    }
    kill(p, SIGKILL); waitpid(p, nullptr, 0); p = -1;
  }
}
static void cleanup() {
  stop(adapter); stop(registry);
  if (!tapname.empty()) { std::string c = "ifconfig " + tapname + " destroy >/dev/null 2>&1"; if (system(c.c_str())) {} tapname.clear(); }
  if (tmpdir[0]) { std::string c = std::string("rm -rf '") + tmpdir + "'"; if (system(c.c_str())) {} tmpdir[0] = 0; }
}
static void on_signal(int) { cleanup(); _exit(2); }
static std::string slurp(const std::string &p) { std::ifstream f(p); std::stringstream s; s << f.rdbuf(); return s.str(); }
static pid_t spawn(const std::vector<std::string> &args, const std::string &log) {
  pid_t p = fork();
  if (p == 0) {
    int fd = open(log.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); }
    std::vector<char *> av;
    for (auto &a : args) av.push_back(const_cast<char *>(a.c_str()));
    av.push_back(nullptr);
    execv(av[0], av.data());
    _exit(127);
  }
  return p;
}
static uint16_t csum(const uint8_t *p, size_t n) {
  uint32_t s = 0;
  for (size_t i = 0; i + 1 < n; i += 2) s += (p[i] << 8) | p[i + 1];
  if (n & 1) s += p[n - 1] << 8;
  while (s >> 16) s = (s & 0xffff) + (s >> 16);
  return (uint16_t)~s;
}

int main() {
  QT_BEGIN("silkit-adapters-tap");
  const std::string bin = getenv("QNX_PREFIX") ? std::string(getenv("QNX_PREFIX")) + "/bin" : "/usr/local/bin";
  const std::string ad = bin + "/sil-kit-adapter-tap", reg = bin + "/sil-kit-registry";
  // SIL Kit puts its local-domain sockets in TMPDIR: a long one overflows
  // sun_path ("File name too long").
  if (getenv("TMPDIR") && strlen(getenv("TMPDIR")) > 60) setenv("TMPDIR", "/tmp", 1);
  snprintf(tmpdir, sizeof tmpdir, "%s/qnxport-silkit-tap.XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  if (!mkdtemp(tmpdir)) { tmpdir[0] = 0; QT_CHECK(false, "temporary directory"); QT_END(); }
  atexit(cleanup);
  signal(SIGINT, on_signal); signal(SIGTERM, on_signal); signal(SIGHUP, on_signal);
  const std::string T = tmpdir;

  // ---- usage ----
  pid_t h = spawn({ad, "--help"}, T + "/help.log");
  int st = 0; waitpid(h, &st, 0);
  std::string help = slurp(T + "/help.log");
  QT_CHECK(help.find("--tap-name") != std::string::npos && help.find("--registry-uri") != std::string::npos,
           "--help lists --tap-name and --registry-uri");

  // ---- registry ----
  const int n = 50 + (int)(getpid() % 150);
  const std::string uri = "silkit://127.0.0.1:" + std::to_string(17500 + (int)(getpid() % 1000));
  registry = spawn({reg, "--listen-uri", uri, "--use-signal-handler", "--x-status-file", T + "/reg.status"}, T + "/reg.log");
  bool running = false;
  for (int i = 0; i < 600 && !running; ++i) {
    if (waitpid(registry, nullptr, WNOHANG) == registry) { registry = -1; break; }
    running = slurp(T + "/reg.status").find("running") != std::string::npos;
    if (!running) usleep(100000);
  }
  QT_CHECK(running, ("registry running on " + uri).c_str());
  if (!running) QT_END();

  // ---- no such TAP device ----
  const std::string missing = "tap" + std::to_string(n + 200);
  pid_t m = spawn({ad, "--tap-name", missing, "--registry-uri", uri, "--name", "TapMissing"}, T + "/missing.log");
  bool ended = false;
  for (int i = 0; i < 300 && !ended; ++i) { if (waitpid(m, &st, WNOHANG) == m) ended = true; else usleep(100000); }
  if (!ended) stop(m);
  std::string ml = slurp(T + "/missing.log");
  QT_CHECK(ended && !(WIFEXITED(st) && WEXITSTATUS(st) == 0) && ml.find("exists and is operational") != std::string::npos,
           "a missing TAP device is refused with the adapter's hint");
  if (ml.find("exists and is operational") == std::string::npos) printf("%s\n", ml.c_str());

  // ---- a real TAP device ----
  tapname = "tap" + std::to_string(n);
  std::string net = "10.77." + std::to_string(n);
  std::string c = "ifconfig " + tapname + " create >/dev/null 2>&1 && ifconfig " + tapname + " inet " + net + ".1 netmask 255.255.255.0 up";
  if (system(c.c_str()) != 0) {
    printf("  info could not create %s (no io-sock tap support, or not root): the data path is not tested\n", tapname.c_str());
    tapname.clear();
    QT_END();
  }
  printf("  info %s is %s.1/24\n", tapname.c_str(), net.c_str());
  adapter = spawn({ad, "--tap-name", tapname, "--network", "TapNet", "--registry-uri", uri, "--name", "TapAdapter", "--log", "Info"},
                  T + "/adapter.log");
  // io-sock's tap (FreeBSD's if_tap) marks the interface down when its
  // device is opened or closed; it is brought up once the adapter holds
  // /dev/tapN. (Linux's TUNSETIFF leaves the link state alone.)
  bool opened = false;
  for (int i = 0; i < 300 && !opened; ++i) {
    opened = slurp(T + "/adapter.log").find("TAP device successfully opened") != std::string::npos;
    if (!opened) usleep(100000);
  }
  QT_CHECK(opened, "the adapter opened /dev/tapN");
  c = "ifconfig " + tapname + " inet " + net + ".1 netmask 255.255.255.0 up";
  QT_CHECK(system(c.c_str()) == 0, "tap interface brought up with the adapter attached");

  try {
    auto cfg = SilKit::Config::ParticipantConfigurationFromString("Logging:\n  Sinks:\n    - Type: Stdout\n      Level: Warn\n");
    auto part = SilKit::CreateParticipant(cfg, "SimHost", uri);
    auto *eth = part->CreateEthernetController("SimEth", "TapNet");
    std::mutex mu; std::condition_variable cv; std::vector<std::vector<uint8_t>> frames;
    eth->AddFrameHandler([&](IEthernetController *, const EthernetFrameEvent &e) {
      std::lock_guard<std::mutex> l(mu); frames.emplace_back(e.frame.raw.begin(), e.frame.raw.end()); cv.notify_all();
    });
    eth->Activate();

    in_addr me{}, peer{};
    inet_pton(AF_INET, (net + ".1").c_str(), &me); inet_pton(AF_INET, (net + ".2").c_str(), &peer);
    int us = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in to{}; to.sin_family = AF_INET; to.sin_port = htons(9); to.sin_addr = peer;
    const char payload[] = "qnx-tap-out";
    // Find the ARP request for .2 (resend until the adapter is connected).
    std::vector<uint8_t> arp;
    for (int tries = 0; tries < 60 && arp.empty(); ++tries) {
      sendto(us, payload, sizeof payload - 1, 0, (sockaddr *)&to, sizeof to);
      std::unique_lock<std::mutex> l(mu);
      cv.wait_for(l, 1s, [&] {
        for (auto &f : frames)
          if (f.size() >= 42 && f[12] == 0x08 && f[13] == 0x06 && f[21] == 1 && !memcmp(&f[38], &peer, 4)) { arp = f; return true; }
        return false;
      });
    }
    QT_CHECK(!arp.empty(), "the host's ARP request for .2 arrives on the SIL Kit network");
    if (!arp.empty()) {
      QT_CHECK(!memcmp(&arp[28], &me, 4), "ARP request comes from the tap's address .1");
      const uint8_t sim[6] = {0x02, 0x51, 0x4b, 0x00, 0x00, 0x02};
      uint8_t hostmac[6]; memcpy(hostmac, &arp[6], 6);
      std::vector<uint8_t> rep(42);
      memcpy(&rep[0], hostmac, 6); memcpy(&rep[6], sim, 6); rep[12] = 0x08; rep[13] = 0x06;
      const uint8_t hdr[8] = {0, 1, 8, 0, 6, 4, 0, 2};
      memcpy(&rep[14], hdr, 8); memcpy(&rep[22], sim, 6); memcpy(&rep[28], &peer, 4);
      memcpy(&rep[32], hostmac, 6); memcpy(&rep[38], &me, 4);
      { std::lock_guard<std::mutex> l(mu); frames.clear(); }
      std::vector<uint8_t> udp;
      for (int tries = 0; tries < 20 && udp.empty(); ++tries) {
        eth->SendFrame(EthernetFrame{rep});
        sendto(us, payload, sizeof payload - 1, 0, (sockaddr *)&to, sizeof to);
        std::unique_lock<std::mutex> l(mu);
        cv.wait_for(l, 1s, [&] {
          for (auto &f : frames)
            if (f.size() >= 42 && f[12] == 0x08 && f[13] == 0x00 && f[23] == 17 && !memcmp(&f[30], &peer, 4)) { udp = f; return true; }
          return false;
        });
      }
      QT_CHECK(!udp.empty() && !memcmp(&udp[0], sim, 6), "after the ARP reply the UDP datagram is sent to the simulated MAC");
      if (!udp.empty()) {
        size_t ihl = (udp[14] & 15) * 4;
        std::string got(udp.begin() + 14 + ihl + 8, udp.begin() + std::min(udp.size(), 14 + ihl + 8 + sizeof payload - 1));
        QT_CHECK(got == payload && udp[14 + ihl + 2] == 0 && udp[14 + ihl + 3] == 9, "UDP datagram to port 9 with its payload intact");
      }
      // Inject a datagram from .2 to the host.
      int rs = socket(AF_INET, SOCK_DGRAM, 0);
      sockaddr_in la{}; la.sin_family = AF_INET; la.sin_addr = me; la.sin_port = htons(40000 + n);
      bool bound = bind(rs, (sockaddr *)&la, sizeof la) == 0;
      timeval tv{0, 300000}; setsockopt(rs, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
      const char in[] = "qnx-tap-in";
      std::vector<uint8_t> f(14 + 20 + 8 + sizeof in - 1);
      memcpy(&f[0], hostmac, 6); memcpy(&f[6], sim, 6); f[12] = 0x08; f[13] = 0x00;
      uint8_t *ip = &f[14];
      ip[0] = 0x45; ip[2] = (uint8_t)((f.size() - 14) >> 8); ip[3] = (uint8_t)(f.size() - 14); ip[8] = 64; ip[9] = 17;
      memcpy(ip + 12, &peer, 4); memcpy(ip + 16, &me, 4);
      uint16_t s = csum(ip, 20); ip[10] = s >> 8; ip[11] = s & 255;
      uint8_t *u = ip + 20;
      u[0] = 0x30; u[1] = 0x39; u[2] = la.sin_port & 255; u[3] = la.sin_port >> 8;   // ports in network order
      u[4] = 0; u[5] = (uint8_t)(8 + sizeof in - 1);                                 // checksum 0: none
      memcpy(u + 8, in, sizeof in - 1);
      char buf[64]; ssize_t r = -1;
      for (int tries = 0; bound && tries < 20 && r <= 0; ++tries) {
        eth->SendFrame(EthernetFrame{f});
        r = recv(rs, buf, sizeof buf, 0);
      }
      QT_CHECK(r == (ssize_t)(sizeof in - 1) && !memcmp(buf, in, r), "a datagram injected on the SIL Kit side reaches a host socket");
      close(rs);
    }
    close(us);
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "SIL Kit calls do not throw");
  }
  if (qt_fail_count) printf("%s\n", slurp(T + "/adapter.log").c_str());
  QT_END();
}
