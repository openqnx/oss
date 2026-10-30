#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <pistache/endpoint.h>
#include <pistache/http.h>
using namespace Pistache;
/* pistache: an HTTP endpoint on the loopback, served from its own thread
 * (its event loop -- libevent on QNX); two requests over a plain socket get
 * the handler's answers, with status, body and the resource echoed. */
struct Hello : public Http::Handler {
  HTTP_PROTOTYPE(Hello)
  void onRequest(const Http::Request &rq, Http::ResponseWriter rw) override {
    if (rq.resource() == "/hello") rw.send(Http::Code::Ok, "world");
    else rw.send(Http::Code::Not_Found, "no " + rq.resource());
  }
};
static std::string get(int port, const char *path) {
  int s = socket(AF_INET, SOCK_STREAM, 0); sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port);
  a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(s, (sockaddr *)&a, sizeof a) != 0) { close(s); return ""; }
  std::string rq = std::string("GET ") + path + " HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n";
  (void)!write(s, rq.data(), rq.size());
  std::string r; char b[1024]; ssize_t n;
  while ((n = read(s, b, sizeof b)) > 0) r.append(b, n);
  close(s); return r;
}
int main() {
  QT_BEGIN("pistache");
  int port = 19000 + getpid() % 1000;
  Http::Endpoint server(Address(Ipv4::loopback(), Port(port)));
  server.init(Http::Endpoint::options().threads(1).flags(Tcp::Options::ReuseAddr));
  server.setHandler(Http::make_handler<Hello>());
  server.serveThreaded();
  QT_CHECK(1, "endpoint serving on the loopback");
  std::string r = get(port, "/hello");
  printf("  info %s\n", r.substr(0, r.find("\r\n")).c_str());
  QT_CHECK(r.rfind("HTTP/1.1 200", 0) == 0, "GET /hello: 200");
  QT_CHECK(r.size() >= 5 && r.compare(r.size() - 5, 5, "world") == 0, "body: world");
  r = get(port, "/qnx");
  QT_CHECK(r.rfind("HTTP/1.1 404", 0) == 0 && r.find("no /qnx") != std::string::npos, "GET /qnx: 404 with the resource");
  server.shutdown();
  QT_CHECK(1, "endpoint shut down");
  QT_END();
}
