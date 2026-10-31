/* CommonAPI C++ core runtime: the binding-independent parts -- the Runtime
 * singleton, address parsing, Variant, Event subscription and notification,
 * Version/ByteBuffer types and the .ini reader CommonAPI configures itself
 * with. (The SOME/IP binding is test_capicxx_someip_runtime.) */
#include "common.h"
#include <cstdio>
#include <string>
#include <vector>
#include <unistd.h>
#include <CommonAPI/CommonAPI.hpp>
/* The binding headers below are normally reached only from generated code,
   which sets this guard the same way. */
#define COMMONAPI_INTERNAL_COMPILATION
#include <CommonAPI/IniFileReader.hpp>
#include <CommonAPI/Variant.hpp>
#include <CommonAPI/ByteBuffer.hpp>
#undef COMMONAPI_INTERNAL_COMPILATION

class TestEvent : public CommonAPI::Event<int, std::string> {
 public:
  void fire(int v, const std::string &s) { notifyListeners(v, s); }
  int added = 0, removed = 0;
 protected:
  void onFirstListenerAdded(const Listener &) override { ++added; }
  void onLastListenerRemoved(const Listener &) override { ++removed; }
};

int main() {
  QT_BEGIN("capicxx-core-runtime");
  auto rt = CommonAPI::Runtime::get();
  QT_CHECK(rt != nullptr && rt == CommonAPI::Runtime::get(), "Runtime::get returns one singleton");

  CommonAPI::Address a("local:com.qnx.test.Echo:v1_0:echo.instance");
  printf("  info address %s\n", a.getAddress().c_str());
  QT_CHECK(a.getDomain() == "local" && a.getInterface() == "com.qnx.test.Echo:v1_0" && a.getInstance() == "echo.instance",
           "address split into domain / interface / instance");
  CommonAPI::Address b("local", "com.qnx.test.Echo:v1_0", "echo.instance");
  QT_CHECK(a == b && !(a < b) && !(b < a), "addresses compare equal");
  b.setInstance("other");
  QT_CHECK(a != b && b.getAddress() == "local:com.qnx.test.Echo:v1_0:other", "setInstance changes the address");

  CommonAPI::Variant<int32_t, std::string, double> v = std::string("hello");
  QT_CHECK(v.isType<std::string>() && v.get<std::string>() == "hello", "Variant holds a string");
  v = 3.5;
  QT_CHECK(v.isType<double>() && !v.isType<std::string>() && v.get<double>() == 3.5, "Variant re-assigned to double");
  CommonAPI::Variant<int32_t, std::string, double> w = 3.5;
  QT_CHECK(v == w, "Variant equality");

  TestEvent ev;
  std::vector<std::string> got;
  auto s1 = ev.subscribe([&](const int &n, const std::string &s) { got.push_back(s + "#" + std::to_string(n)); });
  auto s2 = ev.subscribe([&](const int &n, const std::string &) { got.push_back("second:" + std::to_string(n)); });
  QT_CHECK(ev.added == 1, "onFirstListenerAdded called once");
  ev.fire(7, "seven");
  QT_CHECK(got.size() == 2 && got[0] == "seven#7" && got[1] == "second:7", "both listeners notified in order");
  ev.unsubscribe(s1);
  ev.fire(8, "eight");
  QT_CHECK(got.size() == 3 && got[2] == "second:8", "unsubscribed listener no longer called");
  ev.unsubscribe(s2);
  QT_CHECK(ev.removed == 1, "onLastListenerRemoved called");

  CommonAPI::Version ver(1, 2);
  QT_CHECK(ver.Major == 1 && ver.Minor == 2, "Version");
  CommonAPI::ByteBuffer bb = {0xde, 0xad, 0xbe, 0xef};
  QT_CHECK(bb.size() == 4 && bb[3] == 0xef, "ByteBuffer");

  const char *ini = "/tmp/qnxport-capicxx.ini";
  FILE *f = fopen(ini, "w");
  if (f) {
    fputs("[default]\nbinding=someip\n\n[logging]\nconsole = true\nlevel = verbose\n", f);
    fclose(f);
  }
  CommonAPI::IniFileReader rd;
  QT_CHECK(rd.load(ini), "IniFileReader loads a file");
  auto sec = rd.getSection("logging");
  QT_CHECK(sec && sec->getValue("level") == "verbose" && sec->getValue("console") == "true", "ini values (whitespace trimmed)");
  QT_CHECK(rd.getSection("default") && rd.getSection("default")->getValue("binding") == "someip", "second section");
  QT_CHECK(rd.getSection("missing") == nullptr, "unknown section is null");
  unlink(ini);
  QT_END();
}
