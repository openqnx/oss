/* libyaml: parse a small document event by event. */
#include "common.h"
#include <yaml.h>
int main(void) {
  QT_BEGIN("libyaml");
  const char *doc = "name: qnx\narches: [aarch64le, x86_64]\n";
  yaml_parser_t p; yaml_event_t ev;
  QT_CHECK(yaml_parser_initialize(&p), "parser init");
  yaml_parser_set_input_string(&p, (const unsigned char *)doc, strlen(doc));
  int scalars = 0, seqs = 0, ok = 1;
  for (;;) {
    if (!yaml_parser_parse(&p, &ev)) { ok = 0; break; }
    if (ev.type == YAML_SCALAR_EVENT) scalars++;
    if (ev.type == YAML_SEQUENCE_START_EVENT) seqs++;
    int end = ev.type == YAML_STREAM_END_EVENT;
    yaml_event_delete(&ev);
    if (end) break;
  }
  QT_CHECK(ok, "document parses");
  QT_CHECK(scalars == 5, "five scalars");
  QT_CHECK(seqs == 1, "one sequence");
  yaml_parser_delete(&p);
  QT_END();
}
