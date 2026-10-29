/* Duktape: an embeddable ECMAScript engine. */
#include "common.h"
#include <stdlib.h>
#include <duktape.h>
static duk_ret_t native_double(duk_context *ctx) {
  duk_push_int(ctx, 2 * duk_require_int(ctx, 0));
  return 1;
}
int main(void) {
  QT_BEGIN("duktape");
  printf("  info Duktape %ld\n", (long)DUK_VERSION);
  duk_context *ctx = duk_create_heap_default();
  QT_CHECK(ctx != NULL, "duk_create_heap_default");
  if (!ctx) QT_END();
  duk_push_c_function(ctx, native_double, 1);
  duk_put_global_string(ctx, "dbl");
  QT_CHECK(duk_peval_string(ctx, "dbl(21) + '/' + Math.floor(Math.PI * 100)") == 0,
           "evaluate a script that calls into C");
  const char *r = duk_get_string(ctx, -1);
  QT_CHECK(r && strcmp(r, "42/314") == 0, "script result is correct");
  printf("  info result = %s\n", r ? r : "(null)");
  duk_pop(ctx);
  QT_CHECK(duk_peval_string(ctx, "JSON.stringify({a:[1,2,3],b:'qnx'})") == 0,
           "JSON.stringify");
  QT_CHECK(strcmp(duk_get_string(ctx, -1), "{\"a\":[1,2,3],\"b\":\"qnx\"}") == 0,
           "JSON output matches");
  duk_pop(ctx);
  QT_CHECK(duk_peval_string(ctx, "this is not javascript (((") != 0,
           "a syntax error is reported");
  duk_pop(ctx);
  duk_destroy_heap(ctx);
  QT_END();
}
