#include "common.h"
#include <mqtt/async_client.h>
#include <mqtt/message.h>
int main() {
  QT_BEGIN("paho.mqtt.cpp");
  try {
    mqtt::async_client cli("tcp://127.0.0.1:1883", "qnx_port_test_cpp");
    QT_CHECK(cli.get_client_id() == "qnx_port_test_cpp", "client id readback");
    QT_CHECK(!cli.is_connected(), "not connected before connect()");
    auto msg = mqtt::make_message("vehicle/speed", "88.5");
    msg->set_qos(1);
    QT_CHECK(msg->get_topic() == "vehicle/speed", "message topic");
    QT_CHECK(msg->to_string() == "88.5", "message payload");
    QT_CHECK(msg->get_qos() == 1, "message qos");
    mqtt::connect_options opts;
    opts.set_keep_alive_interval(20);
    QT_CHECK(opts.get_keep_alive_interval().count() == 20, "connect options");
  } catch (const mqtt::exception &e) {
    printf("  FAIL unexpected mqtt::exception: %s\n", e.what());
    return 1;
  }
  QT_END();
}
