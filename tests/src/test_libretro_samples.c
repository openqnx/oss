/* libretro cores, driven by a minimal libretro frontend written here.
 * This file is the test for libretro-samples (its "test" core: a moving
 * checkerboard and a 300 Hz tone, no content needed); the other
 * test_libretro_*.c files define CORE_* and include it.
 *
 * The frontend dlopens the core, checks retro_api_version and the system
 * info, installs environment / video / audio / input callbacks, loads the
 * content (none, or a PICO-8 cart written to /tmp), runs frames and checks
 * that video frames of the advertised geometry arrive (and, where known,
 * what they show), that audio samples arrive, that save states round-trip
 * when the core offers them, and that unload/deinit return cleanly.
 * QNXPORT_LIBRETRO_DIR overrides the core directory (/usr/local/lib/libretro). */
#include "common.h"
#include <dlfcn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef CORE_FILE
#define CORE_TEST_NAME "libretro-samples"
#define CORE_FILE "test_libretro_qnx.so"
#define CORE_LIBNAME "TestCore"
#define CORE_FRAMES 120
#define CORE_EXPECT_AUDIO 1
#define CORE_CHECK_COLOURS 2      /* at least this many distinct colours in the last frame */
#endif
#ifndef CORE_CART
#define CORE_CART NULL            /* NULL: start without content */
#endif
#ifndef CORE_EXPECT_RED
#define CORE_EXPECT_RED 0         /* the cart clears the screen to PICO-8 red */
#endif
#ifndef CORE_CHECK_COLOURS
#define CORE_CHECK_COLOURS 0
#endif

/* ---- the slice of libretro.h a frontend needs (ABI version 1) ---- */
#define RETRO_API_VERSION 1
#define RETRO_ENVIRONMENT_EXPERIMENTAL 0x10000
enum { ENV_GET_CAN_DUPE = 3, ENV_GET_SYSTEM_DIRECTORY = 9, ENV_SET_PIXEL_FORMAT = 10, ENV_GET_VARIABLE = 15,
       ENV_GET_VARIABLE_UPDATE = 17, ENV_SET_SUPPORT_NO_GAME = 18, ENV_SET_FRAME_TIME_CALLBACK = 21,
       ENV_SET_AUDIO_CALLBACK = 22, ENV_GET_LOG_INTERFACE = 27, ENV_GET_SAVE_DIRECTORY = 31,
       ENV_SET_SYSTEM_AV_INFO = 32, ENV_SET_GEOMETRY = 37 };
enum { PIX_0RGB1555 = 0, PIX_XRGB8888 = 1, PIX_RGB565 = 2 };
struct retro_system_info { const char *library_name, *library_version, *valid_extensions; bool need_fullpath, block_extract; };
struct retro_game_geometry { unsigned base_width, base_height, max_width, max_height; float aspect_ratio; };
struct retro_system_timing { double fps, sample_rate; };
struct retro_system_av_info { struct retro_game_geometry geometry; struct retro_system_timing timing; };
struct retro_game_info { const char *path; const void *data; size_t size; const char *meta; };
typedef void (*retro_log_printf_t)(int level, const char *fmt, ...);
struct retro_log_callback { retro_log_printf_t log; };
typedef void (*retro_frame_time_callback_t)(int64_t usec);
struct retro_frame_time_callback { retro_frame_time_callback_t callback; int64_t reference; };
typedef bool (*env_t)(unsigned, void *);
typedef void (*video_t)(const void *, unsigned, unsigned, size_t);
typedef void (*audio_t)(int16_t, int16_t);
typedef size_t (*audio_batch_t)(const int16_t *, size_t);
typedef void (*poll_t)(void);
typedef int16_t (*state_t)(unsigned, unsigned, unsigned, unsigned);

static struct {
  int pixfmt, no_game, log_errors;
  long frames, dupes, samples;
  unsigned w, h; size_t pitch;
  unsigned char *last; size_t last_size;
  retro_frame_time_callback_t ftcb; int64_t ftref;
  char dir[96];
} fe = {PIX_0RGB1555, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, NULL, 0, ""};

