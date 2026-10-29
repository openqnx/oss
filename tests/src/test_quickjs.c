/* QuickJS-ng: evaluate JavaScript. */
#include "common.h"
#include <quickjs.h>
int main(void) {
  QT_BEGIN("quickjs-ng");
  JSRuntime *rt = JS_NewRuntime(); JSContext *ctx = JS_NewContext(rt);
  /* The length is strlen()'s: a hand-counted one that ran past the string had
   * the parser read the terminator and a byte of whatever followed. */
  const char *code = "[1,2,3].map(x => x * 2).reduce((a, b) => a + b)";
  JSValue v = JS_Eval(ctx, code, strlen(code), "<t>", JS_EVAL_TYPE_GLOBAL);
  int32_t n = 0; JS_ToInt32(ctx, &n, v);
  QT_CHECK(n == 12, "arrow functions, map/reduce");
  JS_FreeValue(ctx, v);
  code = "JSON.stringify({os:'qnx'})";
  v = JS_Eval(ctx, code, strlen(code), "<t>", JS_EVAL_TYPE_GLOBAL);
  const char *s = JS_ToCString(ctx, v);
  QT_CHECK(s && !strcmp(s, "{\"os\":\"qnx\"}"), "JSON");
  JS_FreeCString(ctx, s); JS_FreeValue(ctx, v);
  JS_FreeContext(ctx); JS_FreeRuntime(rt);
  QT_END();
}
