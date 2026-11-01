#include "common.h"
#include <vsomeip/vsomeip.hpp>
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <fstream>
#include <ifaddrs.h>
#include <mutex>
#include <net/if.h>
#include <netinet/in.h>
#include <set>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
/* Message construction, then real communication between two processes:
 * - the parent is the routing manager and offers a service with one event and one method;
 * - a forked child subscribes, counts the events and calls the method, over the local
 *   UNIX-domain-socket routing (QNX patch: the routing host must actually read them);
 * - with a non-loopback IPv4 interface, SD is on and the parent also listens on the SD
 *   multicast group for the OFFER (QNX patches: no netlink, so offers must go out anyway). */

static const vsomeip::service_t SVC = 0x1234;
static const vsomeip::instance_t INST = 0x5678;
static const vsomeip::eventgroup_t EG = 0x0001;
static const vsomeip::event_t EVT = 0x8001;
static const vsomeip::method_t METHOD = 0x0421;
static const int N_EVENTS = 20, N_CALLS = 5;
static const char *SD_GROUP = "224.244.224.245";

/* First non-loopback IPv4 address, or empty. */
static std::string iface_addr() {
  struct ifaddrs *ifs = nullptr;
  std::string out;
  if (getifaddrs(&ifs) != 0) return out;
  for (struct ifaddrs *i = ifs; i; i = i->ifa_next) {
    if (!i->ifa_addr || i->ifa_addr->sa_family != AF_INET || (i->ifa_flags & IFF_LOOPBACK) || !(i->ifa_flags & IFF_UP)) continue;
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((struct sockaddr_in *)i->ifa_addr)->sin_addr, buf, sizeof buf);
    out = buf;
    break;
  }
  freeifaddrs(ifs);
  return out;
}

static void write_config(const std::string &path, const std::string &net, const std::string &unicast, int sd_port, bool sd) {
  std::ofstream f(path);
  f << "{ \"unicast\": \"" << unicast << "\", \"network\": \"" << net << "\",\n"
       "  \"logging\": { \"level\": \"warning\", \"console\": \"true\", \"file\": { \"enable\": \"false\" }, \"dlt\": \"false\" },\n"
       "  \"applications\": [ { \"name\": \"qpt_provider\", \"id\": \"0x1301\" }, { \"name\": \"qpt_consumer\", \"id\": \"0x1302\" } ],\n"
       "  \"routing\": \"qpt_provider\",\n"
       "  \"services\": [ { \"service\": \"0x1234\", \"instance\": \"0x5678\", \"unreliable\": \"" << (sd_port + 20) << "\" } ],\n"
       "  \"service-discovery\": { \"enable\": \"" << (sd ? "true" : "false") << "\", \"multicast\": \"" << SD_GROUP
    << "\", \"port\": \"" << sd_port << "\", \"protocol\": \"udp\",\n"
       "    \"initial_delay_min\": \"10\", \"initial_delay_max\": \"50\", \"repetitions_base_delay\": \"100\",\n"
       "    \"repetitions_max\": \"3\", \"ttl\": \"3\", \"cyclic_offer_delay\": \"1000\" } }\n";
}

/* Waits up to 8 s for a SOME/IP-SD message (service 0xFFFF, method 0x8100) carrying an
 * OfferService entry (type 0x01) for SVC on the SD multicast group. */
static bool sniff_offer(const std::string &ifaddr, int sd_port) {
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) return false;
  int one = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof one);
  struct sockaddr_in a = {};
  a.sin_family = AF_INET;
  a.sin_port = htons((uint16_t)sd_port);
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  struct ip_mreq m = {};
  inet_pton(AF_INET, SD_GROUP, &m.imr_multiaddr);
  inet_pton(AF_INET, ifaddr.c_str(), &m.imr_interface);
  struct timeval tv = {1, 0};
  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  bool found = false;
  if (bind(s, (struct sockaddr *)&a, sizeof a) == 0 && setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, sizeof m) == 0) {
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds(8);
    unsigned char b[1500];
    while (!found && std::chrono::steady_clock::now() < end) {
      ssize_t n = recv(s, b, sizeof b, 0);
      if (n < 16 + 12 + 16 || b[0] != 0xFF || b[1] != 0xFF || b[2] != 0x81 || b[3] != 0x00) continue;
      /* SD payload: flags(4) + entries length(4) + 16-byte entries */
      uint32_t elen = (uint32_t)b[20] << 24 | (uint32_t)b[21] << 16 | (uint32_t)b[22] << 8 | b[23];
      for (uint32_t off = 24; off + 16 <= 24 + elen && off + 16 <= (uint32_t)n; off += 16)
        if (b[off] == 0x01 && ((b[off + 4] << 8) | b[off + 5]) == SVC) found = true;
    }
  }
  close(s);
  return found;
}

