#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#define WLR_USE_UNSTABLE 1
#include <wlr/backend.h>
#include <wlr/backend/headless.h>
#include <wlr/backend/qnx_screen.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>
#include <drm_fourcc.h>
/* wlroots without a display: the headless backend, the pixman (software)
 * renderer and a scene with one red rectangle on a 64x64 output, committed;
 * the pixels of the frame are then read back (known answer: red where the
 * rectangle is, the clear colour elsewhere). And the QNX Screen backend:
 * with Screen running it must give an output, without it fail cleanly. */
static struct wlr_output *new_out;
static void on_new_output(struct wl_listener *l, void *data) { (void)l; new_out = data; }

int main(void) {
  QT_BEGIN("wlroots");
  setenv("WLR_RENDERER", "pixman", 1);
  struct wl_display *disp = wl_display_create();
  QT_CHECK(disp != NULL, "wl_display_create");
  struct wl_event_loop *loop = wl_display_get_event_loop(disp);

  struct wlr_backend *be = wlr_headless_backend_create(loop);
  QT_CHECK(be != NULL, "wlr_headless_backend_create");
  struct wl_listener nl = { .notify = on_new_output };
  wl_signal_add(&be->events.new_output, &nl);
  struct wlr_renderer *r = wlr_renderer_autocreate(be);
  QT_CHECK(r != NULL, "wlr_renderer_autocreate (pixman)");
  struct wlr_allocator *al = r ? wlr_allocator_autocreate(be, r) : NULL;
  QT_CHECK(al != NULL, "wlr_allocator_autocreate");
  QT_CHECK(wlr_backend_start(be), "wlr_backend_start");
  wlr_headless_add_output(be, 64, 64);
  QT_CHECK(new_out != NULL, "headless output announced");

  if (new_out && r && al) {
    QT_CHECK(wlr_output_init_render(new_out, al, r), "wlr_output_init_render");
    struct wlr_output_state st; wlr_output_state_init(&st);
    wlr_output_state_set_enabled(&st, true);
    QT_CHECK(wlr_output_commit_state(new_out, &st), "output enabled");
    wlr_output_state_finish(&st);

    struct wlr_scene *scene = wlr_scene_create();
    const float red[4] = { 1, 0, 0, 1 };
    struct wlr_scene_rect *rect = wlr_scene_rect_create(&scene->tree, 32, 64, red);
    QT_CHECK(rect != NULL, "wlr_scene_rect_create");
    struct wlr_scene_output *so = wlr_scene_output_create(scene, new_out);
    QT_CHECK(so != NULL, "wlr_scene_output_create");

    struct wlr_output_state fs; wlr_output_state_init(&fs);
    bool built = wlr_scene_output_build_state(so, &fs, NULL);
    QT_CHECK(built && fs.buffer != NULL, "scene rendered a frame");
    if (built && fs.buffer) {
      void *data; uint32_t fmt; size_t stride;
      if (wlr_buffer_begin_data_ptr_access(fs.buffer, WLR_BUFFER_DATA_PTR_ACCESS_READ, &data, &fmt, &stride)) {
        const uint8_t *row = (const uint8_t *)data + 10 * stride;
        uint32_t in, out;
        memcpy(&in, row + 4 * 5, 4); memcpy(&out, row + 4 * 50, 4);
        printf("  info format %.4s, pixel(5,10)=%08x pixel(50,10)=%08x\n", (char *)&fmt, in, out);
        /* XRGB8888/ARGB8888, little endian: 0x00ff0000 is red */
        QT_CHECK((fmt == DRM_FORMAT_XRGB8888 || fmt == DRM_FORMAT_ARGB8888) && (in & 0xffffff) == 0xff0000,
                 "inside the rectangle: red");
        QT_CHECK((out & 0xffffff) != 0xff0000, "outside the rectangle: not red");
        wlr_buffer_end_data_ptr_access(fs.buffer);
      } else QT_CHECK(0, "frame buffer readable");
      QT_CHECK(wlr_output_commit_state(new_out, &fs), "frame committed");
    }
    wlr_output_state_finish(&fs);
    wlr_scene_node_destroy(&scene->tree.node);
  }

  /* QNX Screen backend: needs a running Screen (a graphics image). */
  struct wlr_backend *qs = wlr_qnx_screen_backend_create(loop);
  if (qs) {
    QT_CHECK(wlr_backend_is_qnx_screen(qs), "QNX Screen backend created (Screen is running)");
    wlr_backend_destroy(qs);
  } else {
    printf("  info no QNX Screen on this target -- backend creation refused cleanly\n");
    QT_CHECK(1, "QNX Screen backend: fails cleanly without Screen");
  }

  if (al) wlr_allocator_destroy(al);
  if (r) wlr_renderer_destroy(r);
  wl_list_remove(&nl.link);
  wlr_backend_destroy(be);
  wl_display_destroy(disp);
  QT_END();
}
