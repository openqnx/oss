/* CommonAPI C++ SOME/IP binding: the address translator between CommonAPI
 * and SOME/IP addresses, SOME/IP method-call messages built on vsomeip, and
 * the binding's serialiser -- integers, doubles, booleans and strings written
 * to a message payload (checked byte for byte, big-endian) and read back. */
#include "common.h"
#include <string>
#include <vector>
#include <CommonAPI/CommonAPI.hpp>
/* The binding headers below are normally reached only from generated code,
   which sets this guard the same way. */
#define COMMONAPI_INTERNAL_COMPILATION
#include <CommonAPI/SomeIP/AddressTranslator.hpp>
#include <CommonAPI/SomeIP/Address.hpp>
#include <CommonAPI/SomeIP/Message.hpp>
#include <CommonAPI/SomeIP/OutputStream.hpp>
#include <CommonAPI/SomeIP/InputStream.hpp>
#undef COMMONAPI_INTERNAL_COMPILATION

using CommonAPI::EmptyDeployment;
static const EmptyDeployment *E = nullptr;

int main() {
  QT_BEGIN("capicxx-someip-runtime");
  auto tr = CommonAPI::SomeIP::AddressTranslator::get();
  QT_CHECK(tr != nullptr, "AddressTranslator singleton");
  tr->insert("local:com.qnx.test.Echo:v1_0:echo", 0x1234, 0x5678, 1, 0);
  CommonAPI::SomeIP::Address sa;
  QT_CHECK(tr->translate(std::string("local:com.qnx.test.Echo:v1_0:echo"), sa) && sa.getService() == 0x1234 &&
           sa.getInstance() == 0x5678 && sa.getMajorVersion() == 1, "CommonAPI address -> SOME/IP service 0x1234 instance 0x5678");
  CommonAPI::Address ca;
  QT_CHECK(tr->translate(CommonAPI::SomeIP::Address(0x1234, 0x5678, 1, 0), ca) && ca.getInstance() == "echo",
           "SOME/IP address -> CommonAPI address");

  CommonAPI::SomeIP::Message msg = CommonAPI::SomeIP::Message::createMethodCall(sa, 0x0421, true);
  QT_CHECK(bool(msg) && msg.isRequestType() && msg.getServiceId() == 0x1234 && msg.getMethodId() == 0x0421,
           "method call message for service 0x1234 method 0x0421");

  {
    CommonAPI::SomeIP::OutputStream out(msg);
    uint32_t u = 0x01020304; int32_t i = -2; bool t = true; double d = 1.5;
    std::string s = "QNX";
    out.writeValue(u, E).writeValue(i, E).writeValue(t, E).writeValue(d, E).writeValue(s, E);
    QT_CHECK(!out.hasError(), "serialise uint32, int32, bool, double, string");
    out.flush();
  }
  const uint8_t *p = msg.getBodyData();
  size_t n = msg.getBodyLength();
  std::string hexs;
  for (size_t k = 0; k < n && k < 40; ++k) { char h[4]; snprintf(h, sizeof h, "%02x", p[k]); hexs += h; }
  printf("  info payload (%zu bytes) %s\n", n, hexs.c_str());
  static const uint8_t head[] = {0x01, 0x02, 0x03, 0x04, 0xff, 0xff, 0xff, 0xfe, 0x01,
                                 0x3f, 0xf8, 0, 0, 0, 0, 0, 0};
  QT_CHECK(n > sizeof head && memcmp(p, head, sizeof head) == 0, "big-endian wire bytes for the fixed-size values");
  bool str_ok = false;
  for (size_t k = sizeof head; k + 3 <= n; ++k) if (memcmp(p + k, "QNX", 3) == 0) str_ok = true;
  QT_CHECK(str_ok, "string bytes present after its length field");

  CommonAPI::SomeIP::InputStream in(msg, false);
  uint32_t u2 = 0; int32_t i2 = 0; bool t2 = false; double d2 = 0; std::string s2;
  in.readValue(u2, E).readValue(i2, E).readValue(t2, E).readValue(d2, E).readValue(s2, E);
  QT_CHECK(!in.hasError(), "deserialise without error");
  QT_CHECK(u2 == 0x01020304u && i2 == -2 && t2 && d2 == 1.5 && s2 == "QNX", "values read back unchanged");

  CommonAPI::SomeIP::Message resp = msg.createResponseMessage();
  QT_CHECK(bool(resp) && resp.isResponseType() && resp.getMethodId() == 0x0421 && resp.getSessionId() == msg.getSessionId(),
           "response message mirrors method and session");
  QT_END();
}