/* Child: subscribe, count events in order, call the method. Exit code 0 on success. */
static int run_consumer() {
  auto rt = vsomeip::runtime::get();
  auto app = rt->create_application("qpt_consumer");
  if (!app || !app->init()) return 10;
  std::mutex mx;
  std::condition_variable cv;
  bool avail = false;
  int rx = 0, in_order = 0, replies = 0;
  app->register_availability_handler(SVC, INST, [&](vsomeip::service_t, vsomeip::instance_t, bool a) {
    std::lock_guard<std::mutex> l(mx); avail = a; cv.notify_all(); });
  app->register_message_handler(SVC, INST, EVT, [&](const std::shared_ptr<vsomeip::message> &msg) {
    auto pl = msg->get_payload();
    std::lock_guard<std::mutex> l(mx);
    if (pl && pl->get_length() == 4 && pl->get_data()[0] == (vsomeip::byte_t)rx) ++in_order;
    ++rx; cv.notify_all(); });
  app->register_message_handler(SVC, INST, METHOD, [&](const std::shared_ptr<vsomeip::message> &msg) {
    auto pl = msg->get_payload();
    std::lock_guard<std::mutex> l(mx);
    if (msg->get_message_type() == vsomeip::message_type_e::MT_RESPONSE && pl && pl->get_length() == 3 &&
        pl->get_data()[0] == 'q' && pl->get_data()[2] == (vsomeip::byte_t)replies)
      ++replies;
    cv.notify_all(); });
  app->request_service(SVC, INST);
  std::thread io([&] { app->start(); });
  int rc = 0;
  {
    std::unique_lock<std::mutex> l(mx);
    if (!cv.wait_for(l, std::chrono::seconds(10), [&] { return avail; })) rc = 11;
  }
  if (!rc) {
    app->request_event(SVC, INST, EVT, {EG}, vsomeip::event_type_e::ET_EVENT);
    app->subscribe(SVC, INST, EG);
    std::unique_lock<std::mutex> l(mx);
    if (!cv.wait_for(l, std::chrono::seconds(10), [&] { return rx >= N_EVENTS; })) rc = 12;
    else if (in_order != N_EVENTS) rc = 13;
  }
  for (int i = 0; !rc && i < N_CALLS; ++i) {
    auto req = rt->create_request();
    req->set_service(SVC); req->set_instance(INST); req->set_method(METHOD);
    std::vector<vsomeip::byte_t> d{'q', 't', (vsomeip::byte_t)i};
    req->set_payload(rt->create_payload(d));
    app->send(req);
    std::unique_lock<std::mutex> l(mx);
    if (!cv.wait_for(l, std::chrono::seconds(3), [&] { return replies > i; })) rc = 14;
  }
  app->clear_all_handler();
  app->release_service(SVC, INST);
  app->stop();
  io.join();
  return rc;
}

