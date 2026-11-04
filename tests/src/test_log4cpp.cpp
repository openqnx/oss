/* log4cpp: logging with exact expected output -- an OstreamAppender with a
 * PatternLayout writing into a string stream, priority thresholds (DEBUG
 * dropped under INFO, ERROR kept), the category hierarchy (a child inherits
 * its parent's appender and priority; additivity off stops that), the
 * stream interface (cat.warnStream() << ...), NDC context in the pattern,
 * a FileAppender writing a file that is read back, a RollingFileAppender
 * rolling over to name.1, and configuration from a properties file. */
#include "common.h"
#include <log4cpp/Category.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/NDC.hh>
#include <log4cpp/Priority.hh>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static std::string slurp(const std::string &p) { std::ifstream f(p); std::stringstream s; s << f.rdbuf(); return s.str(); }

int main() {
  QT_BEGIN("log4cpp");
  const char *td = std::getenv("TMPDIR");
  std::string dir = std::string(td && *td ? td : "/tmp") + "/log4cpp-test." + std::to_string(getpid());
  mkdir(dir.c_str(), 0700);

  std::ostringstream os;
  auto *app = new log4cpp::OstreamAppender("mem", &os);
  auto *lay = new log4cpp::PatternLayout(); lay->setConversionPattern("%p %c: %m%n");
  app->setLayout(lay);
  log4cpp::Category &root = log4cpp::Category::getRoot();
  root.setPriority(log4cpp::Priority::INFO);
  root.addAppender(app);

  root.debug("dropped");
  root.info("hello %d", 42);
  root.error("bad thing");
  if (os.str() != "INFO : hello 42\nERROR : bad thing\n") printf("  info got: [%s]\n", os.str().c_str());
  QT_CHECK(os.str() == "INFO : hello 42\nERROR : bad thing\n", "root: DEBUG dropped under INFO; printf-style message");

  os.str("");
  log4cpp::Category &net = log4cpp::Category::getInstance("app.net");
  net.warn("timeout");
  net.warnStream() << "retry " << 3;
  QT_CHECK(os.str() == "WARN app.net: timeout\nWARN app.net: retry 3\n", "child category inherits the root appender; stream API");
  QT_CHECK(net.getChainedPriority() == log4cpp::Priority::INFO, "child inherits priority INFO");
  net.setPriority(log4cpp::Priority::DEBUG);
  os.str(""); net.debug("now visible");
  QT_CHECK(os.str() == "DEBUG app.net: now visible\n", "child priority DEBUG overrides the parent");

  log4cpp::Category &quiet = log4cpp::Category::getInstance("app.quiet");
  std::string flog = dir + "/q.log";
  auto *fa = new log4cpp::FileAppender("file", flog);
  auto *fl = new log4cpp::PatternLayout(); fl->setConversionPattern("[%x] %m%n"); fa->setLayout(fl);
  quiet.addAppender(fa);
  quiet.setAdditivity(false);
  os.str("");
  log4cpp::NDC::push("req-7");
  quiet.notice("to file only");
  log4cpp::NDC::pop();
  QT_CHECK(os.str().empty(), "additivity off: nothing reaches the root appender");
  fa->close();
  QT_CHECK(slurp(flog) == "[req-7] to file only\n", "FileAppender wrote \"[req-7] to file only\" (NDC in %x)");

  /* rolling: 200-byte files, 1 backup */
  std::string rlog = dir + "/r.log";
  log4cpp::Category &roll = log4cpp::Category::getInstance("roll");
  roll.setAdditivity(false);
  auto *ra = new log4cpp::RollingFileAppender("roll", rlog, 200, 1);
  auto *rl = new log4cpp::PatternLayout(); rl->setConversionPattern("%m%n"); ra->setLayout(rl);
  roll.addAppender(ra);
  for (int i = 0; i < 30; i++) roll.info("line %02d of the rolling test", i);
  struct stat st;
  QT_CHECK(stat((rlog + ".1").c_str(), &st) == 0 && st.st_size >= 200, "RollingFileAppender rolled over to r.log.1");
  QT_CHECK(stat(rlog.c_str(), &st) == 0 && st.st_size < 230 && slurp(rlog).find("line 29") != std::string::npos,
           "current r.log holds the newest lines");

  /* configuration file */
  std::string cfg = dir + "/log4cpp.properties", clog = dir + "/c.log";
  std::ofstream(cfg) << "log4cpp.rootCategory=ERROR, A\n"
                        "log4cpp.category.cfg=DEBUG, B\n"
                        "log4cpp.additivity.cfg=false\n"
                        "log4cpp.appender.A=FileAppender\n"
                        "log4cpp.appender.A.fileName=" << dir << "/root.log\n"
                        "log4cpp.appender.A.layout=BasicLayout\n"
                        "log4cpp.appender.B=FileAppender\n"
                        "log4cpp.appender.B.fileName=" << clog << "\n"
                        "log4cpp.appender.B.layout=PatternLayout\n"
                        "log4cpp.appender.B.layout.ConversionPattern=%c|%p|%m%n\n";
  bool cfg_ok = true;
  try { log4cpp::PropertyConfigurator::configure(cfg); } catch (const std::exception &e) { cfg_ok = false; printf("  info %s\n", e.what()); }
  QT_CHECK(cfg_ok, "PropertyConfigurator read the file");
  log4cpp::Category::getInstance("cfg").debug("configured");
  log4cpp::Category::shutdown();
  QT_CHECK(slurp(clog) == "cfg|DEBUG|configured\n", "configured category wrote cfg|DEBUG|configured");

  std::string rm = "rm -rf " + dir; if (std::system(rm.c_str())) {}
  QT_END();
}
