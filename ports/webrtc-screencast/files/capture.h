/* capture.h -- frames of a Wayland output, through wlr-screencopy (the
 * compositor copies the output into a wl_shm buffer of this client). */
#ifndef SCREENCAST_CAPTURE_H
#define SCREENCAST_CAPTURE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* One captured frame: 32 bits per pixel, `fourcc` as libyuv names the byte
 * order in memory ("ARGB" = B,G,R,A bytes; "ABGR" = R,G,B,A bytes). */
struct capture_frame {
  const uint8_t *data;
  int width, height, stride;
  uint32_t fourcc;
  int y_invert;
};
/* Returns 0, or the number of milliseconds to wait before the next capture
 * (nobody is watching: capture rarely). */
typedef int (*capture_cb)(void *user, const struct capture_frame *f);

/* Captures output number `output` (0 = the first) `fps` times a second and
 * calls `cb` for every frame, until *stop becomes non-zero. Returns 0 when
 * stopped, -1 on an error (the reason is printed on stderr). */
int capture_run(const char *display, int output, int fps, int cursor,
                capture_cb cb, void *user, volatile int *stop);

#ifdef __cplusplus
}
#endif
#endif
