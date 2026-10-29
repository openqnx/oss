/* libepoxy: the GL function loader used by GTK. Every GL entry point resolves
 * against a live context, which a headless target has none of, so this test
 * exercises the part of the public API that is context-free -- the extension
 * string parser -- and proves the library links and its symbols resolve. */
#include "common.h"
#include <epoxy/common.h>
int main(void) {
  QT_BEGIN("libepoxy");
  const char *exts = "GL_ARB_shader_objects GL_EXT_texture_sRGB GL_ARB_sync";
  QT_CHECK(epoxy_extension_in_string(exts, "GL_ARB_sync"), "finds a trailing extension");
  QT_CHECK(epoxy_extension_in_string(exts, "GL_EXT_texture_sRGB"), "finds a middle extension");
  QT_CHECK(epoxy_extension_in_string(exts, "GL_ARB_shader_objects"), "finds a leading extension");
  /* The classic parser bug this API exists to avoid: a prefix must not match. */
  QT_CHECK(!epoxy_extension_in_string(exts, "GL_ARB_syn"), "rejects a prefix of a present name");
  QT_CHECK(!epoxy_extension_in_string(exts, "GL_KHR_no_error"), "rejects an absent extension");
  QT_CHECK(!epoxy_extension_in_string("", "GL_ARB_sync"), "rejects against an empty list");
  QT_END();
}
