/* input.c -- see input.h. Its own Wayland connection and thread: the events
 * arrive through a pipe, so the caller (WebRTC's network thread) never waits
 * for the compositor. The keyboard is a US layout made by libxkbcommon; the
 * modifier state is tracked here, as the virtual-keyboard protocol asks. */
#include "input.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include "virtual-keyboard-unstable-v1-client-protocol.h"
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

enum { EV_MOTION, EV_BUTTON, EV_WHEEL, EV_KEY, EV_QUIT };
struct ev { int type, a, b; double x, y; };

static struct {
  struct wl_display *dpy;
  struct wl_seat *seat;
  struct zwlr_virtual_pointer_manager_v1 *pmgr;
  struct zwp_virtual_keyboard_manager_v1 *kmgr;
  struct zwlr_virtual_pointer_v1 *ptr;
  struct zwp_virtual_keyboard_v1 *kbd;
  struct xkb_context *xctx;
  struct xkb_keymap *xmap;
  struct xkb_state *xstate;
  int pipe[2];
  pthread_t thread;
  int running;
  unsigned char down[256];      /* keys held, released when the sender goes */
  unsigned char bdown[3];
} in = { .pipe = { -1, -1 } };

static uint32_t now_ms(void) {
  struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
  return (uint32_t)(t.tv_sec * 1000 + t.tv_nsec / 1000000);
}

static void reg_global(void *d, struct wl_registry *r, uint32_t name, const char *iface, uint32_t ver) {
  (void)d;
  if (!strcmp(iface, wl_seat_interface.name) && !in.seat)
    in.seat = wl_registry_bind(r, name, &wl_seat_interface, 1);
  else if (!strcmp(iface, zwlr_virtual_pointer_manager_v1_interface.name))
    in.pmgr = wl_registry_bind(r, name, &zwlr_virtual_pointer_manager_v1_interface, ver < 2 ? ver : 2);
  else if (!strcmp(iface, zwp_virtual_keyboard_manager_v1_interface.name))
    in.kmgr = wl_registry_bind(r, name, &zwp_virtual_keyboard_manager_v1_interface, 1);
}
static void reg_remove(void *d, struct wl_registry *r, uint32_t name) { (void)d; (void)r; (void)name; }
static const struct wl_registry_listener reg_listener = { reg_global, reg_remove };

static int send_keymap(void) {
  in.xctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_rule_names names = { .rules = NULL, .model = "pc105", .layout = "us", .variant = NULL, .options = NULL };
  in.xmap = in.xctx ? xkb_keymap_new_from_names(in.xctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS) : NULL;
  if (!in.xmap) { fprintf(stderr, "screencast: no keyboard layout (xkeyboard-config installed?): keys are not passed on\n"); return -1; }
  in.xstate = xkb_state_new(in.xmap);
  char *s = xkb_keymap_get_as_string(in.xmap, XKB_KEYMAP_FORMAT_TEXT_V1);
  size_t len = strlen(s) + 1;
  char name[64]; snprintf(name, sizeof name, "/screencast-keymap-%d", (int)getpid());
  int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
  if (fd < 0) { free(s); return -1; }
  shm_unlink(name);
  if (ftruncate(fd, (off_t)len) != 0) { close(fd); free(s); return -1; }
  void *p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED) { close(fd); free(s); return -1; }
  memcpy(p, s, len); munmap(p, len); free(s);
  zwp_virtual_keyboard_v1_keymap(in.kbd, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, (uint32_t)len);
  wl_display_roundtrip(in.dpy);     /* the compositor has read it before the file goes */
  close(fd);
  return 0;
}

static void do_key(int code, int down) {
  if (!in.kbd || !in.xstate || code <= 0 || code > 255) return;
  if ((in.down[code] != 0) == (down != 0)) return;   /* repeats are the clients' own business */
  in.down[code] = down ? 1 : 0;
  zwp_virtual_keyboard_v1_key(in.kbd, now_ms(), (uint32_t)code,
                              down ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);
  xkb_state_update_key(in.xstate, (xkb_keycode_t)code + 8, down ? XKB_KEY_DOWN : XKB_KEY_UP);
  zwp_virtual_keyboard_v1_modifiers(in.kbd,
      xkb_state_serialize_mods(in.xstate, XKB_STATE_MODS_DEPRESSED),
      xkb_state_serialize_mods(in.xstate, XKB_STATE_MODS_LATCHED),
      xkb_state_serialize_mods(in.xstate, XKB_STATE_MODS_LOCKED),
      xkb_state_serialize_layout(in.xstate, XKB_STATE_LAYOUT_EFFECTIVE));
}

static void do_button(int b, int down) {
  static const uint32_t code[3] = { 0x110, 0x112, 0x111 };   /* BTN_LEFT, BTN_MIDDLE, BTN_RIGHT */
  if (b < 0 || b > 2 || (in.bdown[b] != 0) == (down != 0)) return;
  in.bdown[b] = down ? 1 : 0;
  zwlr_virtual_pointer_v1_button(in.ptr, now_ms(), code[b],
                                 down ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED);
  zwlr_virtual_pointer_v1_frame(in.ptr);
}

