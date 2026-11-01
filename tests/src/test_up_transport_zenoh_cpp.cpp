// up-transport-zenoh-cpp: uProtocol publish/subscribe over Zenoh, in one
// process.
//
// Two ZenohUTransport instances, each opened from a Zenoh configuration file
// (the transport's only constructor): one listens on loopback TCP, the other
// connects to it, multicast scouting off. up-cpp's Subscriber registers on the
// second, up-cpp's Publisher publishes on the first until the message
// arrives; the received UMessage must carry the topic, the priority, a UUIDv7
// id and the payload, all of which travel as Zenoh payload and attachment.
#include "common.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <up-cpp/communication/Publisher.h>
#include <up-cpp/communication/Subscriber.h>
#include <up-cpp/datamodel/serializer/UUri.h>
#include <up-cpp/datamodel/validator/Uuid.h>
#include <up-transport-zenoh-cpp/ZenohUTransport.h>

using namespace uprotocol;
using namespace std::chrono_literals;

static std::string write_config(const char *name, const char *json) {
  std::string path = std::string("/tmp/qnxport-uptz-") + std::to_string(getpid()) + "-" + name + ".json5";
  FILE *f = fopen(path.c_str(), "w");
  if (!f) return "";
  fputs(json, f);
  fclose(f);
  return path;
}

int main() {
  QT_BEGIN("up-transport-zenoh-cpp");
  std::string ca = write_config("a", "{ mode: 'peer', scouting: { multicast: { enabled: false } },"
                                     "  listen: { endpoints: ['tcp/127.0.0.1:17450'] } }");
  std::string cb = write_config("b", "{ mode: 'peer', scouting: { multicast: { enabled: false } },"
                                     "  listen: { endpoints: ['tcp/127.0.0.1:0'] },"
                                     "  connect: { endpoints: ['tcp/127.0.0.1:17450'] } }");
  QT_CHECK(!ca.empty() && !cb.empty(), "write two Zenoh configuration files");
  try {
    auto ent_a = datamodel::serializer::uri::AsString::deserialize("//vehicle1/110AB/1/0");
    auto ent_b = datamodel::serializer::uri::AsString::deserialize("//vehicle1/120CD/1/0");
    auto ta = std::make_shared<transport::ZenohUTransport>(ent_a, ca);
    QT_CHECK(ta != nullptr, "transport A open (listening on tcp/127.0.0.1:17450)");
    auto tb = std::make_shared<transport::ZenohUTransport>(ent_b, cb);
    QT_CHECK(tb != nullptr, "transport B open (connected to A)");
    QT_CHECK(datamodel::serializer::uri::AsString::serialize(ta->getEntityUri()) == "//vehicle1/110AB/1/0",
             "A's entity URI");

    std::mutex m;
    std::atomic<int> received{0};
    v1::UMessage got;
    auto sub = communication::Subscriber::subscribe(
        tb, datamodel::serializer::uri::AsString::deserialize("//vehicle1/110AB/1/8001"),
        [&](const v1::UMessage &msg) {
          std::lock_guard<std::mutex> l(m);
          got = msg;
          received++;
        });
    QT_CHECK(sub.has_value(), "subscribe to //vehicle1/110AB/1/8001 on B");

    communication::Publisher pub(ta, datamodel::serializer::uri::AsString::deserialize("//vehicle1/110AB/1/8001"),
                                 v1::UPAYLOAD_FORMAT_TEXT, v1::UPRIORITY_CS4);
    int sent_ok = 0;
    for (int i = 0; i < 100 && !received; i++) {
      auto st = pub.publish(datamodel::builder::Payload(std::string("hello over zenoh"), v1::UPAYLOAD_FORMAT_TEXT));
      if (st.code() == v1::UCode::OK) sent_ok++;
      std::this_thread::sleep_for(100ms);
    }
    QT_CHECK(sent_ok > 0, "publish on A returns OK");
    QT_CHECK(received > 0, "B's subscriber received the message");
    {
      std::lock_guard<std::mutex> l(m);
      QT_CHECK(got.payload() == "hello over zenoh", "payload intact");
      QT_CHECK(got.attributes().type() == v1::UMESSAGE_TYPE_PUBLISH, "type PUBLISH");
      QT_CHECK(datamodel::serializer::uri::AsString::serialize(got.attributes().source()) ==
                   "//vehicle1/110AB/1/8001", "source is the topic");
      QT_CHECK(got.attributes().priority() == v1::UPRIORITY_CS4, "priority CS4");
      QT_CHECK(got.attributes().payload_format() == v1::UPAYLOAD_FORMAT_TEXT, "payload format TEXT");
      QT_CHECK(datamodel::validator::uuid::getVersion(got.attributes().id()) == 7, "id is a UUIDv7");
    }
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "no exception");
  }
  if (!ca.empty()) unlink(ca.c_str());
  if (!cb.empty()) unlink(cb.c_str());
  QT_END();
}