int main() {
  QT_BEGIN("vsomeip");
  const std::string ifaddr = iface_addr();
  const bool sd = !ifaddr.empty();
  const int sd_port = 31000 + (int)(getpid() % 2000);
  const std::string net = "qpt" + std::to_string(getpid());
  const std::string cfg = "/tmp/" + net + ".json";
  write_config(cfg, net, sd ? ifaddr : "127.0.0.1", sd_port, sd);
  setenv("VSOMEIP_CONFIGURATION", cfg.c_str(), 1);

  /* Fork before the runtime exists: each process gets its own vsomeip state. */
  fflush(stdout);
  pid_t child = fork();
  if (child == 0) {
    usleep(500000); /* let the parent become the routing manager first */
    _exit(run_consumer());
  }
  QT_CHECK(child > 0, "fork consumer process");
  if (child < 0) QT_END();

  auto rt = vsomeip::runtime::get();
  QT_CHECK(rt != nullptr, "runtime::get");
  if (!rt) { kill(child, SIGKILL); QT_END(); }

  /* Message construction (library-only). */
  auto payload = rt->create_payload();
  QT_CHECK(payload != nullptr, "create_payload");
  std::vector<vsomeip::byte_t> data{0xDE, 0xAD, 0xBE, 0xEF};
  payload->set_data(data);
  QT_CHECK(payload->get_length() == 4, "payload length");
  QT_CHECK(payload->get_data()[0] == 0xDE, "payload content");
  auto req = rt->create_request();
  QT_CHECK(req != nullptr, "create_request");
  if (req) {
    req->set_service(0x1234); req->set_instance(0x5678); req->set_method(0x0421);
    req->set_payload(payload);
    QT_CHECK(req->get_service() == 0x1234, "service id readback");
    QT_CHECK(req->get_method() == 0x0421, "method id readback");
    QT_CHECK(req->get_payload()->get_length() == 4, "payload attached to message");
  }

  /* Provider + routing manager. */
  auto app = rt->create_application("qpt_provider");
  QT_CHECK(app != nullptr, "create_application");
  QT_CHECK(app && app->get_name() == "qpt_provider", "application name readback");
  bool inited = app && app->init();
  QT_CHECK(inited, "init as routing manager");
  if (!inited) { kill(child, SIGKILL); waitpid(child, nullptr, 0); QT_END(); }
  std::atomic<bool> subscribed{false};
  std::atomic<int> served{0};
  app->register_message_handler(SVC, INST, METHOD, [&](const std::shared_ptr<vsomeip::message> &m) {
    auto resp = rt->create_response(m);
    resp->set_payload(m->get_payload());
    app->send(resp);
    ++served; });
  app->register_subscription_handler(SVC, INST, EG,
      vsomeip::subscription_handler_sec_t([&](vsomeip::client_t, const vsomeip_sec_client_t *, const std::string &, bool s) {
        if (s) subscribed = true;
        return true; }));
  app->offer_event(SVC, INST, EVT, {EG}, vsomeip::event_type_e::ET_EVENT);
  app->offer_service(SVC, INST);
  std::thread io([&] { app->start(); });

  if (sd) QT_CHECK(sniff_offer(ifaddr, sd_port), "SD OfferService seen on the multicast group");
  else printf("  skip SD offer check (no non-loopback IPv4 interface)\n");

  auto end = std::chrono::steady_clock::now() + std::chrono::seconds(15);
  while (!subscribed && std::chrono::steady_clock::now() < end) std::this_thread::sleep_for(std::chrono::milliseconds(20));
  QT_CHECK(subscribed.load(), "consumer process subscribed over local routing");
  if (subscribed) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    for (int i = 0; i < N_EVENTS; ++i) {
      std::vector<vsomeip::byte_t> d{(vsomeip::byte_t)i, 0x51, 0x4E, 0x58};
      app->notify(SVC, INST, EVT, rt->create_payload(d));
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  int st = -1;
  for (int i = 0; i < 300 && waitpid(child, &st, WNOHANG) == 0; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(100));
  if (st == -1) { kill(child, SIGKILL); waitpid(child, &st, 0); }
  int code = WIFEXITED(st) ? WEXITSTATUS(st) : -1;
  if (code) printf("  consumer exit code %d (11 no availability, 12 events missing, 13 out of order, 14 no reply)\n", code);
  QT_CHECK(code == 0, "consumer received all events in order and all method replies");
  QT_CHECK(served.load() == N_CALLS, "provider served every method call");

  app->clear_all_handler();
  app->stop_offer_service(SVC, INST);
  app->stop();
  io.join();
  unlink(cfg.c_str());
  QT_END();
}
