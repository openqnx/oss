#include "common.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>
#include <sstream>
int main() {
  QT_BEGIN("spdlog");
  std::ostringstream oss;
  auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
  auto log = std::make_shared<spdlog::logger>("qnx", sink);
  log->set_pattern("%v");
  log->set_level(spdlog::level::debug);
  log->info("diag {} {}", "session", 0x1234);
  QT_CHECK(oss.str().find("diag session 4660") != std::string::npos, "formatted record reached sink");
  log->warn("threshold {:.1f}", 12.75);
  QT_CHECK(oss.str().find("threshold 12.8") != std::string::npos, "second record");
  printf("  info spdlog %d.%d.%d\n", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
  QT_END();
}