static void *run(void *arg) {
  (void)arg;
  struct pollfd pf[2] = { { in.pipe[0], POLLIN, 0 }, { wl_display_get_fd(in.dpy), POLLIN, 0 } };
  for (;;) {
    wl_display_flush(in.dpy);
    if (poll(pf, 2, -1) < 0) { if (errno == EINTR) continue; break; }
    if (pf[1].revents & (POLLERR | POLLHUP)) break;
    if (pf[1].revents & POLLIN) { if (wl_display_dispatch(in.dpy) < 0) break; }
    if (!(pf[0].revents & POLLIN)) continue;
    struct ev e;
    if (read(in.pipe[0], &e, sizeof e) != (ssize_t)sizeof e) break;
    if (e.type == EV_QUIT) break;
    uint32_t t = now_ms();
    switch (e.type) {
    case EV_MOTION: {
      /* the position as a fraction of an extent of 10000 x 10000: the
         compositor maps it onto the output */
      double x = e.x < 0 ? 0 : e.x > 1 ? 1 : e.x, y = e.y < 0 ? 0 : e.y > 1 ? 1 : e.y;
      zwlr_virtual_pointer_v1_motion_absolute(in.ptr, t, (uint32_t)(x * 10000), (uint32_t)(y * 10000), 10000, 10000);
      zwlr_virtual_pointer_v1_frame(in.ptr);
      break; }
    case EV_BUTTON: do_button(e.a, e.b); break;
    case EV_WHEEL:
      zwlr_virtual_pointer_v1_axis_source(in.ptr, WL_POINTER_AXIS_SOURCE_WHEEL);
      if (e.y != 0) zwlr_virtual_pointer_v1_axis(in.ptr, t, WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(e.y * 15));
      if (e.x != 0) zwlr_virtual_pointer_v1_axis(in.ptr, t, WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(e.x * 15));
      zwlr_virtual_pointer_v1_frame(in.ptr);
      break;
    case EV_KEY:
      if (e.a < 0) {       /* the sender went away: let go of everything */
        for (int k = 1; k < 256; k++) if (in.down[k]) do_key(k, 0);
        for (int b = 0; b < 3; b++) if (in.bdown[b]) do_button(b, 0);
      } else do_key(e.a, e.b);
      break;
    }
  }
  return NULL;
}

int input_start(const char *display) {
  in.dpy = wl_display_connect(display);
  if (!in.dpy) { fprintf(stderr, "screencast: input: cannot connect to the Wayland display\n"); return -1; }
  struct wl_registry *reg = wl_display_get_registry(in.dpy);
  wl_registry_add_listener(reg, &reg_listener, NULL);
  wl_display_roundtrip(in.dpy);
  if (!in.pmgr) {
    fprintf(stderr, "screencast: the compositor has no virtual pointer (zwlr_virtual_pointer_manager_v1): no input\n");
    wl_display_disconnect(in.dpy); in.dpy = NULL;
    return -1;
  }
  in.ptr = zwlr_virtual_pointer_manager_v1_create_virtual_pointer(in.pmgr, in.seat);
  if (in.kmgr && in.seat) {
    in.kbd = zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(in.kmgr, in.seat);
    if (send_keymap() != 0) { zwp_virtual_keyboard_v1_destroy(in.kbd); in.kbd = NULL; }
  } else {
    fprintf(stderr, "screencast: the compositor has no virtual keyboard: the mouse only\n");
  }
  if (pipe(in.pipe) != 0 || pthread_create(&in.thread, NULL, run, NULL) != 0) {
    fprintf(stderr, "screencast: input: cannot start\n");
    return -1;
  }
  in.running = 1;
  return 0;
}

static void push(struct ev e) {
  if (!in.running) return;
  ssize_t w = write(in.pipe[1], &e, sizeof e);   /* smaller than PIPE_BUF: written whole */
  (void)w;
}
void input_motion(double x, double y) { struct ev e = { EV_MOTION, 0, 0, x, y }; push(e); }
void input_button(int button, int down) { struct ev e = { EV_BUTTON, button, down, 0, 0 }; push(e); }
void input_wheel(double dx, double dy) { struct ev e = { EV_WHEEL, 0, 0, dx, dy }; push(e); }
void input_key(int code, int down) { struct ev e = { EV_KEY, code, down, 0, 0 }; push(e); }

void input_stop(void) {
  if (!in.running) return;
  input_key(-1, 0);
  struct ev e = { EV_QUIT, 0, 0, 0, 0 }; push(e);
  pthread_join(in.thread, NULL);
  in.running = 0;
  if (in.dpy) { wl_display_flush(in.dpy); wl_display_disconnect(in.dpy); in.dpy = NULL; }
}
