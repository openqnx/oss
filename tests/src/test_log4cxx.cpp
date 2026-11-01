// Apache log4cxx: a properties configuration (FileAppender, PatternLayout
// with level, logger name, MDC and message) and an XML configuration
// (DOMConfigurator, through APR-util's expat) each written to a temporary
// directory; messages below a logger's level are dropped, a child logger
// inherits its parent's appender (and additivity=false stops it), NDC and
// MDC values appear in the output, a size-based RollingFileAppender rolls
// over to its backup file, and the log files hold exactly the lines
// expected.
#include "common.h"
#include <log4cxx/logger.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/mdc.h>
#include <log4cxx/ndc.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

using namespace log4cxx;

static std::string slurp(const std::string &p) {
  std::ifstream f(p); std::stringstream s; s << f.rdbuf(); return s.str();
}
static void spit(const std::string &p, const std::string &s) { std::ofstream f(p); f << s; }

int main() {
  QT_BEGIN("log4cxx");
  std::string dir = "/tmp/qnx-log4cxx-" + std::to_string(getpid());
  mkdir(dir.c_str(), 0700);
  spit(dir + "/log.properties",
       "log4j.rootLogger=INFO, F\n"
       "log4j.appender.F=org.apache.log4j.FileAppender\n"
       "log4j.appender.F.File=" + dir + "/props.log\n"
       "log4j.appender.F.Append=false\n"
       "log4j.appender.F.layout=org.apache.log4j.PatternLayout\n"
       "log4j.appender.F.layout.ConversionPattern=%p %c [%X{user}] %x - %m%n\n"
       "log4j.logger.app.quiet=WARN\n"
       "log4j.logger.app.own=INFO, O\n"
       "log4j.additivity.app.own=false\n"
       "log4j.appender.O=org.apache.log4j.FileAppender\n"
       "log4j.appender.O.File=" + dir + "/own.log\n"
       "log4j.appender.O.Append=false\n"
       "log4j.appender.O.layout=org.apache.log4j.PatternLayout\n"
       "log4j.appender.O.layout.ConversionPattern=%c:%m%n\n");
  PropertyConfigurator::configure(dir + "/log.properties");
  LoggerPtr app = Logger::getLogger("app"), quiet = Logger::getLogger("app.quiet"), own = Logger::getLogger("app.own");
  QT_CHECK(app->isInfoEnabled() && !app->isDebugEnabled(), "root level INFO: info enabled, debug not");
  MDC::put("user", "alice");
  NDC::push("req42");
  LOG4CXX_DEBUG(app, "dropped debug");
  LOG4CXX_INFO(app, "hello " << 42);
  LOG4CXX_INFO(quiet, "dropped info");
  LOG4CXX_WARN(quiet, "warned");
  LOG4CXX_ERROR(Logger::getLogger("app.child.deep"), "inherited");
  LOG4CXX_INFO(own, "only mine");
  NDC::pop();
  MDC::remove("user");
  LOG4CXX_INFO(app, "after");
  LogManager::shutdown();
  std::string got = slurp(dir + "/props.log");
  printf("  info props.log:\n%s", got.c_str());
  QT_CHECK(got == "INFO app [alice] req42 - hello 42\n"
                  "WARN app.quiet [alice] req42 - warned\n"
                  "ERROR app.child.deep [alice] req42 - inherited\n"
                  "INFO app [] null - after\n" ||
           got == "INFO app [alice] req42 - hello 42\n"
                  "WARN app.quiet [alice] req42 - warned\n"
                  "ERROR app.child.deep [alice] req42 - inherited\n"
                  "INFO app []  - after\n",
           "properties config: levels, inheritance, MDC and NDC in the file");
  QT_CHECK(slurp(dir + "/own.log") == "app.own:only mine\n", "additivity=false: app.own only in its own appender");

  spit(dir + "/log.xml",
       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<log4j:configuration xmlns:log4j=\"http://jakarta.apache.org/log4j/\">\n"
       "  <appender name=\"R\" class=\"org.apache.log4j.rolling.RollingFileAppender\">\n"
       "    <param name=\"File\" value=\"" + dir + "/roll.log\"/>\n"
       "    <param name=\"Append\" value=\"false\"/>\n"
       "    <rollingPolicy class=\"org.apache.log4j.rolling.FixedWindowRollingPolicy\">\n"
       "      <param name=\"FileNamePattern\" value=\"" + dir + "/roll.%i.log\"/>\n"
       "      <param name=\"MinIndex\" value=\"1\"/><param name=\"MaxIndex\" value=\"2\"/>\n"
       "    </rollingPolicy>\n"
       "    <triggeringPolicy class=\"org.apache.log4j.rolling.SizeBasedTriggeringPolicy\">\n"
       "      <param name=\"MaxFileSize\" value=\"1000\"/>\n"
       "    </triggeringPolicy>\n"
       "    <layout class=\"org.apache.log4j.PatternLayout\"><param name=\"ConversionPattern\" value=\"%-5p %m%n\"/></layout>\n"
       "  </appender>\n"
       "  <root><priority value=\"debug\"/><appender-ref ref=\"R\"/></root>\n"
       "</log4j:configuration>\n");
  xml::DOMConfigurator::configure(dir + "/log.xml");
  LoggerPtr x = Logger::getLogger("x");
  QT_CHECK(x->isDebugEnabled(), "XML config (DOMConfigurator): root level debug");
  for (int i = 0; i < 100; i++) LOG4CXX_DEBUG(x, "line " << i << " of the rolling test");
  LogManager::shutdown();
  std::string cur = slurp(dir + "/roll.log"), b1 = slurp(dir + "/roll.1.log");
  printf("  info roll.log %zu bytes, roll.1.log %zu bytes\n", cur.size(), b1.size());
  QT_CHECK(!b1.empty() && b1.size() >= 1000 && b1.size() < 1100, "RollingFileAppender rolled over at ~1000 bytes into roll.1.log");
  QT_CHECK(cur.find("DEBUG line 99 of the rolling test\n") != std::string::npos, "the last line is in the current file");
  system(("rm -rf '" + dir + "'").c_str());
  QT_END();
}
