/* json-c: build an object, serialise, parse back. */
#include "common.h"
#include <json-c/json.h>
int main(void) {
  QT_BEGIN("json-c");
  json_object *o = json_object_new_object();
  json_object_object_add(o, "sdp", json_object_new_int(8));
  json_object_object_add(o, "os", json_object_new_string("qnx"));
  const char *s = json_object_to_json_string_ext(o, JSON_C_TO_STRING_PLAIN);
  QT_CHECK(strcmp(s, "{\"sdp\":8,\"os\":\"qnx\"}") == 0, "serialises");
  json_object *b = json_tokener_parse(s), *v;
  QT_CHECK(b && json_object_object_get_ex(b, "sdp", &v) && json_object_get_int(v) == 8, "round trip");
  QT_CHECK(json_tokener_parse("{bad") == NULL, "rejects malformed input");
  json_object_put(b); json_object_put(o);
  QT_END();
}
