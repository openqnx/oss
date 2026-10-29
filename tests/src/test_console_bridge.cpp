// console_bridge: log through a custom output handler.
#include "common.h"
#include <console_bridge/console.h>
static int got = 0;
struct H : console_bridge::OutputHandler {
  void log(const std::string &t, console_bridge::LogLevel, const char *, int) override { if (t == "hello 8") got++; }
};
int main() {
  QT_BEGIN("console_bridge");
  H h; console_bridge::useOutputHandler(&h);
  console_bridge::setLogLevel(console_bridge::CONSOLE_BRIDGE_LOG_INFO);
  CONSOLE_BRIDGE_logInform("hello %d", 8);
  CONSOLE_BRIDGE_logDebug("filtered");
  QT_CHECK(got == 1, "message routed, debug filtered");
  console_bridge::restorePreviousOutputHandler();
  QT_END();
}
