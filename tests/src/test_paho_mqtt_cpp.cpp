// paho.mqtt.cpp: a real MQTT exchange on loopback.  A mosquitto broker is
// started on 127.0.0.1 with a minimal config in /tmp; a client connects,
// subscribes with a wildcard, publishes at QoS 1 and receives its own message
// (topic, payload, QoS checked); a retained message is delivered to a second
// client that subscribes afterwards; a connect to a port with no broker fails.
#include "common.h"
#include <mqtt/async_client.h>
#include <mqtt/message.h>
#include <chrono>
#include <cstdlib>
#include <string>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std::chrono;

static bool port_open(unsigned short port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  bool ok = connect(fd, (sockaddr *)&a, sizeof a) == 0;
  close(fd);
  return ok;
}

int main() {
  QT_BEGIN("paho.mqtt.cpp");
  unsigned short port = (unsigned short)(30000 + getpid() % 20000);
  std::string dir = "/tmp/qnxport-mqtt-" + std::to_string(getpid());
  mkdir(dir.c_str(), 0755);
  std::string conf = dir + "/mosquitto.conf";
  FILE *f = fopen(conf.c_str(), "w");
  fprintf(f, "listener %u 127.0.0.1\nallow_anonymous true\npersistence false\nuser root\n"
             "log_dest file %s/mosquitto.log\n", port, dir.c_str());
  fclose(f);
  pid_t broker = fork();
  if (broker == 0) {
    execl("/usr/local/sbin/mosquitto", "mosquitto", "-c", conf.c_str(), (char *)nullptr);
    execlp("mosquitto", "mosquitto", "-c", conf.c_str(), (char *)nullptr);
    _exit(127);
  }
  bool up = false;
  for (int i = 0; i < 100 && !up; i++) {
    if (port_open(port)) { up = true; break; }
    int st; if (waitpid(broker, &st, WNOHANG) == broker) { printf("  info mosquitto exited, status 0x%x\n", st); broker = -1; break; }
    usleep(100000);
  }
  QT_CHECK(up, "mosquitto broker listening on 127.0.0.1");

  std::string uri = "tcp://127.0.0.1:" + std::to_string(port);
  auto msg = mqtt::make_message("vehicle/speed", "88.5");
  msg->set_qos(1);
  QT_CHECK(msg->get_topic() == "vehicle/speed" && msg->to_string() == "88.5" && msg->get_qos() == 1, "message object");

  if (up) try {
    mqtt::async_client cli(uri, "qnx_port_test_pub");
    QT_CHECK(cli.get_client_id() == "qnx_port_test_pub" && !cli.is_connected(), "client created, not connected");
    auto opts = mqtt::connect_options_builder().clean_session(true).keep_alive_interval(seconds(20)).finalize();
    cli.start_consuming();
    auto tok = cli.connect(opts);
    QT_CHECK(tok->wait_for(seconds(10)) && cli.is_connected(), "connected to the broker");
    QT_CHECK(cli.subscribe("qnx/test/#", 1)->wait_for(seconds(10)), "subscribed to qnx/test/# at QoS 1");
    auto dt = cli.publish(mqtt::make_message("qnx/test/speed", "88.5 km/h", 1, false));
    QT_CHECK(dt->wait_for(seconds(10)), "QoS 1 publish acknowledged (PUBACK)");
    mqtt::const_message_ptr got;
    bool rx = cli.try_consume_message_for(&got, seconds(10)) && got;
    if (rx) printf("  info received %s = \"%s\" (qos %d)\n", got->get_topic().c_str(), got->to_string().c_str(), got->get_qos());
    QT_CHECK(rx && got->get_topic() == "qnx/test/speed" && got->to_string() == "88.5 km/h", "own message received through the broker");
    QT_CHECK(rx && got->get_qos() == 1, "delivered at QoS 1");

    // binary payload round trip
    std::string bin("\x00\x01\xfe\xff payload", 12);
    cli.publish("qnx/test/bin", bin.data(), bin.size(), 1, false)->wait_for(seconds(10));
    rx = cli.try_consume_message_for(&got, seconds(10)) && got;
    QT_CHECK(rx && got->get_payload() == bin, "binary payload (with NUL bytes) intact");

    QT_CHECK(cli.publish(mqtt::make_message("qnx/retained/state", "parked", 1, true))->wait_for(seconds(10)), "retained publish");

    mqtt::async_client late(uri, "qnx_port_test_sub");
    late.start_consuming();
    QT_CHECK(late.connect(opts)->wait_for(seconds(10)), "second client connected");
    late.subscribe("qnx/retained/+", 1)->wait_for(seconds(10));
    rx = late.try_consume_message_for(&got, seconds(10)) && got;
    QT_CHECK(rx && got->is_retained() && got->to_string() == "parked", "late subscriber receives the retained message");
    late.unsubscribe("qnx/retained/+")->wait_for(seconds(10));
    late.disconnect()->wait_for(seconds(10));
    QT_CHECK(!late.is_connected(), "second client disconnected");

    cli.unsubscribe("qnx/test/#")->wait_for(seconds(10));
    cli.disconnect()->wait_for(seconds(10));
    QT_CHECK(!cli.is_connected(), "disconnected");
  } catch (const mqtt::exception &e) {
    printf("  FAIL unexpected mqtt::exception: %s\n", e.what());
    ++qt_fail_count;
  }

  // nothing listens on port+1: connect must fail
  try {
    mqtt::async_client nob("tcp://127.0.0.1:" + std::to_string(port + 1), "qnx_port_test_none");
    auto o = mqtt::connect_options_builder().connect_timeout(seconds(3)).finalize();
    nob.connect(o)->wait();
    QT_CHECK(false, "connect to a port without a broker fails");
  } catch (const mqtt::exception &e) {
    printf("  info expected failure: %s\n", e.what());
    QT_CHECK(true, "connect to a port without a broker fails");
  }

  if (broker > 0) { kill(broker, SIGTERM); waitpid(broker, nullptr, 0); }
  if (qt_fail_count) {
    FILE *l = fopen((dir + "/mosquitto.log").c_str(), "r"); char line[256];
    while (l && fgets(line, sizeof line, l)) printf("  info log: %s", line);
    if (l) fclose(l);
  }
  (void)!system(("rm -rf " + dir).c_str());
  QT_END();
}
