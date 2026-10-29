/* json-glib: build a node tree, serialise it, parse it back. */
#include "common.h"
#include <json-glib/json-glib.h>
int main(void) {
  QT_BEGIN("json-glib");
  JsonBuilder *b = json_builder_new();
  json_builder_begin_object(b);
  json_builder_set_member_name(b, "name");
  json_builder_add_string_value(b, "qnx");
  json_builder_set_member_name(b, "sdp");
  json_builder_add_int_value(b, 8);
  json_builder_set_member_name(b, "arches");
  json_builder_begin_array(b);
  json_builder_add_string_value(b, "aarch64le");
  json_builder_add_string_value(b, "x86_64");
  json_builder_end_array(b);
  json_builder_end_object(b);
  JsonNode *root = json_builder_get_root(b);
  QT_CHECK(root != NULL, "json_builder_get_root");
  JsonGenerator *g = json_generator_new();
  json_generator_set_root(g, root);
  gchar *text = json_generator_to_data(g, NULL);
  QT_CHECK(text != NULL, "json_generator_to_data");

  JsonParser *p = json_parser_new();
  GError *err = NULL;
  QT_CHECK(json_parser_load_from_data(p, text, -1, &err), "json_parser_load_from_data");
  JsonObject *o = json_node_get_object(json_parser_get_root(p));
  QT_CHECK(o && json_object_get_int_member(o, "sdp") == 8, "integer survives");
  QT_CHECK(o && json_array_get_length(json_object_get_array_member(o, "arches")) == 2,
           "array survives");
  QT_CHECK(!json_parser_load_from_data(p, "{nope", -1, NULL),
           "malformed input is rejected");
  g_free(text);
  g_object_unref(p); g_object_unref(g); g_object_unref(b);
  json_node_free(root);
  QT_END();
}
