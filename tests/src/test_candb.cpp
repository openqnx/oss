// candb: what its -dev package installs is cxxopts (the command-line parser
// CANdb's tools are built with, in its 2.2 API). Parse a command line with
// typed options, defaults, repeated flags, a repeated vector option and
// positionals, generate the help text, and reject an unknown option.
#include "common.h"
#include <cxxopts.hpp>
#include <string>
#include <vector>

int main() {
  QT_BEGIN("candb");
  printf("  info cxxopts %d.%d.%d\n", CXXOPTS__VERSION_MAJOR, CXXOPTS__VERSION_MINOR, CXXOPTS__VERSION_PATCH);
  try {
    cxxopts::Options opts("candbtool", "reads CAN databases");
    opts.add_options()
      ("n,count", "number of frames", cxxopts::value<int>()->default_value("3"))
      ("b,bitrate", "bus bitrate", cxxopts::value<long>()->default_value("500000"))
      ("name", "bus name", cxxopts::value<std::string>())
      ("id", "frame id", cxxopts::value<std::vector<int>>())
      ("v,verbose", "more output")
      ("files", "input files", cxxopts::value<std::vector<std::string>>());
    opts.parse_positional({"files"});
    const char *argv0[] = { "candbtool", "-n", "7", "--name=can0", "-v", "-v", "--id", "256", "--id", "512", "a.dbc", "b.dbc", nullptr };
    int argc = 12; char **argv = const_cast<char **>(argv0);
    auto r = opts.parse(argc, argv);
    QT_CHECK(r["count"].as<int>() == 7, "-n 7 -> count 7");
    QT_CHECK(r["bitrate"].as<long>() == 500000 && r.count("bitrate") == 0, "bitrate default 500000, not given");
    QT_CHECK(r["name"].as<std::string>() == "can0", "--name=can0");
    QT_CHECK(r.count("verbose") == 2, "-v -v counted twice");
    auto ids = r["id"].as<std::vector<int>>();
    QT_CHECK(ids.size() == 2 && ids[0] == 256 && ids[1] == 512, "--id repeated: 256, 512");
    auto files = r["files"].as<std::vector<std::string>>();
    QT_CHECK(files.size() == 2 && files[0] == "a.dbc" && files[1] == "b.dbc", "positionals a.dbc b.dbc");
    std::string help = opts.help();
    QT_CHECK(help.find("--count") != std::string::npos && help.find("number of frames") != std::string::npos, "help text lists --count");
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "valid command line parsed");
  }
  bool threw = false;
  try {
    cxxopts::Options o("t", "");
    o.add_options()("a", "a flag");
    const char *av0[] = { "t", "--nope", nullptr }; int ac = 2; char **av = const_cast<char **>(av0);
    o.parse(ac, av);
  } catch (const cxxopts::OptionException &e) { threw = true; printf("  info rejected: %s\n", e.what()); }
  QT_CHECK(threw, "unknown option --nope rejected");
  threw = false;
  try {
    cxxopts::Options o("t", "");
    o.add_options()("n", "num", cxxopts::value<int>());
    const char *av0[] = { "t", "-n", "seven", nullptr }; int ac = 3; char **av = const_cast<char **>(av0);
    o.parse(ac, av);
  } catch (const cxxopts::OptionException &) { threw = true; }
  QT_CHECK(threw, "-n seven rejected as not an int");
  QT_END();
}
