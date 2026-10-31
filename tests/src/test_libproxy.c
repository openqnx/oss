/* libproxy: resolve proxies for URLs from a known configuration. The
 * environment configuration plugin is forced (PX_FORCE_CONFIG=config-env), so
 * the answers depend only on http_proxy / https_proxy / no_proxy:
 *  - nothing set -> direct://; http_proxy -> that proxy; https URLs prefer
 *    https_proxy; no_proxy host and domain entries -> direct://; a SOCKS URL
 *    is passed through;
 *  - the proxy program prints the same answers, one line per URL. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <proxy.h>

static pxProxyFactory *pf;
static int expect(const char *url, const char *want) {
  char **p = px_proxy_factory_get_proxies(pf, url);
  const char *got = p && p[0] ? p[0] : "(none)";
  int ok = p && p[0] && !strcmp(p[0], want) && !p[1];
  printf("  info %s -> %s%s\n", url, got, p && p[0] && p[1] ? " (+more)" : "");
  px_proxy_factory_free_proxies(p);
  return ok;
}

int main(void) {
  QT_BEGIN("libproxy");
  setenv("PX_FORCE_CONFIG", "config-env", 1);
  unsetenv("http_proxy"); unsetenv("HTTP_PROXY"); unsetenv("https_proxy"); unsetenv("HTTPS_PROXY");
  unsetenv("ftp_proxy"); unsetenv("FTP_PROXY"); unsetenv("no_proxy"); unsetenv("NO_PROXY");
  pf = px_proxy_factory_new();
  QT_CHECK(pf != NULL, "px_proxy_factory_new");
  if (!pf) QT_END();
  QT_CHECK(expect("http://www.qnx.com/", "direct://"), "no configuration -> direct://");

  setenv("http_proxy", "http://proxy.example:3128", 1);
  QT_CHECK(expect("http://www.qnx.com/index.html", "http://proxy.example:3128"), "http_proxy is used for http://");
  QT_CHECK(expect("https://www.qnx.com/", "http://proxy.example:3128"), "https:// falls back to http_proxy");
  setenv("https_proxy", "http://secure.example:8443", 1);
  QT_CHECK(expect("https://www.qnx.com/", "http://secure.example:8443"), "https_proxy is preferred for https://");
  QT_CHECK(expect("http://www.qnx.com/", "http://proxy.example:3128"), "http:// still uses http_proxy");

  setenv("no_proxy", "localhost,.internal.example,10.1.2.3", 1);
  QT_CHECK(expect("http://build.internal.example/x", "direct://"), "no_proxy domain .internal.example -> direct://");
  QT_CHECK(expect("http://localhost:8080/", "direct://"), "no_proxy localhost -> direct://");
  QT_CHECK(expect("http://10.1.2.3/", "direct://"), "no_proxy address -> direct://");
  QT_CHECK(expect("http://external.example/", "http://proxy.example:3128"), "hosts not in no_proxy still proxied");

  unsetenv("no_proxy"); unsetenv("https_proxy");
  setenv("http_proxy", "socks5://socks.example:1080", 1);
  QT_CHECK(expect("http://www.qnx.com/", "socks5://socks.example:1080"), "SOCKS proxy URL passed through");
  QT_CHECK(expect("not a url", "direct://"), "unparsable URL -> direct://");
  px_proxy_factory_free(pf);

  /* ---- the proxy program ---- */
  setenv("http_proxy", "http://proxy.example:3128", 1);
  setenv("no_proxy", ".internal.example", 1);
  FILE *p = popen("proxy http://www.qnx.com/ http://a.internal.example/ 2>&1", "r");
  char out[1024] = {0};
  if (p) { size_t k = fread(out, 1, sizeof out - 1, p); out[k] = 0; }
  int st = p ? pclose(p) : -1;
  printf("  info $ proxy http://www.qnx.com/ http://a.internal.example/ -> %d\n  info %s", st, out);
  QT_CHECK(st == 0 && !strcmp(out, "http://proxy.example:3128\ndirect://\n"), "proxy program: one answer per URL");
  QT_END();
}
