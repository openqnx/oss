// restbed: an HTTP service on 127.0.0.1 (in a thread of this process) with a
// path-parameter GET resource and a POST resource that reads the request
// body; a plain socket client (restbed 5.0 removed its Http client) calls
// both and checks status, headers and bodies, an unknown path gives 404, and
// the service stops.
#include "common.h"
#include <restbed>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
using namespace restbed;

struct Reply { int status = -1; std::string head, body; };

// one HTTP/1.1 request over a new connection; the service closes it after the
// response (Connection: close), so read until EOF
static Reply http(unsigned short port, const std::string &req) {
  Reply r;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) return r;
  struct timeval tv = { 10, 0 };
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(fd, (sockaddr *)&a, sizeof a) != 0) { close(fd); return r; }
  size_t off = 0;
  while (off < req.size()) { ssize_t n = write(fd, req.data() + off, req.size() - off); if (n <= 0) break; off += (size_t)n; }
  std::string all; char buf[4096]; ssize_t n;
  while ((n = read(fd, buf, sizeof buf)) > 0) all.append(buf, (size_t)n);
  close(fd);
  size_t e = all.find("\r\n\r\n");
  r.head = all.substr(0, e == std::string::npos ? all.size() : e);
  if (e != std::string::npos) r.body = all.substr(e + 4);
  if (r.head.compare(0, 5, "HTTP/") == 0) { size_t sp = r.head.find(' '); if (sp != std::string::npos) r.status = atoi(r.head.c_str() + sp + 1); }
  return r;
}

static bool has_header(const Reply &r, const std::string &line) {
  return r.head.find("\r\n" + line + "\r\n") != std::string::npos ||
         (r.head.size() >= line.size() && r.head.compare(r.head.size() - line.size(), line.size(), line) == 0);
}

int main() {
  QT_BEGIN("restbed");
  auto sensor = std::make_shared<Resource>();
  sensor->set_path("/sensor/{id: [0-9]+}");
  sensor->set_method_handler("GET", [](const std::shared_ptr<Session> s) {
    std::string id = s->get_request()->get_path_parameter("id");
    std::string q = s->get_request()->get_query_parameter("unit", "C");
    std::string b = "{\"sensor\":" + id + ",\"unit\":\"" + q + "\"}";
    s->close(OK, b, { { "Content-Type", "application/json" }, { "Content-Length", std::to_string(b.size()) } });
  });
  auto echo = std::make_shared<Resource>();
  echo->set_path("/echo");
  echo->set_method_handler("POST", [](const std::shared_ptr<Session> s) {
    size_t len = s->get_request()->get_header("Content-Length", 0);
    s->fetch(len, [](const std::shared_ptr<Session> s2, const Bytes &body) {
      std::string b(reinterpret_cast<const char *>(body.data()), body.size());
      for (auto &c : b) c = (char)toupper((unsigned char)c);
      s2->close(CREATED, b, { { "Content-Length", std::to_string(b.size()) }, { "X-Echo", "yes" } });
    });
  });

  unsigned short port = (unsigned short)(21000 + getpid() % 20000);
  auto settings = std::make_shared<Settings>();
  settings->set_port(port);
  settings->set_bind_address("127.0.0.1");
  settings->set_worker_limit(2);
  settings->set_default_header("Connection", "close");

  Service service;
  service.publish(sensor);
  service.publish(echo);
  std::mutex m; std::condition_variable cv; bool ready = false;
  service.set_ready_handler([&](Service &) { std::lock_guard<std::mutex> g(m); ready = true; cv.notify_all(); });
  std::string start_err;
  std::thread th([&] {
    try { service.start(settings); }
    catch (const std::exception &e) { start_err = e.what(); std::lock_guard<std::mutex> g(m); ready = true; cv.notify_all(); }
  });
  {
    std::unique_lock<std::mutex> l(m);
    cv.wait_for(l, std::chrono::seconds(20), [&] { return ready; });
  }
  if (!start_err.empty()) printf("  info service.start: %s\n", start_err.c_str());
  QT_CHECK(ready && start_err.empty() && service.is_up(), "service up on 127.0.0.1");

  if (service.is_up()) {
    Reply g = http(port, "GET /sensor/42?unit=K HTTP/1.1\r\nHost: 127.0.0.1\r\nAccept: */*\r\nConnection: close\r\n\r\n");
    printf("  info GET /sensor/42 -> %d %s\n", g.status, g.body.c_str());
    QT_CHECK(g.status == 200, "GET /sensor/42 -> 200");
    QT_CHECK(has_header(g, "Content-Type: application/json"), "Content-Type header");
    QT_CHECK(g.body == "{\"sensor\":42,\"unit\":\"K\"}", "path and query parameters reached the handler");

    std::string payload = "brake pressure 3.5 bar";
    Reply p = http(port, "POST /echo HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: " + std::to_string(payload.size()) +
                         "\r\nConnection: close\r\n\r\n" + payload);
    printf("  info POST /echo -> %d %s\n", p.status, p.body.c_str());
    QT_CHECK(p.status == 201 && has_header(p, "X-Echo: yes"), "POST /echo -> 201 with custom header");
    QT_CHECK(p.body == "BRAKE PRESSURE 3.5 BAR", "request body read and transformed by the handler");

    Reply n = http(port, "GET /sensor/abc HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n");
    printf("  info GET /sensor/abc -> %d\n", n.status);
    QT_CHECK(n.status == 404, "path not matching the {id: [0-9]+} rule -> 404");
  }
  service.stop();
  th.join();
  QT_CHECK(!service.is_up(), "service stopped");
  QT_END();
}
