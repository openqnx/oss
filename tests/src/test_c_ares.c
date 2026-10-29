/* c-ares: channel lifecycle plus the pure-parsing entry points, which need no
 * name server to be reachable. */
#include "common.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ares.h>
int main(void) {
  QT_BEGIN("c-ares");
  printf("  info c-ares %s\n", ares_version(NULL));
  QT_CHECK(ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS, "ares_library_init");
  ares_channel channel;
  struct ares_options opts;
  memset(&opts, 0, sizeof opts);
  opts.timeout = 1000; opts.tries = 1;
  int rc = ares_init_options(&channel, &opts, ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES);
  QT_CHECK(rc == ARES_SUCCESS, "ares_init_options");
  /* A hand-built A-record response, parsed without any I/O. */
  static const unsigned char reply[] = {
    0x12,0x34, 0x81,0x80, 0x00,0x01, 0x00,0x01, 0x00,0x00, 0x00,0x00,
    0x03,'q','n','x', 0x04,'t','e','s','t', 0x00, 0x00,0x01, 0x00,0x01,
    0xc0,0x0c, 0x00,0x01, 0x00,0x01, 0x00,0x00,0x00,0x3c, 0x00,0x04,
    0x0a,0x00,0x00,0x01
  };
  struct hostent *he = NULL;
  rc = ares_parse_a_reply(reply, (int)sizeof reply, &he, NULL, NULL);
  QT_CHECK(rc == ARES_SUCCESS, "ares_parse_a_reply");
  QT_CHECK(he && strcmp(he->h_name, "qnx.test") == 0, "question name parsed");
  QT_CHECK(he && he->h_addr_list[0] &&
           ((unsigned char *)he->h_addr_list[0])[0] == 10, "A record is 10.0.0.1");
  if (he) ares_free_hostent(he);
  QT_CHECK(strcmp(ares_strerror(ARES_ENOTFOUND), "") != 0, "ares_strerror");
  ares_destroy(channel);
  ares_library_cleanup();
  QT_END();
}
