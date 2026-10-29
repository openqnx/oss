/* inih: parse an INI document through the handler callback. */
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
struct cfg { char name[32]; int sdp; int seen; };
static int handler(void *user, const char *section, const char *name, const char *value) {
  struct cfg *c = user;
  ++c->seen;
  if (strcmp(section, "target") == 0) {
    if (strcmp(name, "name") == 0) snprintf(c->name, sizeof c->name, "%s", value);
    else if (strcmp(name, "sdp") == 0) c->sdp = atoi(value);
  }
  return 1;
}
int main(void) {
  QT_BEGIN("inih");
  char path[] = "/tmp/qnxport-ini-XXXXXX";
  int fd = mkstemp(path);
  QT_CHECK(fd >= 0, "create temp file");
  static const char text[] = "[target]\nname = qnx\nsdp = 8\n\n[other]\nkey = v\n";
  QT_CHECK(write(fd, text, sizeof text - 1) == (ssize_t)(sizeof text - 1), "write ini");
  close(fd);
  struct cfg c = {{0}, 0, 0};
  QT_CHECK(ini_parse(path, handler, &c) == 0, "ini_parse");
  QT_CHECK(strcmp(c.name, "qnx") == 0, "string value");
  QT_CHECK(c.sdp == 8, "integer value");
  QT_CHECK(c.seen == 3, "all three keys visited");
  unlink(path);
  QT_END();
}
