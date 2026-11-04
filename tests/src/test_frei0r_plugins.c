/* frei0r-plugins: load plugins the way ffmpeg/MLT do -- dlopen() from
 * lib/frei0r-1, f0r_init, f0r_get_plugin_info, construct an instance, set
 * parameters, f0r_update on a known 8x4 frame -- and check the output pixels:
 * invert0r (a filter: 255-x per channel), onecol0r (a source: a solid colour
 * from its parameter), addition (a mixer: saturating sum), and that a cairo
 * plugin (cairogradient) and the OpenCV one (facedetect) load. Plus the
 * number of plugins installed. */
#include "common.h"
#include <frei0r.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>

static char dir[256];

typedef struct {
  void *h;
  int (*init)(void); void (*deinit)(void);
  void (*info)(f0r_plugin_info_t *);
  f0r_instance_t (*construct)(unsigned, unsigned);
  void (*destruct)(f0r_instance_t);
  void (*set)(f0r_instance_t, f0r_param_t, int);
  void (*update)(f0r_instance_t, double, const uint32_t *, uint32_t *);
  void (*update2)(f0r_instance_t, double, const uint32_t *, const uint32_t *, const uint32_t *, uint32_t *);
} plug;

static int load(plug *p, const char *name) {
  char fn[512]; snprintf(fn, sizeof fn, "%s/%s.so", dir, name);
  memset(p, 0, sizeof *p);
  p->h = dlopen(fn, RTLD_NOW | RTLD_LOCAL);
  if (!p->h) { printf("  info %s\n", dlerror()); return 0; }
  p->init = (int (*)(void))dlsym(p->h, "f0r_init");
  p->deinit = (void (*)(void))dlsym(p->h, "f0r_deinit");
  p->info = (void (*)(f0r_plugin_info_t *))dlsym(p->h, "f0r_get_plugin_info");
  p->construct = (f0r_instance_t (*)(unsigned, unsigned))dlsym(p->h, "f0r_construct");
  p->destruct = (void (*)(f0r_instance_t))dlsym(p->h, "f0r_destruct");
  p->set = (void (*)(f0r_instance_t, f0r_param_t, int))dlsym(p->h, "f0r_set_param_value");
  p->update = (void (*)(f0r_instance_t, double, const uint32_t *, uint32_t *))dlsym(p->h, "f0r_update");
  p->update2 = (void (*)(f0r_instance_t, double, const uint32_t *, const uint32_t *, const uint32_t *, uint32_t *))dlsym(p->h, "f0r_update2");
  return p->init && p->info && p->construct && p->update && p->init();
}

int main(void) {
  QT_BEGIN("frei0r-plugins");
  const char *dirs[] = { "/usr/local/lib/frei0r-1", "/data/oss/usr/local/lib/frei0r-1" };
  int n = 0;
  for (int i = 0; i < 2 && !n; i++) {
    DIR *d = opendir(dirs[i]); if (!d) continue;
    struct dirent *e; while ((e = readdir(d))) if (strstr(e->d_name, ".so")) n++;
    closedir(d); snprintf(dir, sizeof dir, "%s", dirs[i]);
  }
  printf("  info %d plugins in %s\n", n, dir);
  QT_CHECK(n > 100, "over 100 plugins installed");

  enum { W = 8, H = 4 };
  uint32_t in[W * H], in2[W * H], out[W * H];
  for (int i = 0; i < W * H; i++) { in[i] = 0xff000000u | (i * 8) << 16 | 0x40 << 8 | 0x10; in2[i] = 0xff202020u; }

  plug p; f0r_plugin_info_t info;
  if (load(&p, "invert0r")) {
    p.info(&info);
    QT_CHECK(info.plugin_type == F0R_PLUGIN_TYPE_FILTER && info.frei0r_version == FREI0R_MAJOR_VERSION, "invert0r: a filter, frei0r API 1");
    f0r_instance_t inst = p.construct(W, H);
    p.update(inst, 0, in, out);
    int ok = 1;
    for (int i = 0; i < W * H; i++) if ((out[i] & 0xffffff) != (~in[i] & 0xffffff)) ok = 0;
    QT_CHECK(ok, "invert0r: every pixel 255-x per channel");
    p.destruct(inst); p.deinit(); dlclose(p.h);
  } else QT_CHECK(0, "invert0r loads");

  if (load(&p, "onecol0r")) {
    p.info(&info);
    QT_CHECK(info.plugin_type == F0R_PLUGIN_TYPE_SOURCE, "onecol0r: a source");
    f0r_instance_t inst = p.construct(W, H);
    f0r_param_color_t c = { 1.0f, 0.5f, 0.0f };
    p.set(inst, &c, 0);
    p.update(inst, 0, NULL, out);
    unsigned px = out[5];
    printf("  info onecol0r pixel 0x%08x\n", px);
    /* bytes in memory: R,G,B,A for RGBA8888, B,G,R,A for BGRA8888 (onecol0r's) */
    int rs = info.color_model == F0R_COLOR_MODEL_BGRA8888 ? 16 : 0, bs = 16 - rs;
    printf("  info color model %d\n", info.color_model);
    QT_CHECK(((px >> rs) & 0xff) == 0xff && ((px >> 8) & 0xff) >= 0x7e && ((px >> 8) & 0xff) <= 0x80 && ((px >> bs) & 0xff) == 0 && out[0] == out[W * H - 1], "onecol0r: the whole frame is the colour (1, 0.5, 0)");
    p.destruct(inst); p.deinit(); dlclose(p.h);
  } else QT_CHECK(0, "onecol0r loads");

  if (load(&p, "addition") && p.update2) {
    p.info(&info);
    QT_CHECK(info.plugin_type == F0R_PLUGIN_TYPE_MIXER2, "addition: a two-input mixer");
    f0r_instance_t inst = p.construct(W, H);
    p.update2(inst, 0, in, in2, NULL, out);
    int ok = 1;
    for (int i = 0; i < W * H; i++) {
      unsigned r = (in[i] >> 16 & 0xff) + 0x20; if (r > 255) r = 255;
      if ((out[i] >> 16 & 0xff) != r || (out[i] >> 8 & 0xff) != 0x60 || (out[i] & 0xff) != 0x30) ok = 0;
    }
    QT_CHECK(ok, "addition: channel sums, saturating at 255");
    p.destruct(inst); p.deinit(); dlclose(p.h);
  } else QT_CHECK(0, "addition loads");

  const char *more[] = { "cairogradient", "facedetect" };
  for (int i = 0; i < 2; i++) {
    int ok = load(&p, more[i]);
    if (ok) { p.info(&info); printf("  info %s: %s by %s\n", more[i], info.name, info.author); p.deinit(); dlclose(p.h); }
    char w[64]; snprintf(w, sizeof w, "%s loads and initialises", more[i]);
    QT_CHECK(ok, w);
  }
  QT_END();
}
