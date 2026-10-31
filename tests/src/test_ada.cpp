/* ada: WHATWG URL parsing -- normalisation, components, relative resolution,
 * IDNA, search params and the C API, against known answers from the spec. */
#include "common.h"
#include <string>
#include <ada.h>
extern "C" {
#include <ada_c.h>
}

int main() {
  QT_BEGIN("ada");
  printf("  info ada %s\n", ADA_VERSION);

  auto u = ada::parse<ada::url_aggregator>("HTTPS://user:pa%20ss@EXAMPLE.com:443/a/./b/../c?x=1&y=2#frag");
  QT_CHECK(bool(u), "parse an absolute URL");
  if (u) {
    std::string href(u->get_href());
    printf("  info href %s\n", href.c_str());
    QT_CHECK(href == "https://user:pa%20ss@example.com/a/c?x=1&y=2#frag", "href is normalised (scheme, host, default port, dot segments)");
    QT_CHECK(u->get_protocol() == "https:", "protocol");
    QT_CHECK(u->get_hostname() == "example.com", "hostname lower-cased");
    QT_CHECK(u->get_port().empty(), "default port dropped");
    QT_CHECK(u->get_pathname() == "/a/c", "pathname");
    QT_CHECK(u->get_search() == "?x=1&y=2", "search");
    QT_CHECK(u->get_hash() == "#frag", "hash");
    QT_CHECK(u->get_origin() == "https://example.com", "origin");
    u->set_pathname("/new path");
    QT_CHECK(u->get_pathname() == "/new%20path", "set_pathname percent-encodes");
  }

  auto base = ada::parse<ada::url_aggregator>("http://h.test:8080/dir/sub/page.html");
  auto rel = ada::parse<ada::url_aggregator>("../other?q", &*base);
  QT_CHECK(bool(rel) && rel->get_href() == "http://h.test:8080/dir/other?q", "relative URL resolved against a base");

  auto idn = ada::parse<ada::url_aggregator>("https://bücher.example/");
  QT_CHECK(bool(idn) && idn->get_hostname() == "xn--bcher-kva.example", "IDNA: host converted to punycode");
  auto ip = ada::parse<ada::url_aggregator>("http://0x7f.1/");
  QT_CHECK(bool(ip) && ip->get_hostname() == "127.0.0.1", "IPv4 in hex shorthand normalised");
  auto ip6 = ada::parse<ada::url_aggregator>("http://[0:0:0:0:0:0:0:1]:81/");
  QT_CHECK(bool(ip6) && ip6->get_host() == "[::1]:81", "IPv6 compressed");

  QT_CHECK(!ada::can_parse("not a url"), "can_parse rejects garbage");
  QT_CHECK(!ada::parse<ada::url_aggregator>("http://exa mple.com/"), "space in host is an error");
  QT_CHECK(ada::can_parse("file:///etc/hosts"), "file URL accepted");

  ada::url_search_params sp("a=1&b=two+words&a=3");
  QT_CHECK(sp.size() == 3, "search params count");
  QT_CHECK(sp.get("b").has_value() && *sp.get("b") == "two words", "search param decoded");
  QT_CHECK(sp.get_all("a").size() == 2, "get_all");
  sp.append("c", "x&y");
  sp.sort();
  QT_CHECK(sp.to_string() == "a=1&a=3&b=two+words&c=x%26y", "sort + serialise");

  const char *in = "https://www.example.org:8443/p?k=v";
  ada_url cu = ada_parse(in, strlen(in));
  QT_CHECK(ada_is_valid(cu), "C API: ada_parse");
  ada_string port = ada_get_port(cu);
  QT_CHECK(port.length == 4 && memcmp(port.data, "8443", 4) == 0, "C API: port");
  ada_string host = ada_get_hostname(cu);
  QT_CHECK(host.length == 15 && memcmp(host.data, "www.example.org", 15) == 0, "C API: hostname");
  ada_free(cu);
  QT_END();
}
