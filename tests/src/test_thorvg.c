/* ThorVG: a vector graphics engine. Rasterises a real shape into a buffer.
 * The 1.x C API (handles are typedef'd pointers; the engine takes a thread
 * count, the canvas an option; shapes are added, not pushed). */
#include "common.h"
#include <stdlib.h>
#include <thorvg_capi.h>
#define W 64
#define H 64
int main(void) {
  QT_BEGIN("thorvg");
  QT_CHECK(tvg_engine_init(1) == TVG_RESULT_SUCCESS, "tvg_engine_init");
  uint32_t *buf = calloc(W * H, sizeof(uint32_t));
  QT_CHECK(buf != NULL, "allocate the canvas buffer");
  Tvg_Canvas canvas = tvg_swcanvas_create(TVG_ENGINE_OPTION_DEFAULT);
  QT_CHECK(canvas != NULL, "tvg_swcanvas_create");
  QT_CHECK(tvg_swcanvas_set_target(canvas, buf, W, W, H, TVG_COLORSPACE_ARGB8888)
           == TVG_RESULT_SUCCESS, "set the render target");
  Tvg_Paint rect = tvg_shape_new();
  QT_CHECK(rect != NULL, "tvg_shape_new");
  tvg_shape_append_rect(rect, 8, 8, 48, 48, 4, 4, true);
  QT_CHECK(tvg_shape_set_fill_color(rect, 255, 0, 0, 255) == TVG_RESULT_SUCCESS,
           "set the fill colour");
  QT_CHECK(tvg_canvas_add(canvas, rect) == TVG_RESULT_SUCCESS, "add the shape");
  QT_CHECK(tvg_canvas_draw(canvas, true) == TVG_RESULT_SUCCESS, "tvg_canvas_draw");
  QT_CHECK(tvg_canvas_sync(canvas) == TVG_RESULT_SUCCESS, "tvg_canvas_sync");
  /* The centre must be red; a corner must still be clear. */
  QT_CHECK((buf[(H / 2) * W + (W / 2)] & 0x00FF0000u) != 0, "centre pixel was filled red");
  QT_CHECK(buf[0] == 0, "corner outside the shape is untouched");
  tvg_canvas_destroy(canvas);
  tvg_engine_term();
  free(buf);
  QT_END();
}
