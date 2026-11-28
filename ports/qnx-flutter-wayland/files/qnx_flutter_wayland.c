// qnx-flutter-wayland: a small Flutter embedder for QNX Neutrino on Wayland
// (labwc): the engine's software renderer draws into wl_shm buffers of an
// xdg-shell toplevel; wl_pointer events become Flutter pointer events.
//
//   qnx-flutter-wayland <bundle dir>
//     <bundle dir>/flutter_assets/   (flutter build bundle)
//     <bundle dir>/libapp.so         (gen_snapshot --snapshot_kind=app-aot-elf)
//     <bundle dir>/icudtl.dat        (optional: else the engine's, ICU_DATA_DEFAULT)
//
// The platform task runner is this thread's loop: poll(2) on the Wayland
// display fd and a wake-up pipe, with the next task's time as the timeout.
#define _QNX_SOURCE 1
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>

#include "flutter_embedder.h"
#include "xdg-shell-client-protocol.h"

#ifndef ICU_DATA_DEFAULT
#define ICU_DATA_DEFAULT "/usr/local/share/flutter/icudtl.dat"
#endif

#define MAX_TASKS 1024
// three buffers: the compositor can hold one on screen and one queued while
// the engine draws into the third (with two, frames were dropped while both
// were still held, which capped animation at a third of the output rate)
#define NBUF 3

struct buf {
  struct wl_buffer* wl;
  void* data;
  int w, h, stride;
  size_t size;
  bool busy;
};

static struct {
  struct wl_display* display;
  struct wl_compositor* compositor;
  struct wl_shm* shm;
  struct xdg_wm_base* wm_base;
  struct wl_seat* seat;
  struct wl_pointer* pointer;
  struct wl_surface* surface;
  struct xdg_surface* xdg_surface;
  struct xdg_toplevel* toplevel;
  int width, height;          // current window size
  bool configured, closed;

  pthread_mutex_t buf_mu;     // buffers are swapped here and drawn on the raster thread
  struct buf bufs[NBUF];

  FlutterEngine engine;
  pthread_t platform_thread;
  pthread_mutex_t task_mu;
  struct { FlutterTask task; uint64_t target; } tasks[MAX_TASKS];
  int ntasks;
  int wake[2];

  double px, py;              // pointer position
  int64_t buttons;
  bool pointer_added;
  int fps_every;              // QNX_FLUTTER_FPS: report interval in seconds, 0 = off
  uint64_t fps_t0;
  int fps_n, fps_dropped;
  bool debug_input;            // QNX_FLUTTER_DEBUG_INPUT: log pointer axis events
} g = {.width = 480, .height = 640};

static uint64_t now_ns(void) { return FlutterEngineGetCurrentTime(); }

static void wake_up(void) {
  char c = 1;
  (void)!write(g.wake[1], &c, 1);
}

// ---- wl_shm buffers ----

static void buffer_release(void* data, struct wl_buffer* wl) {
  (void)wl;
  struct buf* b = data;
  pthread_mutex_lock(&g.buf_mu);
  b->busy = false;
  pthread_mutex_unlock(&g.buf_mu);
}
static const struct wl_buffer_listener buffer_listener = {buffer_release};

static void destroy_buf(struct buf* b) {
  if (b->wl) wl_buffer_destroy(b->wl);
  if (b->data) munmap(b->data, b->size);
  memset(b, 0, sizeof(*b));
}

static bool create_buf(struct buf* b, int w, int h) {
  int stride = w * 4;
  size_t size = (size_t)stride * h;
  char name[64];
  snprintf(name, sizeof name, "/qnx-flutter-%d-%p", getpid(), (void*)b);
  int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
  if (fd < 0) return false;
  shm_unlink(name);
  if (ftruncate(fd, (off_t)size) != 0) { close(fd); return false; }
  void* data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) { close(fd); return false; }
  struct wl_shm_pool* pool = wl_shm_create_pool(g.shm, fd, (int32_t)size);
  b->wl = wl_shm_pool_create_buffer(pool, 0, w, h, stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  close(fd);
  b->data = data;
  b->w = w; b->h = h; b->stride = stride; b->size = size; b->busy = false;
  wl_buffer_add_listener(b->wl, &buffer_listener, b);
  return true;
}

