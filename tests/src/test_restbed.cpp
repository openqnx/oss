// restbed: an HTTP service on 127.0.0.1 (in a thread of this process) with a
// path-parameter GET resource and a POST resource that reads the request
// body; restbed's own client (Http::sync) calls both and checks status,
// headers and bodies, an unknown path gives 404, and the service stops.
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
using namespace restbed;

static std::string body_of(const std::shared_ptr<Response> &r) {
  size_t len = r->get_header("Content-Length", 0);
  if (!len) return "";
  Http::fetch(len, r);
  auto b = r->get_body();
  return std::string(b.begin(), b.end());
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
      std::string b(body.begin(), body.end());
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
    std::string base = "http://127.0.0.1:" + std::to_string(port);
    try {
      auto rq = std::make_shared<Request>(Uri(base + "/sensor/42?unit=K"));
      rq->set_header("Accept", "*/*");
      rq->set_header("Host", "127.0.0.1");
      auto rs = Http::sync(rq);
      std::string b = body_of(rs);
      printf("  info GET /sensor/42 -> %d %s\n", rs->get_status_code(), b.c_str());
      QT_CHECK(rs->get_status_code() == 200, "GET /sensor/42 -> 200");
      QT_CHECK(rs->get_header("Content-Type") == "application/json", "Content-Type header");
      QT_CHECK(b == "{\"sensor\":42,\"unit\":\"K\"}", "path and query parameters reached the handler");
    } catch (const std::exception &e) { printf("  FAIL GET threw: %s\n", e.what()); ++qt_fail_count; }

    try {
      auto rq = std::make_shared<Request>(Uri(base + "/echo"));
      rq->set_method("POST");
      std::string payload = "brake pressure 3.5 bar";
      rq->set_header("Content-Length", std::to_string(payload.size()));
      rq->set_header("Host", "127.0.0.1");
      rq->set_body(payload);
      auto rs = Http::sync(rq);
      std::string b = body_of(rs);
      printf("  info POST /echo -> %d %s\n", rs->get_status_code(), b.c_str());
      QT_CHECK(rs->get_status_code() == 201 && rs->get_header("X-Echo") == "yes", "POST /echo -> 201 with custom header");
      QT_CHECK(b == "BRAKE PRESSURE 3.5 BAR", "request body read and transformed by the handler");
    } catch (const std::exception &e) { printf("  FAIL POST threw: %s\n", e.what()); ++qt_fail_count; }

    try {
      auto rq = std::make_shared<Request>(Uri(base + "/sensor/abc"));
      rq->set_header("Host", "127.0.0.1");
      auto rs = Http::sync(rq);
      printf("  info GET /sensor/abc -> %d\n", rs->get_status_code());
      QT_CHECK(rs->get_status_code() == 404, "path not matching the {id: [0-9]+} rule -> 404");
    } catch (const std::exception &e) { printf("  FAIL 404 request threw: %s\n", e.what()); ++qt_fail_count; }
  }
  service.stop();
  th.join();
  QT_CHECK(!service.is_up(), "service stopped");
  QT_END();
}
