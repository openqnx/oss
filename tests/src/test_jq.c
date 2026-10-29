/* libjq: compile a jq program and run it over a JSON input. */
#include "common.h"
#include <stdlib.h>
#include <jv.h>
#include <jq.h>
int main(void) {
  QT_BEGIN("jq");
  jq_state *jq = jq_init();
  QT_CHECK(jq != NULL, "jq_init");
  if (!jq) QT_END();
  QT_CHECK(jq_compile(jq, ".arches | length") != 0, "jq_compile");
  jv input = jv_parse("{\"name\":\"qnx\",\"arches\":[\"aarch64le\",\"x86_64\"]}");
  QT_CHECK(jv_is_valid(input), "jv_parse");
  jq_start(jq, input, 0);
  jv result = jq_next(jq);
  QT_CHECK(jv_is_valid(result), "jq_next produced a result");
  QT_CHECK(jv_get_kind(result) == JV_KIND_NUMBER && jv_number_value(result) == 2,
           "the program returned 2");
  jv_free(result);
  QT_CHECK(!jv_is_valid(jq_next(jq)), "only one result");

  /* String output and the built-in function library. */
  QT_CHECK(jq_compile(jq, "[.arches[] | ascii_upcase] | join(\",\")") != 0,
           "compile a program using built-ins");
  jq_start(jq, jv_parse("{\"arches\":[\"aarch64le\",\"x86_64\"]}"), 0);
  result = jq_next(jq);
  QT_CHECK(jv_get_kind(result) == JV_KIND_STRING, "result is a string");
  QT_CHECK(strcmp(jv_string_value(result), "AARCH64LE,X86_64") == 0,
           "built-ins produced the expected string");
  jv_free(result);
  jv_free(jq_next(jq));

  QT_CHECK(jq_compile(jq, "this is not jq (((") == 0, "a bad program fails to compile");
  jq_teardown(&jq);
  QT_END();
}