static void resize_bufs(int w, int h) {
  pthread_mutex_lock(&g.buf_mu);
  for (int i = 0; i < NBUF; i++) {
    destroy_buf(&g.bufs[i]);
    if (!create_buf(&g.bufs[i], w, h)) fprintf(stderr, "wl_shm buffer %dx%d failed\n", w, h);
  }
  pthread_mutex_unlock(&g.buf_mu);
}

// Raster thread: copy the engine's frame (32-bit native = BGRA in memory, the
// byte order of WL_SHM_FORMAT_ARGB8888 on little-endian) into a free buffer.
static bool present(void* user_data, const void* allocation, size_t row_bytes, size_t height) {
  (void)user_data;
  pthread_mutex_lock(&g.buf_mu);
  struct buf* b = NULL;
  for (int i = 0; i < NBUF; i++)
    if (g.bufs[i].wl && !g.bufs[i].busy) { b = &g.bufs[i]; break; }
  if (b == NULL) {  // the compositor still holds them all: drop this frame
    g.fps_dropped++;
    pthread_mutex_unlock(&g.buf_mu);
    return true;
  }
  size_t rows = height < (size_t)b->h ? height : (size_t)b->h;
  size_t n = row_bytes < (size_t)b->stride ? row_bytes : (size_t)b->stride;
  for (size_t y = 0; y < rows; y++)
    memcpy((char*)b->data + y * b->stride, (const char*)allocation + y * row_bytes, n);
  b->busy = true;
  if (g.fps_every) {  // QNX_FLUTTER_FPS=<seconds>: frames presented per second
    uint64_t t = now_ns();
    if (g.fps_t0 == 0) g.fps_t0 = t;
    g.fps_n++;
    if (t - g.fps_t0 >= (uint64_t)g.fps_every * 1000000000ULL) {
      fprintf(stderr, "qnx-flutter-wayland: %.1f fps (%dx%d), %d frame(s) dropped\n", g.fps_n * 1e9 / (double)(t - g.fps_t0),
              b->w, b->h, g.fps_dropped);
      g.fps_t0 = t; g.fps_n = 0; g.fps_dropped = 0;
    }
  }
  wl_surface_attach(g.surface, b->wl, 0, 0);
  wl_surface_damage_buffer(g.surface, 0, 0, b->w, b->h);
  wl_surface_commit(g.surface);
  pthread_mutex_unlock(&g.buf_mu);
  wl_display_flush(g.display);
  return true;
}

// ---- engine events ----

static void send_metrics(void) {
  FlutterWindowMetricsEvent ev = {0};
  ev.struct_size = sizeof ev;
  ev.width = (size_t)g.width;
  ev.height = (size_t)g.height;
  ev.pixel_ratio = 1.0;
  FlutterEngineSendWindowMetricsEvent(g.engine, &ev);
}

static void send_pointer(FlutterPointerPhase phase) {
  FlutterPointerEvent ev = {0};
  ev.struct_size = sizeof ev;
  ev.phase = phase;
  ev.timestamp = (size_t)(now_ns() / 1000);
  ev.x = g.px;
  ev.y = g.py;
  ev.device = 0;
  ev.signal_kind = kFlutterPointerSignalKindNone;
  ev.device_kind = kFlutterPointerDeviceKindMouse;
  ev.buttons = g.buttons;
  FlutterEngineSendPointerEvent(g.engine, &ev, 1);
}

// ---- xdg-shell ----

static void wm_base_ping(void* data, struct xdg_wm_base* wm, uint32_t serial) {
  (void)data;
  xdg_wm_base_pong(wm, serial);
}
static const struct xdg_wm_base_listener wm_base_listener = {wm_base_ping};

