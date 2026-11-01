#include "common.h"
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
/* mesa on Wayland: a Wayland client renders with EGL + OpenGL ES on
 * llvmpipe and presents through wl_shm, the path every Wayland compositor
 * (labwc here) supports. The compositor is a minimal one inside this test
 * (libwayland-server, in a thread: wl_compositor/wl_surface and wl_shm), so
 * the presented pixels can be checked exactly:
 *  - frame 1: blue, with the lower-left 32x32 quarter (GL coordinates)
 *    cleared red through a scissor. In the 64x64 shm buffer (top row first)
 *    the red pixels are exactly rows 32..63, columns 0..31: 1024 of them;
 *  - frame 2: all green, presented after the compositor released frame 1's
 *    buffer (the swap chain is reused, not leaked). */
#define W 64
#define H 64

/* ---------------- the compositor ---------------- */
static struct {
  struct wl_display *d;
  pthread_mutex_t m;
  int frames, w, h, stop;
  uint32_t fmt;
  unsigned char px[W * H * 4];
} S = { .m = PTHREAD_MUTEX_INITIALIZER };

struct surf { struct wl_resource *buffer; struct wl_list frames; };

static void s_destroy(struct wl_client *c, struct wl_resource *r) { (void)c; wl_resource_destroy(r); }
static void s_attach(struct wl_client *c, struct wl_resource *r, struct wl_resource *b, int32_t x, int32_t y) {
  (void)c; (void)x; (void)y; ((struct surf *)wl_resource_get_user_data(r))->buffer = b;
}
static void s_damage(struct wl_client *c, struct wl_resource *r, int32_t x, int32_t y, int32_t w, int32_t h) {
  (void)c; (void)r; (void)x; (void)y; (void)w; (void)h;
}
static void cb_unlink(struct wl_resource *cb) { wl_list_remove(wl_resource_get_link(cb)); }
static void s_frame(struct wl_client *c, struct wl_resource *r, uint32_t id) {
  struct wl_resource *cb = wl_resource_create(c, &wl_callback_interface, 1, id);
  wl_resource_set_implementation(cb, NULL, NULL, cb_unlink);
  wl_list_insert(((struct surf *)wl_resource_get_user_data(r))->frames.prev, wl_resource_get_link(cb));
}
static void s_region(struct wl_client *c, struct wl_resource *r, struct wl_resource *g) { (void)c; (void)r; (void)g; }
static void s_commit(struct wl_client *c, struct wl_resource *r) {
  (void)c;
  struct surf *s = wl_resource_get_user_data(r);
  struct wl_shm_buffer *b = s->buffer ? wl_shm_buffer_get(s->buffer) : NULL;
  if (b) {
    pthread_mutex_lock(&S.m);
    S.w = wl_shm_buffer_get_width(b); S.h = wl_shm_buffer_get_height(b);
    S.fmt = wl_shm_buffer_get_format(b);
    int stride = wl_shm_buffer_get_stride(b);
    wl_shm_buffer_begin_access(b);
    const unsigned char *src = wl_shm_buffer_get_data(b);
    for (int y = 0; y < H && y < S.h; y++) memcpy(S.px + y * W * 4, src + y * stride, (size_t)(S.w < W ? S.w : W) * 4);
    wl_shm_buffer_end_access(b);
    S.frames++;
    pthread_mutex_unlock(&S.m);
    wl_buffer_send_release(s->buffer);
    s->buffer = NULL;
  }
  struct wl_resource *cb, *tmp;
  wl_resource_for_each_safe(cb, tmp, &s->frames) { wl_callback_send_done(cb, 0); wl_resource_destroy(cb); }
}
static void s_transform(struct wl_client *c, struct wl_resource *r, int32_t t) { (void)c; (void)r; (void)t; }
static void s_offset(struct wl_client *c, struct wl_resource *r, int32_t x, int32_t y) { (void)c; (void)r; (void)x; (void)y; }
static const struct wl_surface_interface surface_impl = {
  s_destroy, s_attach, s_damage, s_frame, s_region, s_region, s_commit, s_transform, s_transform, s_damage, s_offset,
};
static void surf_free(struct wl_resource *r) {
  struct surf *s = wl_resource_get_user_data(r);
  struct wl_resource *cb, *tmp;
  /* callbacks outliving the surface: detach them from its list */
  wl_resource_for_each_safe(cb, tmp, &s->frames) { wl_list_remove(wl_resource_get_link(cb)); wl_list_init(wl_resource_get_link(cb)); }
  free(s);
}
static void r_add(struct wl_client *c, struct wl_resource *r, int32_t x, int32_t y, int32_t w, int32_t h) {
  (void)c; (void)r; (void)x; (void)y; (void)w; (void)h;
}
static const struct wl_region_interface region_impl = { s_destroy, r_add, r_add };
static void c_create_surface(struct wl_client *c, struct wl_resource *r, uint32_t id) {
  struct surf *s = calloc(1, sizeof *s); wl_list_init(&s->frames);
  struct wl_resource *sr = wl_resource_create(c, &wl_surface_interface, wl_resource_get_version(r), id);
  wl_resource_set_implementation(sr, &surface_impl, s, surf_free);
}
static void c_create_region(struct wl_client *c, struct wl_resource *r, uint32_t id) {
  struct wl_resource *rr = wl_resource_create(c, &wl_region_interface, 1, id);
  (void)r; wl_resource_set_implementation(rr, &region_impl, NULL, NULL);
}
static const struct wl_compositor_interface compositor_impl = { c_create_surface, c_create_region };
static void bind_compositor(struct wl_client *c, void *data, uint32_t version, uint32_t id) {
  (void)data;
  struct wl_resource *r = wl_resource_create(c, &wl_compositor_interface, version < 4 ? version : 4, id);
  wl_resource_set_implementation(r, &compositor_impl, NULL, NULL);
}
static void *server_main(void *arg) {
  (void)arg;
  struct wl_event_loop *loop = wl_display_get_event_loop(S.d);
  while (!__atomic_load_n(&S.stop, __ATOMIC_SEQ_CST)) {
    wl_event_loop_dispatch(loop, 20);
    wl_display_flush_clients(S.d);
  }
  return NULL;
}

