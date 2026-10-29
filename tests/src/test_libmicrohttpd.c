/* libmicrohttpd: serve one request on loopback and fetch it with a socket. */
#include "common.h"
#include <microhttpd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
static enum MHD_Result h(void *c, struct MHD_Connection *conn, const char *url, const char *m,
                         const char *v, const char *d, size_t *s, void **p) {
  struct MHD_Response *r = MHD_create_response_from_buffer(5, (void *)"hello", MHD_RESPMEM_PERSISTENT);
  enum MHD_Result ret = MHD_queue_response(conn, 200, r); MHD_destroy_response(r); return ret;
}
int main(void) {
  QT_BEGIN("libmicrohttpd");
  struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_POLL, 18080, NULL, NULL, h, NULL, MHD_OPTION_END);
  QT_CHECK(d != NULL, "daemon starts");
  int fd = socket(AF_INET, SOCK_STREAM, 0); struct sockaddr_in sa = {0};
  sa.sin_family = AF_INET; sa.sin_port = htons(18080); sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  QT_CHECK(connect(fd, (struct sockaddr *)&sa, sizeof sa) == 0, "connects");
  const char *q = "GET / HTTP/1.0\r\n\r\n"; write(fd, q, strlen(q));
  char buf[512] = {0}; size_t n = 0; ssize_t k;
  while ((k = read(fd, buf + n, sizeof buf - 1 - n)) > 0) n += k;
  QT_CHECK(strstr(buf, "200") && strstr(buf, "hello"), "HTTP 200 with body");
  close(fd); MHD_stop_daemon(d);
  QT_END();
}
