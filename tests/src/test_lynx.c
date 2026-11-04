/* lynx: -dump of a local HTML file and of the same page over HTTP from a
 * server forked here on 127.0.0.1. Checks: the heading, paragraph text
 * wrapped at -width=60, a bulleted list, an entity (&eacute; -> é with
 * -display_charset=utf-8), a link numbered [1] with its URL in the
 * References list, -nolist dropping that list, a table's cells on one
 * row, -source returning the HTML unchanged, the HTTP fetch (the server
 * saw lynx's GET and User-Agent, the page rendered), and -version naming
 * the TLS library it was built with. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const char *HTML =
  "<html><head><title>QNX test page</title></head><body>\n"
  "<h1>Lynx on QNX</h1>\n"
  "<p>This paragraph is long enough that a sixty column dump has to wrap it onto more than one line of output text.</p>\n"
  "<ul><li>first item</li><li>second item</li></ul>\n"
  "<p>Caf&eacute; &amp; more</p>\n"
  "<p>See <a href=\"http://example.org/docs\">the docs</a>.</p>\n"
  "<table><tr><td>alpha</td><td>beta</td></tr></table>\n"
  "</body></html>\n";
static char dir[64], out[65536];
static pid_t srv = -1;
static void stop(void) { if (srv > 0) { kill(srv, SIGKILL); waitpid(srv, NULL, 0); srv = -1; } }
static void on_alarm(int s) { (void)s; if (srv > 0) kill(srv, SIGKILL); static const char m[] = "FAIL lynx (watchdog)\n"; (void)!write(1, m, sizeof m - 1); _exit(2); }
static const char *run(const char *args) {
  char cmd[1024]; snprintf(cmd, sizeof cmd, "cd %s && HOME=%s TERM=vt100 lynx %s 2>&1", dir, dir, args);
  FILE *p = popen(cmd, "r"); size_t n = 0; out[0] = 0;
  if (p) { n = fread(out, 1, sizeof out - 1, p); pclose(p); }
  out[n] = 0; return out;
}
int main(void) {
  QT_BEGIN("lynx");
  signal(SIGALRM, on_alarm); alarm(120); atexit(stop); signal(SIGPIPE, SIG_IGN);
  snprintf(dir, sizeof dir, "/tmp/qnxport-lynx-%d", (int)getpid());
  mkdir(dir, 0700);
  char path[128]; snprintf(path, sizeof path, "%s/page.html", dir);
  FILE *f = fopen(path, "w"); if (f) { fputs(HTML, f); fclose(f); }

  const char *r = run("-version");
  char *nl = strchr(out, '\n'); if (nl) *nl = 0;
  printf("  info %s\n", r);
  QT_CHECK(strstr(r, "Lynx Version 2.9.2") != NULL, "lynx -version: 2.9.2");
  r = run("-version");
  QT_CHECK(strstr(r, "OpenSSL") != NULL, "-version names OpenSSL");

  r = run("-dump -width=60 -display_charset=utf-8 page.html");
  QT_CHECK(strstr(r, "Lynx on QNX") != NULL, "-dump: the <h1> heading");
  int wrapped = 1; for (char *l = out; l && *l; ) { char *e = strchr(l, '\n'); size_t len = e ? (size_t)(e - l) : strlen(l); if (len > 60) wrapped = 0; l = e ? e + 1 : NULL; }
  QT_CHECK(strstr(r, "This paragraph") && strstr(r, "output text") && wrapped, "paragraph wrapped within 60 columns");
  QT_CHECK(strstr(r, "* first item") && strstr(r, "* second item"), "list items as \"* item\"");
  QT_CHECK(strstr(r, "Caf\xc3\xa9 & more") != NULL, "entities: &eacute; -> UTF-8 \\u00e9, &amp; -> &");
  QT_CHECK(strstr(r, "[1]the docs") && strstr(r, "References") && strstr(r, "1. http://example.org/docs"), "link [1] and the References list");
  QT_CHECK(strstr(r, "alpha") && strstr(r, "beta"), "table cells rendered");
  r = run("-dump -nolist -width=60 page.html");
  QT_CHECK(!strstr(r, "References") && strstr(r, "the docs"), "-nolist: no References section");
  r = run("-source page.html");
  QT_CHECK(!strcmp(r, HTML), "-source: the file byte for byte");

  /* the same page over HTTP */
  int ls = socket(AF_INET, SOCK_STREAM, 0), one = 1;
  setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  struct sockaddr_in a; memset(&a, 0, sizeof a); a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  socklen_t al = sizeof a;
  int lok = bind(ls, (struct sockaddr *)&a, sizeof a) == 0 && listen(ls, 4) == 0 && getsockname(ls, (struct sockaddr *)&a, &al) == 0;
  char reqlog[128]; snprintf(reqlog, sizeof reqlog, "%s/request.txt", dir);
  srv = lok ? fork() : -1;
  if (srv == 0) {
    int c = accept(ls, NULL, NULL); char req[4096]; int n = 0, k;
    while (c >= 0 && n < (int)sizeof req - 1 && (k = (int)read(c, req + n, sizeof req - 1 - n)) > 0) { n += k; req[n] = 0; if (strstr(req, "\r\n\r\n")) break; }
    FILE *l = fopen(reqlog, "w"); if (l) { fwrite(req, 1, n, l); fclose(l); }
    char hdr[256]; int hl = snprintf(hdr, sizeof hdr, "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %d\r\n\r\n", (int)strlen(HTML));
    (void)!write(c, hdr, hl); (void)!write(c, HTML, strlen(HTML)); close(c); _exit(0);
  }
  close(ls);
  char args[256]; snprintf(args, sizeof args, "-dump -width=60 http://127.0.0.1:%d/page.html", ntohs(a.sin_port));
  r = run(args);
  QT_CHECK(lok && strstr(r, "Lynx on QNX") && strstr(r, "[1]the docs"), "-dump http://127.0.0.1:<port>/page.html renders the served page");
  stop();
  f = fopen(reqlog, "r"); char req[4096] = ""; size_t rn = f ? fread(req, 1, sizeof req - 1, f) : 0; req[rn] = 0; if (f) fclose(f);
  QT_CHECK(!strncmp(req, "GET /page.html HTTP/1.", 22) && strstr(req, "User-Agent: Lynx/2.9.2"), "the server saw GET /page.html with User-Agent Lynx/2.9.2");
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
