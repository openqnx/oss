#include "common.h"
#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
int main() {
  QT_BEGIN("boost");
  boost::atomic<int> counter(0);
  boost::thread t([&]{ for (int i = 0; i < 1000; ++i) ++counter; });
  t.join();
  QT_CHECK(counter.load() == 1000, "thread + atomic");
  boost::filesystem::path p("/tmp/qnx-port-test");
  QT_CHECK(p.filename().string() == "qnx-port-test", "filesystem path parsing");
  boost::regex re("svc-([0-9]+)");
  boost::smatch m;
  std::string s = "svc-4660";
  QT_CHECK(boost::regex_match(s, m, re) && m[1] == "4660", "regex capture");
  auto now = boost::posix_time::second_clock::local_time();
  QT_CHECK(!now.is_not_a_date_time(), "date_time clock");
  printf("  info boost %d.%d.%d\n", BOOST_VERSION / 100000,
         BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100);
  QT_END();
}
