/* cxxopts (header-only): parse argument vectors with known results --
 * short and long options, typed values (int, double, string, vector),
 * defaults and implicit values, grouped short flags (-vvv counted),
 * positional arguments, "--" ending option parsing, errors for unknown options and bad values, and the generated
 * --help text. */
#include "common.h"
#include <cxxopts.hpp>
#include <string>
#include <vector>

static cxxopts::Options make() {
  cxxopts::Options o("tool", "a test program");
  o.add_options()
    ("v,verbose", "verbosity")
    ("n,count", "how many", cxxopts::value<int>()->default_value("1"))
    ("r,ratio", "a ratio", cxxopts::value<double>())
    ("o,output", "output file", cxxopts::value<std::string>()->default_value("a.out"))
    ("I,include", "include dirs", cxxopts::value<std::vector<std::string>>())
    ("color", "colour mode", cxxopts::value<std::string>()->implicit_value("auto"))
    ("files", "inputs", cxxopts::value<std::vector<std::string>>())
    ("h,help", "print help");
  o.parse_positional({"files"});
  return o;
}

int main() {
  QT_BEGIN("cxxopts");
  printf("  info cxxopts %d.%d.%d\n", CXXOPTS__VERSION_MAJOR, CXXOPTS__VERSION_MINOR, CXXOPTS__VERSION_PATCH);
  {
    auto o = make();
    const char *argv[] = {"tool", "-vvv", "--count=7", "-r", "2.5", "-I", "a", "-Ib", "x.c", "--color", "y.c", "--", "-n", nullptr};
    auto r = o.parse(13, argv);
    QT_CHECK(r.count("verbose") == 3, "-vvv counted 3 times");
    QT_CHECK(r["count"].as<int>() == 7 && r["ratio"].as<double>() == 2.5, "--count=7, -r 2.5");
    QT_CHECK(r["output"].as<std::string>() == "a.out" && r.count("output") == 0, "default a.out, not given");
    QT_CHECK(r["include"].as<std::vector<std::string>>() == std::vector<std::string>({"a", "b"}), "-I a -Ib -> [a, b]");
    QT_CHECK(r["color"].as<std::string>() == "auto", "--color without value takes the implicit auto");
    /* after "--" nothing is an option: -n is the third positional file */
    QT_CHECK(r["files"].as<std::vector<std::string>>() == std::vector<std::string>({"x.c", "y.c", "-n"}),
             "positional files x.c y.c, and -n after --");
    QT_CHECK(r["count"].as<int>() == 7 && r.unmatched().empty(), "-n after -- did not change --count");
  }
  {
    auto o = make();
    const char *argv[] = {"tool", "--nope", nullptr};
    bool threw = false;
    try { o.parse(2, argv); } catch (const cxxopts::exceptions::no_such_option &) { threw = true; }
    QT_CHECK(threw, "unknown --nope throws no_such_option");
  }
  {
    auto o = make();
    const char *argv[] = {"tool", "-n", "seven", nullptr};
    bool threw = false;
    try { auto r = o.parse(3, argv); (void)r["count"].as<int>(); } catch (const cxxopts::exceptions::incorrect_argument_type &) { threw = true; }
    QT_CHECK(threw, "-n seven throws incorrect_argument_type");
  }
  {
    auto o = make();
    std::string h = o.help();
    QT_CHECK(h.find("a test program") != std::string::npos && h.find("Usage:") != std::string::npos
             && h.find("-n, --count arg") != std::string::npos && h.find("(default: 1)") != std::string::npos,
             "help text: usage, description, -n, --count arg (default: 1)");
  }
  QT_END();
}
