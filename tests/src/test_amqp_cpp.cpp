/* AMQP-CPP: drive the client through a real AMQP 0-9-1 handshake against a
 * scripted broker in the same process -- protocol header, Start/StartOk with
 * PLAIN credentials, Tune/TuneOk, Open/OpenOk, a channel, a queue declare and
 * a publish -- checking every frame the client puts on the wire. */
#include "common.h"
#include <string>
#include <vector>
#include <amqpcpp.h>

/* ---- frame building / parsing (big-endian, as AMQP is) ---- */
static void put8(std::string &s, unsigned v) { s.push_back((char)(v & 0xff)); }
static void put16(std::string &s, unsigned v) { put8(s, v >> 8); put8(s, v); }
static void put32(std::string &s, uint32_t v) { put16(s, v >> 16); put16(s, v & 0xffff); }
static void shortstr(std::string &s, const std::string &v) { put8(s, (unsigned)v.size()); s += v; }
static void longstr(std::string &s, const std::string &v) { put32(s, (uint32_t)v.size()); s += v; }
static std::string frame(unsigned type, unsigned ch, const std::string &payload) {
  std::string f; put8(f, type); put16(f, ch); put32(f, (uint32_t)payload.size()); f += payload; put8(f, 0xCE);
  return f;
}
static std::string method(unsigned cls, unsigned m, const std::string &args) {
  std::string p; put16(p, cls); put16(p, m); p += args; return p;
}
struct Frame { unsigned type, ch, cls, meth; std::string payload; };
static unsigned g16(const std::string &s, size_t o) { return ((unsigned char)s[o] << 8) | (unsigned char)s[o + 1]; }
static uint32_t g32(const std::string &s, size_t o) { return ((uint32_t)g16(s, o) << 16) | g16(s, o + 2); }

struct Handler : AMQP::ConnectionHandler {
  std::string out;
  bool ready = false, error = false;
  void onData(AMQP::Connection *, const char *b, size_t n) override { out.append(b, n); }
  void onReady(AMQP::Connection *) override { ready = true; }
  void onError(AMQP::Connection *, const char *msg) override { error = true; printf("  info onError: %s\n", msg ? msg : "(null)"); }
};
/* Pull every complete frame out of what the client wrote so far. */
static std::vector<Frame> take(Handler &h) {
  std::vector<Frame> v;
  size_t o = 0;
  while (h.out.size() - o >= 8) {
    uint32_t len = g32(h.out, o + 3);
    if (h.out.size() - o < 8 + len) break;
    Frame f{(unsigned char)h.out[o], g16(h.out, o + 1), 0, 0, h.out.substr(o + 7, len)};
    if (f.type == 1 && len >= 4) { f.cls = g16(f.payload, 0); f.meth = g16(f.payload, 2); }
    if ((unsigned char)h.out[o + 7 + len] != 0xCE) f.type = 0xff;  // bad frame-end
    v.push_back(f); o += 8 + len;
  }
  h.out.erase(0, o);
  return v;
}
static void feed(AMQP::Connection &c, const std::string &bytes) {
  size_t off = 0;
  while (off < bytes.size()) {
    uint64_t n = c.parse(bytes.data() + off, bytes.size() - off);
    if (n == 0) break;
    off += (size_t)n;
  }
}

int main() {
  QT_BEGIN("amqp-cpp");
  Handler h;
  AMQP::Connection conn(&h, AMQP::Login("qnxuser", "s3cret"), "/vh");
  QT_CHECK(h.out.size() >= 8 && h.out.compare(0, 8, std::string("AMQP\0\0\x09\x01", 8)) == 0,
           "client sends the AMQP 0-9-1 protocol header");
  h.out.erase(0, 8);

  std::string args; put8(args, 0); put8(args, 9); put32(args, 0);  // empty server-properties
  longstr(args, "PLAIN"); longstr(args, "en_US");
  feed(conn, frame(1, 0, method(10, 10, args)));
  auto fr = take(h);
  QT_CHECK(fr.size() == 1 && fr[0].cls == 10 && fr[0].meth == 11, "Connection.Start answered with StartOk");
  if (fr.size() == 1) {
    const std::string &p = fr[0].payload;
    QT_CHECK(p.find(std::string("\0qnxuser\0s3cret", 15)) != std::string::npos, "StartOk carries the PLAIN response");
    QT_CHECK(p.find("PLAIN") != std::string::npos, "StartOk selects the PLAIN mechanism");
  }

  args.clear(); put16(args, 16); put32(args, 131072); put16(args, 0);
  feed(conn, frame(1, 0, method(10, 30, args)));
  fr = take(h);
  bool tuneok = false, open = false;
  for (auto &f : fr) {
    if (f.cls == 10 && f.meth == 31) {
      printf("  info TuneOk channel-max %u frame-max %u\n", g16(f.payload, 4), (unsigned)g32(f.payload, 6));
      tuneok = g16(f.payload, 4) >= 1 && g16(f.payload, 4) <= 16 && g32(f.payload, 6) >= 4096 && g32(f.payload, 6) <= 131072;
    }
    if (f.cls == 10 && f.meth == 40) open = f.payload.size() > 5 && (unsigned char)f.payload[4] == 3 && f.payload.compare(5, 3, "/vh") == 0;
  }
  QT_CHECK(tuneok, "Tune answered with TuneOk within the offered limits");
  QT_CHECK(open, "Connection.Open names the virtual host /vh");
  QT_CHECK(!h.ready, "not ready before OpenOk");

  args.clear(); shortstr(args, "");
  feed(conn, frame(1, 0, method(10, 41, args)));
  QT_CHECK(h.ready && !h.error, "OpenOk makes the connection ready");

  AMQP::Channel chan(&conn);
  bool declared = false; std::string qname; uint32_t qcount = 99;
  chan.declareQueue("jobs", AMQP::durable).onSuccess([&](const std::string &n, uint32_t mc, uint32_t) {
    declared = true; qname = n; qcount = mc; });
  fr = take(h);
  QT_CHECK(!fr.empty() && fr[0].cls == 20 && fr[0].meth == 10 && fr[0].ch == 1, "Channel.Open on channel 1");
  args.clear(); put32(args, 0);
  feed(conn, frame(1, 1, method(20, 11, args)));
  fr = take(h);
  bool qd = false;
  for (auto &f : fr)
    if (f.cls == 50 && f.meth == 10 && f.payload.find("jobs") != std::string::npos) qd = true;
  QT_CHECK(qd, "Queue.Declare for \"jobs\" sent after OpenOk");
  args.clear(); shortstr(args, "jobs"); put32(args, 7); put32(args, 0);
  feed(conn, frame(1, 1, method(50, 11, args)));
  QT_CHECK(declared && qname == "jobs" && qcount == 7, "DeclareOk delivers name and message count to the callback");

  chan.publish("", "jobs", "hello from QNX");
  fr = take(h);
  bool pub = false, hdr = false, body = false;
  for (auto &f : fr) {
    if (f.type == 1 && f.cls == 60 && f.meth == 40) pub = f.payload.find("jobs") != std::string::npos;
    if (f.type == 2) hdr = g16(f.payload, 0) == 60 && g32(f.payload, 8) == 14;  // body size (low 32 bits)
    if (f.type == 3) body = f.payload == "hello from QNX";
  }
  QT_CHECK(pub, "Basic.Publish method frame");
  QT_CHECK(hdr, "content header announces a 14-byte body");
  QT_CHECK(body, "body frame carries the message");
  QT_CHECK(!h.error, "no protocol error reported");
  QT_END();
}
