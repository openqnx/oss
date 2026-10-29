/* libcurl: feature inventory, URL parsing, and a real loopback HTTP GET
 * served by a thread inside this same process -- no network needed. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <curl/curl.h>
static int listen_fd = -1;
static unsigned short port;
static void *server(void *arg) {
  (void)arg;
  int c = accept(listen_fd, NULL, NULL);
  if (c < 0) return NULL;
  char req[1024];
  (void)!read(c, req, sizeof req);
  static const char resp[] =
      "HTTP/1.1 200 OK\r\nContent-Length: 12\r\nContent-Type: text/plain\r\n"
      "Connection: close\r\n\r\nhello-from-q";
  (void)!write(c, resp, sizeof resp - 1);
  close(c);
  return NULL;
}
static size_t sink(char *p, size_t s, size_t n, void *u) {
  char *b = u; size_t got = s * n;
  if (got > 63) got = 63;
  memcpy(b, p, got); b[got] = 0;
  return s * n;
}
int main(void) {
  QT_BEGIN("curl");
  curl_version_info_data *vi = curl_version_info(CURLVERSION_NOW);
  printf("  info %s\n", curl_version());
  QT_CHECK(vi != NULL, "curl_version_info");
  QT_CHECK(vi && (vi->features & CURL_VERSION_SSL), "built with TLS support");
  QT_CHECK(vi && vi->ssl_version && vi->ssl_version[0], "TLS backend named");

  CURLU *u = curl_url();
  QT_CHECK(curl_url_set(u, CURLUPART_URL, "https://user@example.com:8443/a/b?x=1", 0) == CURLUE_OK,
           "curl_url_set parses a URL");
  char *host = NULL;
  curl_url_get(u, CURLUPART_HOST, &host, 0);
  QT_CHECK(host && strcmp(host, "example.com") == 0, "host extracted");
  curl_free(host);
  curl_url_cleanup(u);

  QT_CHECK(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK, "curl_global_init");
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sa; memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET; sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); sa.sin_port = 0;
  QT_CHECK(bind(listen_fd, (struct sockaddr *)&sa, sizeof sa) == 0, "bind loopback");
  socklen_t sl = sizeof sa;
  getsockname(listen_fd, (struct sockaddr *)&sa, &sl);
  port = ntohs(sa.sin_port);
  listen(listen_fd, 1);
  pthread_t th;
  pthread_create(&th, NULL, server, NULL);

  char url[64], body[64] = {0};
  snprintf(url, sizeof url, "http://127.0.0.1:%u/", port);
  CURL *c = curl_easy_init();
  curl_easy_setopt(c, CURLOPT_URL, url);
  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, sink);
  curl_easy_setopt(c, CURLOPT_WRITEDATA, body);
  curl_easy_setopt(c, CURLOPT_TIMEOUT, 10L);
  CURLcode rc = curl_easy_perform(c);
  QT_CHECK(rc == CURLE_OK, "curl_easy_perform over loopback");
  if (rc != CURLE_OK) printf("  info %s\n", curl_easy_strerror(rc));
  long code = 0;
  curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
  QT_CHECK(code == 200, "HTTP 200 received");
  QT_CHECK(strcmp(body, "hello-from-q") == 0, "response body matches");
  curl_easy_cleanup(c);
  pthread_join(th, NULL);
  close(listen_fd);
  curl_global_cleanup();
  QT_END();
}
