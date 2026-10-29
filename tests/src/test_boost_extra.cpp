/* The Boost libraries beyond the middleware subset: JSON, URL, Context and the
 * coroutine/fiber stack built on it, Process (whose QNX backend is implemented
 * in this workspace against QNX's own procfs), Locale, Random and Charconv. */
#include "common.h"
#include <string>
#include <vector>
#include <boost/json.hpp>
#include <boost/url.hpp>
#include <boost/context/continuation.hpp>
#include <boost/process/v2/pid.hpp>
#include <boost/process/v2/ext/exe.hpp>
#include <boost/process/v2/ext/cmd.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/charconv.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
namespace ctx = boost::context;
namespace bp  = boost::process::v2;
int main() {
  QT_BEGIN("boost (json/url/context/process/random/charconv)");

  /* --- Boost.JSON --- */
  boost::json::value jv = boost::json::parse(
      R"({"name":"qnx","sdp":8,"arches":["aarch64le","x86_64"]})");
  QT_CHECK(jv.is_object(), "boost::json::parse");
  QT_CHECK(jv.at("sdp").as_int64() == 8, "integer member");
  QT_CHECK(jv.at("arches").as_array().size() == 2, "array member");
  QT_CHECK(!boost::json::serialize(jv).empty(), "boost::json::serialize");

  /* --- Boost.URL --- */
  auto url = boost::urls::parse_uri("https://qnx.example:8443/ports?arch=aarch64le");
  QT_CHECK(url.has_value(), "boost::urls::parse_uri");
  if (url) {
    QT_CHECK(url->host() == "qnx.example", "host");
    QT_CHECK(url->port_number() == 8443, "port");
    QT_CHECK(url->params().find("arch") != url->params().end(), "query parameter");
  }

  /* --- Boost.Context: hand-written per-architecture assembly --- */
  int hops = 0;
  ctx::continuation c = ctx::callcc([&hops](ctx::continuation && caller) {
    ++hops;
    caller = caller.resume();
    ++hops;
    return std::move(caller);
  });
  QT_CHECK(hops == 1, "callcc entered the new context");
  c = c.resume();
  QT_CHECK(hops == 2, "resume returned into the context a second time");

  /* --- Boost.Process: the QNX procfs backend --- */
  bp::pid_type self = ::getpid();
  boost::system::error_code ec;
  auto pids = bp::all_pids(ec);
  QT_CHECK(!ec && !pids.empty(), "all_pids enumerated /proc");
  QT_CHECK(std::find(pids.begin(), pids.end(), self) != pids.end(),
           "our own pid is in the list");
  ec.clear();
  bp::pid_type parent = bp::parent_pid(self, ec);
  QT_CHECK(!ec, "parent_pid via devctl(DCMD_PROC_INFO)");
  QT_CHECK(parent > 0 && parent != self, "parent pid is plausible");
  printf("  info pid %d, parent %d, %zu process(es) visible\n",
         (int)self, (int)parent, pids.size());
  ec.clear();
  auto exe = bp::ext::exe(self, ec);
  QT_CHECK(!ec, "ext::exe read /proc/<pid>/exefile");
  QT_CHECK(!exe.empty() && exe.filename() == "test_boost_extra",
           "exe path names this binary");
  printf("  info exe = %s\n", exe.c_str());

  /* --- Boost.Random --- */
  boost::random::mt19937 rng(42);
  boost::random::uniform_int_distribution<> dist(1, 6);
  int sum = 0;
  for (int i = 0; i < 600; ++i) sum += dist(rng);
  QT_CHECK(sum > 1800 && sum < 2400, "600 dice rolls average about 3.5");

  /* --- Boost.Charconv --- */
  char buf[32];
  auto r = boost::charconv::to_chars(buf, buf + sizeof buf, 3.14159, boost::charconv::chars_format::fixed, 5);
  QT_CHECK(r.ec == std::errc(), "charconv::to_chars");
  double back = 0;
  auto r2 = boost::charconv::from_chars(buf, r.ptr, back);
  QT_CHECK(r2.ec == std::errc() && back > 3.1415 && back < 3.1416,
           "charconv::from_chars round trip");

  /* --- Boost.Container --- */
  boost::container::flat_map<std::string, int> fm;
  fm["aarch64le"] = 1; fm["x86_64"] = 2;
  QT_CHECK(fm.size() == 2 && fm.begin()->first == "aarch64le", "flat_map is sorted");

  /* --- Boost.Algorithm (header-only, but proves the tree is usable) --- */
  std::vector<std::string> parts;
  boost::split(parts, std::string("a,b,c"), boost::is_any_of(","));
  QT_CHECK(parts.size() == 3, "boost::split");
  QT_END();
}
