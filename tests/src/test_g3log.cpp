/* g3log: asynchronous logging through a custom sink and the default file
 * sink -- stream and printf-style messages, levels, LOG_IF, file and line
 * metadata, ordering, and the flush on worker shutdown. */
#include "common.h"
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

struct Rec { std::string level, msg, file; int line; };
static std::mutex mtx;
static std::vector<Rec> recs;
struct CaptureSink {
  void receive(g3::LogMessageMover m) {
    std::lock_guard<std::mutex> g(mtx);
    recs.push_back({m.get().level(), m.get().message(), m.get().file(), atoi(m.get().line().c_str())});
  }
};

int main() {
  QT_BEGIN("g3log");
  std::string logfile;
  int line_info = 0;
  {
    auto worker = g3::LogWorker::createLogWorker();
    auto sink = worker->addSink(std::make_unique<CaptureSink>(), &CaptureSink::receive);
    auto fh = worker->addDefaultLogger("qnxport-g3log", "/tmp/");
    g3::initializeLogging(worker.get());
    QT_CHECK(g3::internal::isLoggingInitialized(), "logging initialised with two sinks");
    logfile = fh->call(&g3::FileSink::fileName).get();
    printf("  info log file %s\n", logfile.c_str());

    line_info = __LINE__ + 1;
    LOG(INFO) << "hello from " << "QNX " << 8;
    LOGF(WARNING, "value=%d hex=%x str=%s", 42, 255, "ok");
    LOG_IF(INFO, 1 + 1 == 3) << "never logged";
    LOG_IF(DEBUG, true) << "conditional debug";
    for (int i = 0; i < 100; ++i) LOG(INFO) << "seq " << i;
    g3::internal::shutDownLogging();   // drains the queue into the sinks
  }
  std::lock_guard<std::mutex> g(mtx);
  printf("  info %zu record(s) captured\n", recs.size());
  QT_CHECK(recs.size() == 103, "103 records delivered (LOG_IF false suppressed)");
  if (recs.size() >= 3) {
    QT_CHECK(recs[0].level == "INFO" && recs[0].msg == "hello from QNX 8", "stream message and level");
    QT_CHECK(recs[0].line == line_info && recs[0].file.find("test_g3log") != std::string::npos, "file and line captured");
    QT_CHECK(recs[1].level == "WARNING" && recs[1].msg == "value=42 hex=ff str=ok", "printf-style LOGF");
    QT_CHECK(recs[2].level == "DEBUG" && recs[2].msg == "conditional debug", "LOG_IF(true) logged at DEBUG");
    bool ordered = recs.size() == 103;
    for (int i = 0; ordered && i < 100; ++i) ordered = recs[3 + i].msg == "seq " + std::to_string(i);
    QT_CHECK(ordered, "100 messages arrive in order");
  }
  std::ifstream in(logfile);
  std::stringstream ss; ss << in.rdbuf();
  std::string text = ss.str();
  QT_CHECK(text.find("hello from QNX 8") != std::string::npos && text.find("value=42 hex=ff str=ok") != std::string::npos &&
           text.find("seq 99") != std::string::npos, "file sink wrote the messages");
  QT_CHECK(text.find("never logged") == std::string::npos, "suppressed message absent from the file");
  if (!logfile.empty()) unlink(logfile.c_str());
  QT_END();
}