/* ---------------- the client ---------------- */
static struct wl_compositor *compositor;
static void reg_global(void *data, struct wl_registry *r, uint32_t name, const char *iface, uint32_t ver) {
  (void)data; (void)ver;
  if (strcmp(iface, "wl_compositor") == 0) compositor = wl_registry_bind(r, name, &wl_compositor_interface, 4);
}
static void reg_remove(void *data, struct wl_registry *r, uint32_t name) { (void)data; (void)r; (void)name; }
static const struct wl_registry_listener reg_listener = { reg_global, reg_remove };

/* BGRA in memory (ARGB8888/XRGB8888, little-endian) */
static int is(const unsigned char *p, int r, int g, int b) { return p[2] == r && p[1] == g && p[0] == b; }

int main(void) {
  QT_BEGIN("mesa_wayland");
  setenv("MESA_SHADER_CACHE_DISABLE", "true", 0);
  char dir[] = "/tmp/test_mesa_wl.XXXXXX";
  QT_CHECK(mkdtemp(dir) != NULL, "runtime directory");
  setenv("XDG_RUNTIME_DIR", dir, 1);
  S.d = wl_display_create();
  const char *sock = S.d ? wl_display_add_socket_auto(S.d) : NULL;
  QT_CHECK(sock != NULL, "compositor socket");
  if (!sock) QT_END();
  QT_CHECK(wl_display_init_shm(S.d) == 0, "compositor offers wl_shm");
  wl_global_create(S.d, &wl_compositor_interface, 4, NULL, bind_compositor);
  pthread_t th; pthread_create(&th, NULL, server_main, NULL);

  struct wl_display *wd = wl_display_connect(sock);
  QT_CHECK(wd != NULL, "client connects");
  if (!wd) QT_END();
  struct wl_registry *reg = wl_display_get_registry(wd);
  wl_registry_add_listener(reg, &reg_listener, NULL);
  wl_display_roundtrip(wd);
  QT_CHECK(compositor != NULL, "client binds wl_compositor");
  struct wl_surface *ws = wl_compositor_create_surface(compositor);
  struct wl_egl_window *ew = wl_egl_window_create(ws, W, H);

  const char *cexts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  QT_CHECK(cexts && strstr(cexts, "EGL_EXT_platform_wayland"), "EGL client extensions list EGL_EXT_platform_wayland");
  PFNEGLGETPLATFORMDISPLAYEXTPROC gpd = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
  EGLDisplay d = gpd ? gpd(EGL_PLATFORM_WAYLAND_EXT, wd, NULL) : EGL_NO_DISPLAY;
  EGLint maj = 0, min = 0;
  QT_CHECK(d != EGL_NO_DISPLAY && eglInitialize(d, &maj, &min), "EGL on the Wayland display initialises");
  if (d == EGL_NO_DISPLAY) QT_END();
  const EGLint ca[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
                        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };
  EGLConfig cfg; EGLint n = 0;
  QT_CHECK(eglBindAPI(EGL_OPENGL_ES_API) && eglChooseConfig(d, ca, &cfg, 1, &n) && n == 1, "window config");
  const EGLint ctxa[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  EGLContext c = eglCreateContext(d, cfg, EGL_NO_CONTEXT, ctxa);
  EGLSurface s = eglCreateWindowSurface(d, cfg, (EGLNativeWindowType)ew, NULL);
  QT_CHECK(c != EGL_NO_CONTEXT && s != EGL_NO_SURFACE && eglMakeCurrent(d, s, s, c), "window surface current");
  if (c == EGL_NO_CONTEXT || s == EGL_NO_SURFACE) QT_END();
  printf("  renderer: %s\n", glGetString(GL_RENDERER));
  QT_CHECK(strstr((const char *)glGetString(GL_RENDERER), "llvmpipe") != NULL, "renderer is llvmpipe");

  glViewport(0, 0, W, H);
  glClearColor(0, 0, 1, 1); glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_SCISSOR_TEST); glScissor(0, 0, W / 2, H / 2);
  glClearColor(1, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);
  QT_CHECK(eglSwapBuffers(d, s), "eglSwapBuffers (frame 1)");
  wl_display_roundtrip(wd);
  pthread_mutex_lock(&S.m);
  QT_CHECK(S.frames == 1, "compositor received frame 1");
  QT_CHECK(S.w == W && S.h == H, "buffer is 64x64");
  QT_CHECK(S.fmt == WL_SHM_FORMAT_ARGB8888 || S.fmt == WL_SHM_FORMAT_XRGB8888, "buffer is (A|X)RGB8888");
  int red = 0, place = 1;
  for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
    const unsigned char *p = S.px + 4 * (y * W + x);
    int want_red = y >= H / 2 && x < W / 2;
    red += is(p, 255, 0, 0);
    if (want_red ? !is(p, 255, 0, 0) : !is(p, 0, 0, 255)) place = 0;
  }
  pthread_mutex_unlock(&S.m);
  printf("  red pixels: %d\n", red);
  QT_CHECK(red == 1024, "1024 red pixels");
  QT_CHECK(place, "red is the bottom-left quarter (rows 32..63, columns 0..31), the rest blue");

  glClearColor(0, 1, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
  QT_CHECK(eglSwapBuffers(d, s), "eglSwapBuffers (frame 2)");
  wl_display_roundtrip(wd);
  pthread_mutex_lock(&S.m);
  int green = 1;
  for (int i = 0; i < W * H; i++) if (!is(S.px + 4 * i, 0, 255, 0)) { green = 0; break; }
  QT_CHECK(S.frames == 2, "compositor received frame 2");
  pthread_mutex_unlock(&S.m);
  QT_CHECK(green, "frame 2 is all green");

  eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroySurface(d, s); eglDestroyContext(d, c); eglTerminate(d);
  wl_egl_window_destroy(ew); wl_surface_destroy(ws);
  wl_display_disconnect(wd);
  __atomic_store_n(&S.stop, 1, __ATOMIC_SEQ_CST);
  pthread_join(th, NULL);
  wl_display_destroy(S.d);
  rmdir(dir);
  QT_END();
}
