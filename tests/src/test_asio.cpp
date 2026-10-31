// asio (standalone, header-only): a TCP echo server on loopback driven by
// asynchronous accept/read/write, a client exchanging three messages with
// it; a UDP datagram round trip; a steady_timer; work posted from another
// thread to an io_context; address parsing.
#include "common.h"
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

using asio::ip::tcp;
using asio::ip::udp;

struct Session : std::enable_shared_from_this<Session> {
  tcp::socket sock; char data[256];
  explicit Session(tcp::socket s) : sock(std::move(s)) {}
  void read() {
    auto self = shared_from_this();
    sock.async_read_some(asio::buffer(data), [this, self](std::error_code ec, std::size_t n) {
      if (ec) return;
      for (std::size_t i = 0; i < n; i++) data[i] = (char)toupper((unsigned char)data[i]);
      asio::async_write(sock, asio::buffer(data, n), [this, self](std::error_code ec2, std::size_t) { if (!ec2) read(); });
    });
  }
};

static void accept_loop(tcp::acceptor &acc, std::atomic<int> &accepted) {
  acc.async_accept([&](std::error_code ec, tcp::socket s) {
    if (ec) return;
    accepted++;
    std::make_shared<Session>(std::move(s))->read();
    accept_loop(acc, accepted);
  });
}

int main() {
  QT_BEGIN("asio");
  printf("  info asio %d.%d.%d\n", ASIO_VERSION / 100000, ASIO_VERSION / 100 % 1000, ASIO_VERSION % 100);
  try {
    asio::io_context io;
    tcp::acceptor acc(io, tcp::endpoint(asio::ip::make_address("127.0.0.1"), 0));
    unsigned short port = acc.local_endpoint().port();
    QT_CHECK(port != 0, "acceptor bound to 127.0.0.1, ephemeral port");
    std::atomic<int> accepted{0};
    accept_loop(acc, accepted);
    auto guard = asio::make_work_guard(io);
    std::thread srv([&] { io.run(); });

    asio::io_context cio;
    tcp::socket c(cio);
    c.connect(tcp::endpoint(asio::ip::address_v4::loopback(), port));
    QT_CHECK(c.is_open(), "client connected");
    const char *msgs[] = { "hello qnx", "asio echo", "third message" };
    int good = 0;
    for (const char *m : msgs) {
      asio::write(c, asio::buffer(m, strlen(m)));
      std::string got(strlen(m), '\0');
      asio::read(c, asio::buffer(&got[0], got.size()));
      std::string want(m);
      for (auto &ch : want) ch = (char)toupper((unsigned char)ch);
      good += got == want;
    }
    QT_CHECK(good == 3, "three messages echoed back upper-cased");
    c.shutdown(tcp::socket::shutdown_both);
    c.close();
    QT_CHECK(accepted == 1, "server accepted one connection");

    // UDP
    udp::socket us(cio, udp::endpoint(asio::ip::address_v4::loopback(), 0)), uc(cio, udp::endpoint(udp::v4(), 0));
    uc.send_to(asio::buffer("ping", 4), us.local_endpoint());
    char ub[16]; udp::endpoint from;
    std::size_t un = us.receive_from(asio::buffer(ub), from);
    QT_CHECK(un == 4 && !memcmp(ub, "ping", 4) && from.port() == uc.local_endpoint().port(), "UDP datagram received, sender port right");

    // timer
    asio::steady_timer t(cio, std::chrono::milliseconds(50));
    bool fired = false;
    auto t0 = std::chrono::steady_clock::now();
    t.async_wait([&](std::error_code ec) { fired = !ec; });
    cio.run();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
    printf("  info timer fired after %lld ms\n", (long long)ms);
    QT_CHECK(fired && ms >= 45 && ms < 2000, "steady_timer 50 ms");

    // post from another thread
    std::atomic<int> posted{0};
    std::thread th([&] { for (int i = 0; i < 100; i++) asio::post(io, [&] { posted++; }); });
    th.join();
    for (int i = 0; i < 200 && posted < 100; i++) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    QT_CHECK(posted == 100, "100 handlers posted from another thread ran");

    guard.reset();
    acc.close();
    io.stop();
    srv.join();
  } catch (std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "no exception");
  }
  QT_CHECK(asio::ip::make_address("192.168.1.10").to_v4().to_uint() == 0xC0A8010Au, "make_address(192.168.1.10)");
  QT_CHECK(asio::ip::make_address("::1").is_loopback() && asio::ip::make_address("::1").is_v6(), "make_address(::1) is v6 loopback");
  std::error_code ec;
  asio::ip::make_address("300.1.1.1", ec);
  QT_CHECK((bool)ec, "make_address rejects 300.1.1.1");
  QT_END();
}
