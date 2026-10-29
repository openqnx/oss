#include "common.h"
#include <vsomeip/vsomeip.hpp>
/* Exercises the SOME/IP runtime and message construction without needing a
 * routing manager on the network: create_application/init are local, and
 * message + payload assembly is pure library code. */
int main() {
  QT_BEGIN("vsomeip");
  auto rt = vsomeip::runtime::get();
  QT_CHECK(rt != nullptr, "runtime::get");
  if (!rt) QT_END();
  auto app = rt->create_application("qnx_port_test");
  QT_CHECK(app != nullptr, "create_application");
  QT_CHECK(app && app->get_name() == "qnx_port_test", "application name readback");
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
  QT_END();
}
