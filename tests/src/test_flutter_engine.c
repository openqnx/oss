/* flutter-engine: the embedder API of libflutter_engine.so on QNX.
 *  - the procedure table is complete and the engine is the AOT (release) one;
 *  - the engine's clock runs (FlutterEngineGetCurrentTime is monotonic);
 *  - an AOT ELF that is not there is refused, a real one loads (the counter
 *    example's libapp.so, from flutter-counter-example);
 *  - the engine runs that app headless with the software renderer: after a
 *    window size it presents frames of that size, with a painted surface (the
 *    counter app's light background) -- Dart VM, framework, Skia raster and
 *    the fml message loop together. Platform tasks run on this thread from a
 *    small queue (the engine's custom task runner API).
 */
#include "common.h"
#include <flutter_embedder.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define APP "/usr/local/share/flutter-apps/counter"
#define MAXT 256

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_t platform;
static struct { FlutterTask t; uint64_t when; } q[MAXT];
static int nq;
static volatile int frames, painted;
static volatile size_t last_w, last_h;

static bool on_platform(void* u) { (void)u; return pthread_equal(pthread_self(), platform); }
static void post(FlutterTask t, uint64_t when, void* u) {
  (void)u;
  pthread_mutex_lock(&mu);
  if (nq < MAXT) { q[nq].t = t; q[nq].when = when; nq++; }
  pthread_mutex_unlock(&mu);
}
static void run_due(FlutterEngine e) {
  for (;;) {
    FlutterTask t; int found = 0;
    uint64_t now = FlutterEngineGetCurrentTime();
    pthread_mutex_lock(&mu);
    for (int i = 0; i < nq; i++)
      if (q[i].when <= now) { t = q[i].t; q[i] = q[--nq]; found = 1; break; }
    pthread_mutex_unlock(&mu);
    if (!found) return;
    FlutterEngineRunTask(e, &t);
  }
}
static bool present(void* u, const void* px, size_t row_bytes, size_t height) {
  (void)u;
  const uint32_t* p = px;
  size_t n = row_bytes / 4;
  /* the surface in the middle of the frame: the counter app's body is near-white */
  uint32_t c = p[(height / 2) * n + n / 2];
  if (((c >> 16) & 0xff) > 0xe0 && ((c >> 8) & 0xff) > 0xe0 && (c & 0xff) > 0xe0) painted = 1;
  last_w = n; last_h = height;
  frames++;
  return true;
}

int main(void) {
  QT_BEGIN("flutter_engine");
  FlutterEngineProcTable pt;
  memset(&pt, 0, sizeof pt);
  pt.struct_size = sizeof pt;
  QT_CHECK(FlutterEngineGetProcAddresses(&pt) == kSuccess && pt.Run && pt.RunTask && pt.SendWindowMetricsEvent,
           "procedure table complete");
  QT_CHECK(FlutterEngineRunsAOTCompiledDartCode(), "release engine: runs AOT compiled Dart code");
  uint64_t t0 = FlutterEngineGetCurrentTime();
  usleep(20000);
  uint64_t t1 = FlutterEngineGetCurrentTime();
  QT_CHECK(t1 > t0 && t1 - t0 >= 15000000ULL, "engine clock advances (20 ms sleep)");

  FlutterEngineAOTDataSource src = {0};
  src.type = kFlutterEngineAOTDataSourceTypeElfPath;
  src.elf_path = "/nonexistent/libapp.so";
  FlutterEngineAOTData bad = NULL;
  QT_CHECK(FlutterEngineCreateAOTData(&src, &bad) != kSuccess, "a missing AOT ELF is refused");

  if (access(APP "/libapp.so", R_OK) != 0) {
    printf("  info %s not installed (flutter-counter-example): no app run\n", APP);
    QT_END();
  }
  src.elf_path = APP "/libapp.so";
  FlutterEngineAOTData aot = NULL;
  QT_CHECK(FlutterEngineCreateAOTData(&src, &aot) == kSuccess && aot, "the counter app's AOT ELF loads");

  platform = pthread_self();
  FlutterRendererConfig rc = {0};
  rc.type = kSoftware;
  rc.software.struct_size = sizeof(FlutterSoftwareRendererConfig);
  rc.software.surface_present_callback = present;
  FlutterTaskRunnerDescription tr = {0};
  tr.struct_size = sizeof tr;
  tr.runs_task_on_current_thread_callback = on_platform;
  tr.post_task_callback = post;
  FlutterCustomTaskRunners runners = {0};
  runners.struct_size = sizeof runners;
  runners.platform_task_runner = &tr;
  FlutterProjectArgs args = {0};
  args.struct_size = sizeof args;
  args.assets_path = APP "/flutter_assets";
  args.icu_data_path = "/usr/local/share/flutter/icudtl.dat";
  args.aot_data = aot;
  args.custom_task_runners = &runners;
  FlutterEngine e = NULL;
  FlutterEngineResult r = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &rc, &args, NULL, &e);
  QT_CHECK(r == kSuccess && e, "engine runs the app (software renderer)");
  if (r == kSuccess) {
    FlutterWindowMetricsEvent m = {0};
    m.struct_size = sizeof m;
    m.width = 320; m.height = 240; m.pixel_ratio = 1.0;
    QT_CHECK(FlutterEngineSendWindowMetricsEvent(e, &m) == kSuccess, "window metrics 320x240 accepted");
    for (int i = 0; i < 1500 && (frames < 2 || !painted); i++) { run_due(e); usleep(10000); }
    printf("  info %d frame(s), last %zux%zu\n", frames, (size_t)last_w, (size_t)last_h);
    QT_CHECK(frames >= 1, "frames presented within 15 s");
    QT_CHECK(last_w == 320 && last_h == 240, "frames have the window size 320x240");
    QT_CHECK(painted, "the app's surface is painted (light body colour in the middle)");
    QT_CHECK(FlutterEngineShutdown(e) == kSuccess, "engine shuts down");
  }
  FlutterEngineCollectAOTData(aot);
  QT_END();
}