static void xdg_surface_configure(void* data, struct xdg_surface* s, uint32_t serial) {
  (void)data;
  xdg_surface_ack_configure(s, serial);
  if (!g.configured || g.bufs[0].w != g.width || g.bufs[0].h != g.height) {
    resize_bufs(g.width, g.height);
    if (g.engine) send_metrics();
  }
  g.configured = true;
}
static const struct xdg_surface_listener xdg_surface_listener = {xdg_surface_configure};

static void toplevel_configure(void* data, struct xdg_toplevel* t, int32_t w, int32_t h,
                               struct wl_array* states) {
  (void)data; (void)t; (void)states;
  if (w > 0 && h > 0) { g.width = w; g.height = h; }
}
static void toplevel_close(void* data, struct xdg_toplevel* t) {
  (void)data; (void)t;
  g.closed = true;
}
static void toplevel_configure_bounds(void* data, struct xdg_toplevel* t, int32_t w, int32_t h) {
  (void)data; (void)t; (void)w; (void)h;
}
static void toplevel_wm_capabilities(void* data, struct xdg_toplevel* t, struct wl_array* c) {
  (void)data; (void)t; (void)c;
}
static const struct xdg_toplevel_listener toplevel_listener = {
    toplevel_configure, toplevel_close, toplevel_configure_bounds, toplevel_wm_capabilities};

// ---- pointer ----

static void ptr_enter(void* data, struct wl_pointer* p, uint32_t serial, struct wl_surface* s,
                      wl_fixed_t x, wl_fixed_t y) {
  (void)data; (void)p; (void)serial; (void)s;
  g.px = wl_fixed_to_double(x);
  g.py = wl_fixed_to_double(y);
  if (!g.pointer_added) { send_pointer(kAdd); g.pointer_added = true; }
  send_pointer(kHover);
}
static void ptr_leave(void* data, struct wl_pointer* p, uint32_t serial, struct wl_surface* s) {
  (void)data; (void)p; (void)serial; (void)s;
  if (g.pointer_added && g.buttons == 0) { send_pointer(kRemove); g.pointer_added = false; }
}
static void ptr_motion(void* data, struct wl_pointer* p, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  (void)data; (void)p; (void)time;
  g.px = wl_fixed_to_double(x);
  g.py = wl_fixed_to_double(y);
  if (!g.pointer_added) { send_pointer(kAdd); g.pointer_added = true; }
  send_pointer(g.buttons ? kMove : kHover);
}
static void ptr_button(void* data, struct wl_pointer* p, uint32_t serial, uint32_t time,
                       uint32_t button, uint32_t state) {
  (void)data; (void)p; (void)serial; (void)time;
  int64_t bit = button == 0x110 /* BTN_LEFT */ ? kFlutterPointerButtonMousePrimary
              : button == 0x111 /* BTN_RIGHT */ ? kFlutterPointerButtonMouseSecondary
              : button == 0x112 /* BTN_MIDDLE */ ? kFlutterPointerButtonMouseMiddle : 0;
  if (bit == 0) return;
  if (!g.pointer_added) { send_pointer(kAdd); g.pointer_added = true; }
  if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
    bool first = g.buttons == 0;
    g.buttons |= bit;
    send_pointer(first ? kDown : kMove);
  } else {
    g.buttons &= ~bit;
    send_pointer(g.buttons == 0 ? kUp : kMove);
  }
}
// Wheel and touchpad scrolling: a Flutter pointer signal (scroll) at the
// pointer position. Wayland gives the distance in surface pixels per event
// (about 10-15 for one wheel notch); Flutter scrolls by the logical pixels it
// is given, so a notch is made about three lines of a list.
static void ptr_axis(void* data, struct wl_pointer* p, uint32_t time, uint32_t axis, wl_fixed_t v) {
  (void)data; (void)p; (void)time;
  double d = wl_fixed_to_double(v) * 4.0;
  if (g.debug_input) fprintf(stderr, "qnx-flutter-wayland: axis %u value %.3f -> scroll %.1f at %.0f,%.0f\n", axis, wl_fixed_to_double(v), d, g.px, g.py);
  if (!g.pointer_added) { send_pointer(kAdd); g.pointer_added = true; }
  FlutterPointerEvent ev = {0};
  ev.struct_size = sizeof ev;
  ev.phase = g.buttons ? kMove : kHover;
  ev.timestamp = (size_t)(now_ns() / 1000);
  ev.x = g.px;
  ev.y = g.py;
  ev.device = 0;
  ev.signal_kind = kFlutterPointerSignalKindScroll;
  ev.device_kind = kFlutterPointerDeviceKindMouse;
  ev.buttons = g.buttons;
  if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) ev.scroll_delta_y = d;
  else ev.scroll_delta_x = d;
  FlutterEngineSendPointerEvent(g.engine, &ev, 1);
}
static void ptr_frame(void* data, struct wl_pointer* p) { (void)data; (void)p; }
static void ptr_axis_source(void* data, struct wl_pointer* p, uint32_t s) { (void)data; (void)p; (void)s; }
static void ptr_axis_stop(void* data, struct wl_pointer* p, uint32_t t, uint32_t a) {
  (void)data; (void)p; (void)t; (void)a;
}
static void ptr_axis_discrete(void* data, struct wl_pointer* p, uint32_t a, int32_t d) {
  (void)data; (void)p; (void)a; (void)d;
}
static const struct wl_pointer_listener pointer_listener = {
    ptr_enter, ptr_leave, ptr_motion, ptr_button, ptr_axis,
    ptr_frame, ptr_axis_source, ptr_axis_stop, ptr_axis_discrete};