static void log_cb(int level, const char *fmt, ...) {
  char buf[512];
  va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
  size_t n = strlen(buf);
  while (n && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) buf[--n] = 0;
  if (level >= 2) { printf("  info core log[%d]: %s\n", level, buf); if (level >= 3) ++fe.log_errors; }
}
static bool env_cb(unsigned cmd, void *data) {
  switch (cmd & ~RETRO_ENVIRONMENT_EXPERIMENTAL) {
    case ENV_GET_CAN_DUPE: if (data) *(bool *)data = true; return true;
    case ENV_GET_SYSTEM_DIRECTORY: case ENV_GET_SAVE_DIRECTORY: if (data) *(const char **)data = fe.dir; return true;
    case ENV_SET_PIXEL_FORMAT: if (!data) return false; fe.pixfmt = *(const int *)data; return fe.pixfmt >= 0 && fe.pixfmt <= 2;
    case ENV_SET_SUPPORT_NO_GAME: if (data) fe.no_game = *(const bool *)data; return true;
    case ENV_GET_LOG_INTERFACE: if (data) ((struct retro_log_callback *)data)->log = log_cb; return true;
    case ENV_SET_FRAME_TIME_CALLBACK: {
      const struct retro_frame_time_callback *f = data;
      if (f) { fe.ftcb = f->callback; fe.ftref = f->reference; }
      return true;
    }
    case ENV_SET_SYSTEM_AV_INFO: case ENV_SET_GEOMETRY: return true;
    case ENV_GET_VARIABLE: case ENV_GET_VARIABLE_UPDATE: case ENV_SET_AUDIO_CALLBACK: return false;
    default: return false;
  }
}
static void video_cb(const void *d, unsigned w, unsigned h, size_t pitch) {
  ++fe.frames;
  if (!d) { ++fe.dupes; return; }
  fe.w = w; fe.h = h; fe.pitch = pitch;
  size_t sz = pitch * h;
  if (sz != fe.last_size) { free(fe.last); fe.last = malloc(sz); fe.last_size = sz; }
  if (fe.last) memcpy(fe.last, d, sz);
}
static void audio_cb(int16_t l, int16_t r) { (void)l; (void)r; ++fe.samples; }
static size_t audio_batch_cb(const int16_t *d, size_t frames) { (void)d; fe.samples += (long)frames; return frames; }
static void poll_cb(void) {}
static int16_t state_cb(unsigned port, unsigned dev, unsigned idx, unsigned id) { (void)port; (void)dev; (void)idx; (void)id; return 0; }

static void rgb_at(unsigned x, unsigned y, int *r, int *g, int *b) {
  const unsigned char *row = fe.last + y * fe.pitch;
  if (fe.pixfmt == PIX_XRGB8888) {
    uint32_t p; memcpy(&p, row + x * 4, 4);
    *r = (p >> 16) & 255; *g = (p >> 8) & 255; *b = p & 255;
  } else {
    uint16_t p; memcpy(&p, row + x * 2, 2);
    if (fe.pixfmt == PIX_RGB565) { *r = (p >> 11) << 3; *g = ((p >> 5) & 63) << 2; *b = (p & 31) << 3; }
    else { *r = ((p >> 10) & 31) << 3; *g = ((p >> 5) & 31) << 3; *b = (p & 31) << 3; }
  }
}
static int distinct_colours(void) {
  uint32_t seen[16]; int n = 0;
  for (unsigned y = 0; y < fe.h; y += 3)
    for (unsigned x = 0; x < fe.w; x += 3) {
      int r, g, b; rgb_at(x, y, &r, &g, &b);
      uint32_t c = (uint32_t)(r << 16 | g << 8 | b);
      int k; for (k = 0; k < n && seen[k] != c; ++k) {}
      if (k == n && n < 16) seen[n++] = c;
    }
  return n;
}

