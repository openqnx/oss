/* libconfig: parse a configuration from a string and read typed settings. */
#include "common.h"
#include <libconfig.h>
int main(void) {
  QT_BEGIN("libconfig");
  static const char text[] =
      "target: { name = \"qnx\"; sdp = 8; arches = [\"aarch64le\", \"x86_64\"]; };";
  config_t cfg;
  config_init(&cfg);
  QT_CHECK(config_read_string(&cfg, text) == CONFIG_TRUE, "config_read_string");
  const char *name = NULL;
  QT_CHECK(config_lookup_string(&cfg, "target.name", &name) == CONFIG_TRUE &&
           strcmp(name, "qnx") == 0, "string lookup");
  int sdp = 0;
  QT_CHECK(config_lookup_int(&cfg, "target.sdp", &sdp) == CONFIG_TRUE && sdp == 8,
           "integer lookup");
  config_setting_t *arches = config_lookup(&cfg, "target.arches");
  QT_CHECK(arches && config_setting_length(arches) == 2, "array lookup");
  config_destroy(&cfg);
  config_init(&cfg);
  QT_CHECK(config_read_string(&cfg, "broken = ;") == CONFIG_FALSE, "syntax error is reported");
  config_destroy(&cfg);
  QT_END();
}
