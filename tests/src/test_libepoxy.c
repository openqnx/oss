/* libepoxy: the GL function loader used by GTK. Every GL entry point resolves
 * against a live context, which a headless target has none of, so this test
 * exercises the part of the public API that is context-free -- the extension
 * string parser -- and proves the library links and its symbols resolve.
 * It also checks the GLES 2 library name epoxy dlopen()s exists here: it once
 * asked for Linux's libGLESv2.so.2 (QNX: .so.1) and GTK 4 aborted. */
#include "common.h"
#include <epoxy/common.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

/* The "libGLESv2.so.N" string compiled into libepoxy, or NULL. */
static char *gles2_name(void) {
  const char *paths[] = { "/usr/local/lib/libepoxy.so.0", "/data/oss/usr/local/lib/libepoxy.so.0" };
  for (int i = 0; i < 2; i++) {
    FILE *f = fopen(paths[i], "rb");
    if (!f) continue;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *d = malloc(n + 1); size_t got = fread(d, 1, n, f); fclose(f); d[got] = 0;
    for (size_t j = 0; j + 14 < got; j++)
      if (memcmp(d + j, "libGLESv2.so.", 13) == 0 && (j == 0 || d[j - 1] == 0)) {
        char *r = strdup(d + j); free(d); return r;
      }
    free(d);
  }
  return NULL;
}
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
  char *gl = gles2_name();
  printf("  info libepoxy loads %s\n", gl ? gl : "(not found)");
  QT_CHECK(gl && strcmp(gl, "libGLESv2.so.1") == 0, "epoxy asks for QNX's libGLESv2.so.1");
  void *h = gl ? dlopen(gl, RTLD_NOW) : NULL;
  QT_CHECK(h != NULL, "that library loads on this target");
  if (h) dlclose(h);
  free(gl);
  QT_END();
}