#define SYM(RET, name, ARGS) RET (*name) ARGS = (RET (*) ARGS)dlsym(h, #name); \
  if (!name) { printf("  info missing symbol %s\n", #name); ++missing; }
int main(void) {
  QT_BEGIN(CORE_TEST_NAME);
  alarm(120);
  char path[256];
  const char *dir = getenv("QNXPORT_LIBRETRO_DIR") ? getenv("QNXPORT_LIBRETRO_DIR") : "/usr/local/lib/libretro";
  snprintf(path, sizeof path, "%s/%s", dir, CORE_FILE);
  snprintf(fe.dir, sizeof fe.dir, "/tmp/qnxport-retro-%d", (int)getpid());
  mkdir(fe.dir, 0755);
  void *h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
  if (!h) printf("  info dlopen: %s\n", dlerror());
  QT_CHECK(h != NULL, "core loads with dlopen");
  if (!h) QT_END();
  int missing = 0;
  SYM(unsigned, retro_api_version, (void))
  SYM(void, retro_get_system_info, (struct retro_system_info *))
  SYM(void, retro_get_system_av_info, (struct retro_system_av_info *))
  SYM(void, retro_set_environment, (env_t))
  SYM(void, retro_set_video_refresh, (video_t))
  SYM(void, retro_set_audio_sample, (audio_t))
  SYM(void, retro_set_audio_sample_batch, (audio_batch_t))
  SYM(void, retro_set_input_poll, (poll_t))
  SYM(void, retro_set_input_state, (state_t))
  SYM(void, retro_init, (void))
  SYM(void, retro_deinit, (void))
  SYM(bool, retro_load_game, (const struct retro_game_info *))
  SYM(void, retro_unload_game, (void))
  SYM(void, retro_run, (void))
  SYM(size_t, retro_serialize_size, (void))
  SYM(bool, retro_serialize, (void *, size_t))
  SYM(bool, retro_unserialize, (const void *, size_t))
  QT_CHECK(missing == 0, "all libretro entry points exported");
  if (missing) QT_END();

  QT_CHECK(retro_api_version() == RETRO_API_VERSION, "retro_api_version() == 1");
  struct retro_system_info si; memset(&si, 0, sizeof si);
  retro_get_system_info(&si);
  printf("  info %s %s, extensions \"%s\"\n", si.library_name ? si.library_name : "(null)",
         si.library_version ? si.library_version : "(null)", si.valid_extensions ? si.valid_extensions : "");
  QT_CHECK(si.library_name && strstr(si.library_name, CORE_LIBNAME) != NULL, "system info names the core");

  retro_set_environment(env_cb);
  retro_set_video_refresh(video_cb);
  retro_set_audio_sample(audio_cb);
  retro_set_audio_sample_batch(audio_batch_cb);
  retro_set_input_poll(poll_cb);
  retro_set_input_state(state_cb);
  retro_init();

  const char *cart = CORE_CART;
  struct retro_game_info gi, *gp = NULL;
  char cartpath[160];
  if (cart) {
    snprintf(cartpath, sizeof cartpath, "%s/qnxtest.p8", fe.dir);
    FILE *f = fopen(cartpath, "w");
    if (f) { fputs(cart, f); fclose(f); }
    gi.path = cartpath; gi.data = cart; gi.size = strlen(cart); gi.meta = NULL;
    gp = &gi;
  } else {
    QT_CHECK(fe.no_game, "core declares it runs without content");
  }
  bool loaded = retro_load_game(gp);
  QT_CHECK(loaded, cart ? "retro_load_game with a PICO-8 cart" : "retro_load_game(NULL)");
  if (!loaded) { retro_deinit(); QT_END(); }

  struct retro_system_av_info av; memset(&av, 0, sizeof av);
  retro_get_system_av_info(&av);
  printf("  info %ux%u (max %ux%u), %.2f fps, %.0f Hz audio, pixel format %d\n", av.geometry.base_width,
         av.geometry.base_height, av.geometry.max_width, av.geometry.max_height, av.timing.fps, av.timing.sample_rate, fe.pixfmt);
  QT_CHECK(av.geometry.base_width > 0 && av.geometry.base_height > 0 && av.timing.fps > 0, "AV info: geometry and frame rate");

  for (int i = 0; i < CORE_FRAMES; ++i) {
    if (fe.ftcb) fe.ftcb(fe.ftref);
    retro_run();
  }
  printf("  info %d run(s): %ld video callback(s) (%ld dupes), last %ux%u pitch %zu, %ld audio frame(s)\n",
         CORE_FRAMES, fe.frames, fe.dupes, fe.w, fe.h, fe.pitch, fe.samples);
  QT_CHECK(fe.frames >= CORE_FRAMES / 2 && fe.last != NULL, "video frames delivered");
  QT_CHECK(fe.w > 0 && fe.w <= (av.geometry.max_width ? av.geometry.max_width : fe.w) &&
           fe.h <= (av.geometry.max_height ? av.geometry.max_height : fe.h) &&
           fe.pitch >= fe.w * (fe.pixfmt == PIX_XRGB8888 ? 4u : 2u), "frame size within the advertised geometry");
  if (CORE_EXPECT_AUDIO) {
    long want = av.timing.sample_rate > 0 ? (long)(av.timing.sample_rate / av.timing.fps * CORE_FRAMES / 4) : 1;
    QT_CHECK(fe.samples >= want, "audio samples delivered");
  }
  if (fe.last && CORE_CHECK_COLOURS) {
    int n = distinct_colours();
    printf("  info %d distinct colour(s) in the last frame\n", n);
    QT_CHECK(n >= CORE_CHECK_COLOURS, "the frame has content (several colours)");
  }
  if (fe.last && CORE_EXPECT_RED) {
    int r, g, b; rgb_at(fe.w / 2, fe.h / 2, &r, &g, &b);
    printf("  info centre pixel rgb(%d,%d,%d)\n", r, g, b);
    QT_CHECK(r > 200 && g < 60 && b > 40 && b < 120, "cls(8) painted the screen PICO-8 red (255,0,77)");
  }
  size_t ss = retro_serialize_size();
  if (ss > 0) {
    void *st = malloc(ss);
    bool s1 = retro_serialize(st, ss);
    if (s1) retro_run();
    bool s2 = s1 && retro_unserialize(st, ss);
    printf("  info save state %zu bytes\n", ss);
    QT_CHECK(s1 && s2, "save state written and restored");
    free(st);
  } else printf("  info no save-state support\n");
  if (fe.log_errors) printf("  info core logged %d error message(s)\n", fe.log_errors);
  retro_unload_game();
  retro_deinit();
  QT_CHECK(1, "unload + deinit returned");
  if (cart) unlink(cartpath);
  rmdir(fe.dir);
  free(fe.last);
  QT_END();
}
