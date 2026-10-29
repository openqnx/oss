// libzmq + cppzmq: REQ/REP over inproc, and a CURVE keypair.
#include "common.h"
#include <zmq.hpp>
int main() {
  QT_BEGIN("libzmq");
  int maj, min, pat; zmq_version(&maj, &min, &pat);
  QT_CHECK(maj == 4 && min == 3, "libzmq 4.3");
  zmq::context_t ctx;
  zmq::socket_t rep(ctx, zmq::socket_type::rep), req(ctx, zmq::socket_type::req);
  rep.bind("inproc://t"); req.connect("inproc://t");
  req.send(zmq::buffer(std::string("ping")), zmq::send_flags::none);
  zmq::message_t m; auto r = rep.recv(m);
  QT_CHECK(r && m.to_string() == "ping", "request received");
  rep.send(zmq::buffer(std::string("pong")), zmq::send_flags::none);
  r = req.recv(m);
  QT_CHECK(r && m.to_string() == "pong", "reply received");
  char pub[41], sec[41];
  QT_CHECK(zmq_curve_keypair(pub, sec) == 0, "CURVE keypair (libsodium)");
  rep.bind("tcp://127.0.0.1:*");
  QT_CHECK(rep.get(zmq::sockopt::last_endpoint).rfind("tcp://127.0.0.1:", 0) == 0, "tcp bind");
  QT_END();
}
