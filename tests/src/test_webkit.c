/* WebKitGTK on QNX: the engine, without a window.
 *
 * X11 and Wayland windowing are off in this port -- QNX's only EGL belongs to
 * Screen and types EGLNativeDisplayType as an integer handle, so WebKit's
 * display backends cannot bind to it.  What that leaves is still the bulk of
 * the engine, and this exercises the parts reachable without a display:
 * JavaScriptCore through WebKit's GLib API, and the WebKit library itself. */
#include "common.h"
#include <string.h>
#include <jsc/jsc.h>
#include <webkit2/webkit2.h>

int main(void) {
  QT_BEGIN("webkitgtk");
  printf("  info WebKitGTK %u.%u.%u\n", webkit_get_major_version(),
         webkit_get_minor_version(), webkit_get_micro_version());
  QT_CHECK(webkit_get_major_version() == 2, "webkit_get_major_version");

  /* --- JavaScriptCore through the GLib API --- */
  JSCContext *ctx = jsc_context_new();
  QT_CHECK(ctx != NULL, "jsc_context_new");
  if (!ctx) QT_END();

  JSCValue *v = jsc_context_evaluate(ctx, "2 + 3 * 4", -1);
  QT_CHECK(v && jsc_value_to_int32(v) == 14, "arithmetic");
  g_clear_object(&v);

  v = jsc_context_evaluate(ctx, "[5,3,9,1].sort((a,b)=>a-b).join('-')", -1);
  char *s = v ? jsc_value_to_string(v) : NULL;
  QT_CHECK(s && strcmp(s, "1-3-5-9") == 0, "array sort with a closure");
  g_free(s); g_clear_object(&v);

  v = jsc_context_evaluate(ctx, "JSON.parse('{\"a\":[1,2,3]}').a[2]", -1);
  QT_CHECK(v && jsc_value_to_int32(v) == 3, "JSON.parse");
  g_clear_object(&v);

  /* Allocation pressure, so the collector runs under the GLib API too. */
  v = jsc_context_evaluate(ctx,
      "(function(){let n=0;for(let i=0;i<200000;i++){const o={i,s:'x'.repeat(8)};n+=o.i%3;}return n;})()", -1);
  QT_CHECK(v && jsc_value_to_int32(v) == 199999, "200k allocations, collected while running");
  g_clear_object(&v);

  /* An exception must surface as a JSCException, not abort the process. */
  v = jsc_context_evaluate(ctx, "throw new Error('boom')", -1);
  JSCException *exc = jsc_context_get_exception(ctx);
  QT_CHECK(exc != NULL, "a thrown Error reaches the embedder");
  if (exc) printf("  info exception: %s\n", jsc_exception_get_message(exc));
  jsc_context_clear_exception(ctx);
  g_clear_object(&v);

  /* A C function called from JavaScript: the GLib<->JS bridge over libffi. */
  jsc_context_evaluate(ctx, "function twice(x){ return x*2 }", -1);
  JSCValue *fn = jsc_context_get_value(ctx, "twice");
  QT_CHECK(fn && jsc_value_is_function(fn), "function object round trip");
  if (fn) {
    JSCValue *r = jsc_value_function_call(fn, G_TYPE_INT, 21, G_TYPE_NONE);
    QT_CHECK(r && jsc_value_to_int32(r) == 42, "calling a JS function from C");
    g_clear_object(&r);
  }
  g_clear_object(&fn);
  g_object_unref(ctx);

  /* WebKit's own types register: this pulls in WebCore's GObject layer, which
     is what proves the 99 MB library actually loaded and initialised. */
  QT_CHECK(webkit_settings_get_type() != 0, "WebKitSettings type registered");
  WebKitSettings *settings = webkit_settings_new();
  QT_CHECK(settings != NULL, "webkit_settings_new");
  if (settings) {
    webkit_settings_set_enable_javascript(settings, TRUE);
    QT_CHECK(webkit_settings_get_enable_javascript(settings), "settings round trip");
    printf("  info default UA: %.60s...\n", webkit_settings_get_user_agent(settings));
    g_object_unref(settings);
  }
  QT_CHECK(webkit_web_context_get_type() != 0, "WebKitWebContext type registered");

  QT_END();
}