static void seat_caps(void* data, struct wl_seat* seat, uint32_t caps) {
  (void)data;
  if ((caps & WL_SEAT_CAPABILITY_POINTER) && g.pointer == NULL) {
    g.pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(g.pointer, &pointer_listener, NULL);
  }
}
static void seat_name(void* data, struct wl_seat* seat, const char* name) {
  (void)data; (void)seat; (void)name;
}
static const struct wl_seat_listener seat_listener = {seat_caps, seat_name};

static void registry_global(void* data, struct wl_registry* r, uint32_t name, const char* iface,
                            uint32_t version) {
  (void)data;
  if (strcmp(iface, wl_compositor_interface.name) == 0)
    g.compositor = wl_registry_bind(r, name, &wl_compositor_interface, version < 4 ? version : 4);
  else if (strcmp(iface, wl_shm_interface.name) == 0)
    g.shm = wl_registry_bind(r, name, &wl_shm_interface, 1);
  else if (strcmp(iface, xdg_wm_base_interface.name) == 0) {
    g.wm_base = wl_registry_bind(r, name, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(g.wm_base, &wm_base_listener, NULL);
  } else if (strcmp(iface, wl_seat_interface.name) == 0 && g.seat == NULL) {
    g.seat = wl_registry_bind(r, name, &wl_seat_interface, version < 5 ? version : 5);
    wl_seat_add_listener(g.seat, &seat_listener, NULL);
  }
}
static void registry_remove(void* data, struct wl_registry* r, uint32_t name) {
  (void)data; (void)r; (void)name;
}
static const struct wl_registry_listener registry_listener = {registry_global, registry_remove};

// ---- platform task runner ----

static bool runs_on_platform(void* user_data) {
  (void)user_data;
  return pthread_equal(pthread_self(), g.platform_thread);
}

static void post_task(FlutterTask task, uint64_t target_time, void* user_data) {
  (void)user_data;
  pthread_mutex_lock(&g.task_mu);
  if (g.ntasks < MAX_TASKS) {
    g.tasks[g.ntasks].task = task;
    g.tasks[g.ntasks].target = target_time;
    g.ntasks++;
  } else {
    fprintf(stderr, "task queue full\n");
  }
  pthread_mutex_unlock(&g.task_mu);
  wake_up();
}

// Runs the tasks that are due; returns the poll timeout (ms) until the next one.
static int run_due_tasks(void) {
  for (;;) {
    FlutterTask t;
    bool found = false;
    uint64_t now = now_ns(), next = UINT64_MAX;
    pthread_mutex_lock(&g.task_mu);
    for (int i = 0; i < g.ntasks; i++) {
      if (g.tasks[i].target <= now) {
        t = g.tasks[i].task;
        g.tasks[i] = g.tasks[--g.ntasks];
        found = true;
        break;
      }
      if (g.tasks[i].target < next) next = g.tasks[i].target;
    }
    pthread_mutex_unlock(&g.task_mu);
    if (found) {
      FlutterEngineRunTask(g.engine, &t);
      continue;
    }
    if (next == UINT64_MAX) return -1;
    uint64_t ms = (next - now + 999999) / 1000000;
    return ms > 1000 ? 1000 : (int)ms;
  }
}

static void on_signal(int s) {
  (void)s;
  g.closed = true;
  wake_up();
}

static void usage(FILE* f, const char* prog) {
  fprintf(f,
          "usage: %s [--size WxH] [--title TEXT] [--app-id ID] <bundle dir>\n"
          "Runs a Flutter app (release AOT) in a Wayland window (labwc).\n"
          "  <bundle dir>/flutter_assets/  assets (flutter build bundle)\n"
          "  <bundle dir>/libapp.so        the app's AOT code (gen_snapshot app-aot-elf)\n"
          "  <bundle dir>/icudtl.dat       optional, else %s\n"
          "  --size WxH     window size the app starts with (default 480x640);\n"
          "                 the compositor may still resize it\n"
          "  --title TEXT   window title (default \"Flutter on QNX\")\n"
          "  --app-id ID    Wayland app id (default qnx-flutter-wayland)\n"
          "QNX_FLUTTER_FPS=<s> prints the frames presented per second every <s> s.\n"
          "Needs WAYLAND_DISPLAY and XDG_RUNTIME_DIR of the desktop session.\n",
          prog, ICU_DATA_DEFAULT);
}

int main(int argc, char** argv) {
  const char* title = "Flutter on QNX";
  const char* app_id = "qnx-flutter-wayland";
  const char* dir = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { usage(stdout, argv[0]); return 0; }
    if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
      int w = 0, h = 0;
      if (sscanf(argv[++i], "%dx%d", &w, &h) != 2 || w < 64 || h < 64 || w > 8192 || h > 8192) {
        fprintf(stderr, "bad --size %s (want WxH, 64..8192)\n", argv[i]); return 2;
      }
      g.width = w; g.height = h;
    } else if (strcmp(argv[i], "--title") == 0 && i + 1 < argc) title = argv[++i];
    else if (strcmp(argv[i], "--app-id") == 0 && i + 1 < argc) app_id = argv[++i];
    else if (argv[i][0] == '-') { fprintf(stderr, "unknown option %s\n", argv[i]); usage(stderr, argv[0]); return 2; }
    else if (!dir) dir = argv[i];
    else { usage(stderr, argv[0]); return 2; }
  }
  if (!dir) { usage(stderr, argv[0]); return 2; }
  const char* fps = getenv("QNX_FLUTTER_FPS");
  if (fps && *fps) { g.fps_every = atoi(fps); if (g.fps_every <= 0) g.fps_every = 5; }
  g.debug_input = getenv("QNX_FLUTTER_DEBUG_INPUT") != NULL;
  char assets[PATH_MAX], icu[PATH_MAX], elf[PATH_MAX];
  snprintf(assets, sizeof assets, "%s/flutter_assets", dir);
  snprintf(icu, sizeof icu, "%s/icudtl.dat", dir);
  if (access(icu, R_OK) != 0) snprintf(icu, sizeof icu, "%s", ICU_DATA_DEFAULT);
  snprintf(elf, sizeof elf, "%s/libapp.so", dir);

  pthread_mutex_init(&g.buf_mu, NULL);
  pthread_mutex_init(&g.task_mu, NULL);
  if (pipe2(g.wake, O_CLOEXEC | O_NONBLOCK) != 0) { perror("pipe2"); return 1; }
  g.platform_thread = pthread_self();
  signal(SIGINT, on_signal);
  signal(SIGTERM, on_signal);

  g.display = wl_display_connect(NULL);
  if (!g.display) { fprintf(stderr, "cannot connect to the Wayland display (WAYLAND_DISPLAY, XDG_RUNTIME_DIR)\n"); return 1; }
  struct wl_registry* reg = wl_display_get_registry(g.display);
  wl_registry_add_listener(reg, &registry_listener, NULL);
  wl_display_roundtrip(g.display);
  wl_display_roundtrip(g.display);
  if (!g.compositor || !g.shm || !g.wm_base) { fprintf(stderr, "compositor lacks wl_compositor/wl_shm/xdg_wm_base\n"); return 1; }

  g.surface = wl_compositor_create_surface(g.compositor);
  g.xdg_surface = xdg_wm_base_get_xdg_surface(g.wm_base, g.surface);
  xdg_surface_add_listener(g.xdg_surface, &xdg_surface_listener, NULL);
  g.toplevel = xdg_surface_get_toplevel(g.xdg_surface);
  xdg_toplevel_add_listener(g.toplevel, &toplevel_listener, NULL);
  xdg_toplevel_set_title(g.toplevel, title);
  xdg_toplevel_set_app_id(g.toplevel, app_id);
  wl_surface_commit(g.surface);
  while (!g.configured && wl_display_dispatch(g.display) != -1) {}

  FlutterEngineAOTDataSource src = {0};
  src.type = kFlutterEngineAOTDataSourceTypeElfPath;
  src.elf_path = elf;
  FlutterEngineAOTData aot = NULL;
  if (FlutterEngineCreateAOTData(&src, &aot) != kSuccess) { fprintf(stderr, "cannot load %s\n", elf); return 1; }

  FlutterRendererConfig rc = {0};
  rc.type = kSoftware;
  rc.software.struct_size = sizeof(FlutterSoftwareRendererConfig);
  rc.software.surface_present_callback = present;

  FlutterTaskRunnerDescription platform = {0};
  platform.struct_size = sizeof platform;
  platform.runs_task_on_current_thread_callback = runs_on_platform;
  platform.post_task_callback = post_task;
  FlutterCustomTaskRunners runners = {0};
  runners.struct_size = sizeof runners;
  runners.platform_task_runner = &platform;

  FlutterProjectArgs args = {0};
  args.struct_size = sizeof args;
  args.assets_path = assets;
  args.icu_data_path = icu;
  args.aot_data = aot;
  args.custom_task_runners = &runners;

  FlutterEngineResult r = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &rc, &args, NULL, &g.engine);
  if (r != kSuccess) { fprintf(stderr, "FlutterEngineRun failed: %d\n", (int)r); return 1; }
  send_metrics();
  printf("qnx-flutter-wayland: running %s (%dx%d)\n", dir, g.width, g.height);
  fflush(stdout);

  while (!g.closed) {
    int timeout = run_due_tasks();
    while (wl_display_prepare_read(g.display) != 0) wl_display_dispatch_pending(g.display);
    wl_display_flush(g.display);
    struct pollfd fds[2] = {{wl_display_get_fd(g.display), POLLIN, 0}, {g.wake[0], POLLIN, 0}};
    int n = poll(fds, 2, timeout);
    if (n > 0 && (fds[0].revents & POLLIN)) {
      if (wl_display_read_events(g.display) == -1) break;
    } else {
      wl_display_cancel_read(g.display);
    }
    if (fds[0].revents & (POLLERR | POLLHUP)) break;
    if (fds[1].revents & POLLIN) {
      char tmp[64];
      while (read(g.wake[0], tmp, sizeof tmp) > 0) {}
    }
    wl_display_dispatch_pending(g.display);
  }

  FlutterEngineShutdown(g.engine);
  FlutterEngineCollectAOTData(aot);
  for (int i = 0; i < NBUF; i++) destroy_buf(&g.bufs[i]);
  wl_display_disconnect(g.display);
  return 0;
}
