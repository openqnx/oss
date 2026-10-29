/* jansson: build a document, serialise it, parse it back. */
#include "common.h"
#include <stdlib.h>
#include <jansson.h>
int main(void) {
  QT_BEGIN("jansson");
  json_t *root = json_object();
  json_object_set_new(root, "name", json_string("qnx"));
  json_object_set_new(root, "sdp", json_integer(8));
  json_t *arr = json_array();
  json_array_append_new(arr, json_string("aarch64le"));
  json_array_append_new(arr, json_string("x86_64"));
  json_object_set_new(root, "arches", arr);
  char *text = json_dumps(root, JSON_COMPACT | JSON_SORT_KEYS);
  QT_CHECK(text != NULL, "json_dumps");
  json_error_t err;
  json_t *back = json_loads(text, 0, &err);
  QT_CHECK(back != NULL, "json_loads round trip");
  QT_CHECK(back && json_integer_value(json_object_get(back, "sdp")) == 8, "integer survives");
  QT_CHECK(back && json_array_size(json_object_get(back, "arches")) == 2, "array survives");
  QT_CHECK(json_loads("{bad", 0, &err) == NULL, "malformed input is rejected");
  json_decref(back); json_decref(root); free(text);
  QT_END();
}
