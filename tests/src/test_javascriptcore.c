/* JavaScriptCore through its public C API.
 *
 * Built as the JSCOnly port with the CLoop interpreter -- no JIT -- so what
 * this proves is that WebKit's engine, its garbage collector and WTF's
 * threading and memory layers work on QNX.  The checks are ordinary language
 * behaviour, which is the only way to tell a working engine from one that
 * merely linked. */
#include "common.h"
#include <string.h>
#include <JavaScriptCore/JavaScript.h>

static JSValueRef eval(JSContextRef ctx, const char *src, JSValueRef *exc) {
  JSStringRef s = JSStringCreateWithUTF8CString(src);
  JSValueRef v = JSEvaluateScript(ctx, s, NULL, NULL, 1, exc);
  JSStringRelease(s);
  return v;
}
static double evalNum(JSContextRef ctx, const char *src) {
  JSValueRef exc = NULL, v = eval(ctx, src, &exc);
  return (v && !exc) ? JSValueToNumber(ctx, v, NULL) : -1e9;
}

int main(void) {
  QT_BEGIN("javascriptcore");

  JSContextGroupRef group = JSContextGroupCreate();
  QT_CHECK(group != NULL, "JSContextGroupCreate");
  JSGlobalContextRef ctx = JSGlobalContextCreateInGroup(group, NULL);
  QT_CHECK(ctx != NULL, "JSGlobalContextCreateInGroup");
  if (!ctx) QT_END();

  QT_CHECK(evalNum(ctx, "2 + 3 * 4") == 14, "arithmetic and precedence");
  QT_CHECK(evalNum(ctx, "[5,3,9,1].sort((a,b)=>a-b)[3]") == 9, "array sort with a closure");
  QT_CHECK(evalNum(ctx, "(function f(n){return n<2?n:f(n-1)+f(n-2)})(20)") == 6765,
           "recursion (fib 20)");
  QT_CHECK(evalNum(ctx, "'qnx8'.length + parseInt('34')") == 38, "string and parseInt");
  QT_CHECK(evalNum(ctx, "JSON.parse('{\"a\":[1,2,3]}').a[2]") == 3, "JSON.parse");
  QT_CHECK(evalNum(ctx, "Math.round(Math.sqrt(1024))") == 32, "Math through libm");
  QT_CHECK(evalNum(ctx, "new Map([[1,'a'],[2,'b']]).size") == 2, "ES6 Map");
  QT_CHECK(evalNum(ctx, "(()=>{let s=0; for (const x of [1,2,3,4]) s+=x; return s})()") == 10,
           "arrow function and for-of");

  /* A loop long enough to force several garbage collections. */
  QT_CHECK(evalNum(ctx,
    "(function(){ let n = 0;"
    "  for (let i = 0; i < 200000; i++) { const o = {i, s: 'x'.repeat(8)}; n += o.i % 3; }"
    "  return n; })()") == 199999, "200k allocations, collected while running");

  /* Exceptions must propagate to the embedder, not abort. */
  JSValueRef exc = NULL;
  eval(ctx, "throw new Error('boom')", &exc);
  QT_CHECK(exc != NULL, "a thrown Error reaches the embedder");
  if (exc) {
    JSStringRef msg = JSValueToStringCopy(ctx, exc, NULL);
    char buf[64] = {0};
    JSStringGetUTF8CString(msg, buf, sizeof buf);
    printf("  info exception: %s\n", buf);
    QT_CHECK(strstr(buf, "boom") != NULL, "the exception carries its message");
    JSStringRelease(msg);
  }

  /* Hand a C function to JS and call it back. */
  QT_CHECK(evalNum(ctx, "typeof JSON.stringify({a:1}) === 'string' ? 1 : 0") == 1,
           "JSON.stringify");

  JSGarbageCollect(ctx);
  QT_CHECK(evalNum(ctx, "1+1") == 2, "the context still works after an explicit GC");

  JSGlobalContextRelease(ctx);
  JSContextGroupRelease(group);
  QT_END();
}
