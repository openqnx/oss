/* opencl-icd-loader: libOpenCL.so.1 finds and loads OpenCL drivers and
 * dispatches calls to them. Nothing is compiled at run time and no OpenCL
 * implementation needs to be installed: the drivers are upstream's stub
 * drivers, which the component installs in libexec/opencl-icd-loader/
 * (libOpenCLDriverStub.so, platform "ICD_LOADER_TEST_OPENCL_STUB";
 * libOpenCLDriverStubICD2.so, the same with the ICD 2 dispatch scheme).
 * The loader enumerates drivers once per process, so every case runs in a
 * child process of its own (this program again) with its own environment:
 *  - no driver (OCL_ICD_VENDORS names an empty directory): clGetPlatformIDs
 *    returns CL_PLATFORM_NOT_FOUND_KHR (-1001) and reports 0 platforms;
 *  - an ICD file "stub.icd" holding the driver's path in the directory
 *    OCL_ICD_VENDORS names: one platform, and clGetPlatformInfo -- answered by
 *    the driver through the dispatch table -- gives its name, vendor
 *    "stubvendorxxx", version "OpenCL 1.2 Stub" and ICD suffix "ilts";
 *    clGetDeviceIDs reaches the driver too (1 device), and the driver's own
 *    log shows the call arrived there with the arguments passed;
 *  - OCL_ICD_FILENAMES names the ICD 2 driver: its platform is found without
 *    any ICD file;
 *  - both together: two platforms, the OCL_ICD_FILENAMES one first;
 *  - the ICD file in <prefix>/etc/OpenCL/vendors and no OCL_ICD_VENDORS: the
 *    directory this port adds to the loader's search (ports/opencl-icd-loader/
 *    patches/0001). Checked when the test may write there, skipped otherwise;
 *  - upstream's icd_loader_test, with the plain and the ICD 2 driver: it
 *    calls the whole API through the loader and compares the arguments the
 *    application passed with those the driver logged ("ICD Loader Test
 *    PASSED");
 *  - cllayerinfo lists upstream's test layer (OPENCL_LAYERS): "print_layer",
 *    layer API version 100.
 * A child that does not finish within 60 s is killed and counts as failed. */
#define CL_TARGET_OPENCL_VERSION 300
#include "common.h"
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>

extern char **environ;
#define STUB_NAME  "ICD_LOADER_TEST_OPENCL_STUB"
#define STUB2_NAME "ICD_LOADER_TEST_OPENCL_STUB_ICD2"

/* ---- child: one enumeration, in the environment the parent prepared ---- */
static int pinfo(cl_platform_id p, cl_platform_info what, char *buf, size_t n) {
  buf[0] = 0;
  return clGetPlatformInfo(p, what, n, buf, NULL);
}

static int child(const char *mode) {
  cl_platform_id p[8]; cl_uint n = 77; cl_int rc; char s[256], t[256];
  /* The stub drivers log every call through libIcdLog.so, and the log has to
     be open before the first call (icd_loader_test does the same). */
  if (strcmp(mode, "none")) {
    void *h = dlopen("libIcdLog.so", RTLD_NOW | RTLD_GLOBAL);
    int (*init)(void) = h ? (int (*)(void))dlsym(h, "test_icd_initialize_stub_log") : NULL;
    QT_CHECK(init && init() == 0, "the stub drivers' log (libIcdLog.so) is open");
    if (!init) return 1;
  }
  rc = clGetPlatformIDs(0, NULL, &n);
  if (!strcmp(mode, "none")) {
    QT_CHECK(rc == CL_PLATFORM_NOT_FOUND_KHR, "no driver: clGetPlatformIDs returns CL_PLATFORM_NOT_FOUND_KHR (-1001)");
    QT_CHECK(n == 0, "no driver: 0 platforms");
    rc = clGetPlatformIDs(8, p, NULL);
    QT_CHECK(rc == CL_PLATFORM_NOT_FOUND_KHR, "no driver: the same when asking for the list");
    return qt_fail_count;
  }
  if (!strcmp(mode, "prefix")) {       /* other drivers may be installed there too */
    cl_uint i, found = 0;
    QT_CHECK(rc == CL_SUCCESS && n >= 1 && n <= 8, "<prefix>/etc/OpenCL/vendors: clGetPlatformIDs finds a platform");
    if (rc != CL_SUCCESS || n > 8) return 1;
    rc = clGetPlatformIDs(n, p, NULL);
    for (i = 0; rc == CL_SUCCESS && i < n; i++)
      if (pinfo(p[i], CL_PLATFORM_NAME, s, sizeof s) == CL_SUCCESS && !strcmp(s, STUB_NAME)) found++;
    QT_CHECK(found == 1, "<prefix>/etc/OpenCL/vendors: the stub driver's platform is among them, once");
    return qt_fail_count;
  }
  if (!strcmp(mode, "both")) {
    QT_CHECK(rc == CL_SUCCESS && n == 2, "ICD file and OCL_ICD_FILENAMES: 2 platforms");
    if (rc != CL_SUCCESS || n != 2) return 1;
    rc = clGetPlatformIDs(2, p, &n);
    QT_CHECK(rc == CL_SUCCESS && n == 2 && p[0] != p[1], "two different platform ids");
    pinfo(p[0], CL_PLATFORM_NAME, s, sizeof s); pinfo(p[1], CL_PLATFORM_NAME, t, sizeof t);
    QT_CHECK(!strcmp(s, STUB2_NAME) && !strcmp(t, STUB_NAME), "the OCL_ICD_FILENAMES driver comes first, then the ICD file's");
    pinfo(p[0], CL_PLATFORM_VENDOR, s, sizeof s); pinfo(p[1], CL_PLATFORM_VENDOR, t, sizeof t);
    QT_CHECK(!strcmp(s, "stubvendorxxx") && !strcmp(t, "stubvendorxxx"), "each platform's calls reach its own driver");
    return qt_fail_count;
  }
  /* "icdfile" (plain driver) and "filenames" (ICD 2 driver): one platform */
  { int icd2 = !strcmp(mode, "filenames"); cl_device_id dev = NULL; cl_uint nd = 0; size_t len = 0;
    QT_CHECK(rc == CL_SUCCESS && n == 1, icd2 ? "OCL_ICD_FILENAMES: 1 platform" : "ICD file in OCL_ICD_VENDORS: 1 platform");
    if (rc != CL_SUCCESS || n != 1) return 1;
    p[0] = NULL; rc = clGetPlatformIDs(1, p, NULL);
    QT_CHECK(rc == CL_SUCCESS && p[0] != NULL, "clGetPlatformIDs returns its id");
    rc = pinfo(p[0], CL_PLATFORM_NAME, s, sizeof s);
    QT_CHECK(rc == CL_SUCCESS && !strcmp(s, icd2 ? STUB2_NAME : STUB_NAME),
             icd2 ? "CL_PLATFORM_NAME is " STUB2_NAME : "CL_PLATFORM_NAME is " STUB_NAME);
    rc = pinfo(p[0], CL_PLATFORM_VENDOR, s, sizeof s);
    QT_CHECK(rc == CL_SUCCESS && !strcmp(s, "stubvendorxxx"), "CL_PLATFORM_VENDOR is stubvendorxxx");
    rc = pinfo(p[0], CL_PLATFORM_VERSION, s, sizeof s);
    QT_CHECK(rc == CL_SUCCESS && !strcmp(s, "OpenCL 1.2 Stub"), "CL_PLATFORM_VERSION is OpenCL 1.2 Stub");
    rc = pinfo(p[0], CL_PLATFORM_ICD_SUFFIX_KHR, s, sizeof s);
    QT_CHECK(rc == CL_SUCCESS && !strcmp(s, "ilts"), "CL_PLATFORM_ICD_SUFFIX_KHR is ilts");
    rc = clGetPlatformInfo(p[0], CL_PLATFORM_NAME, 0, NULL, &len);
    QT_CHECK(rc == CL_SUCCESS && len == strlen(icd2 ? STUB2_NAME : STUB_NAME) + 1, "the size query gives the name's length + 1");
    rc = clGetPlatformInfo(p[0], CL_PLATFORM_NAME, 3, s, NULL);
    QT_CHECK(rc == CL_INVALID_VALUE, "a buffer too small: CL_INVALID_VALUE");
    rc = clGetDeviceIDs(p[0], CL_DEVICE_TYPE_ALL, 1, &dev, &nd);
    QT_CHECK(rc == CL_SUCCESS && nd == 1 && dev != NULL, "clGetDeviceIDs is dispatched to the driver: 1 device");
  }
  return qt_fail_count;
}

/* ---- parent ---- */
static char T[512];                       /* scratch directory */
static const char *L;                     /* installation prefix */
static char X[600];                       /* <prefix>/libexec/opencl-icd-loader */

/* Run argv with the environment minus every OpenCL loader variable, plus
   add[] ("NAME=value", NULL terminated); stdout and stderr to out when given.
   Returns the exit status, -1 when it could not run, -2 after 60 s. */
static int run(char *const argv[], const char *const add[], const char *out) {
  static const char *const drop[] = { "OCL_ICD_", "OPENCL_", "APP_LOG_FILE=", "APP_STUB_FILE=", "APP_PLATFORM=", "LD_LIBRARY_PATH=", NULL };
  char *envp[512], ldp[2048]; int n = 0, i, st; pid_t pid; const char *old = getenv("LD_LIBRARY_PATH");
  posix_spawn_file_actions_t fa; struct timespec t0, t, nap = { 0, 20 * 1000 * 1000 };
  for (char **e = environ; *e && n < 480; e++) {
    for (i = 0; drop[i] && strncmp(*e, drop[i], strlen(drop[i])); i++) ;
    if (!drop[i]) envp[n++] = *e;
  }
  /* libIcdLog.so, which the stub drivers and icd_loader_test need, is next to them */
  snprintf(ldp, sizeof ldp, "LD_LIBRARY_PATH=%s%s%s", X, old && *old ? ":" : "", old && *old ? old : "");
  envp[n++] = ldp;
  for (i = 0; add[i]; i++) envp[n++] = (char *)add[i];
  envp[n] = NULL;
  posix_spawn_file_actions_init(&fa);
  if (out) {
    posix_spawn_file_actions_addopen(&fa, 1, out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    posix_spawn_file_actions_adddup2(&fa, 1, 2);
  }
  i = posix_spawn(&pid, argv[0], &fa, NULL, argv, envp);
  posix_spawn_file_actions_destroy(&fa);
  if (i != 0) { printf("  info cannot start %s: %s\n", argv[0], strerror(i)); return -1; }
  clock_gettime(CLOCK_MONOTONIC, &t0);
  for (;;) {
    pid_t r = waitpid(pid, &st, WNOHANG);
    if (r == pid) break;
    if (r < 0 && errno != EINTR) return -1;
    clock_gettime(CLOCK_MONOTONIC, &t);
    if (t.tv_sec - t0.tv_sec >= 60) { kill(pid, SIGKILL); waitpid(pid, &st, 0); return -2; }
    nanosleep(&nap, NULL);
  }
  if (WIFEXITED(st)) return WEXITSTATUS(st);
  printf("  info %s ended by signal %d\n", argv[0], WIFSIGNALED(st) ? WTERMSIG(st) : 0);
  return -1;
}

static int file_has(const char *path, const char *needle) {
  static char buf[65536]; size_t n; FILE *f = fopen(path, "r");
  if (!f) return 0;
  n = fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = 0;
  return strstr(buf, needle) != NULL;
}
static int write_file(const char *path, const char *text) {
  FILE *f = fopen(path, "w");
  if (!f) return -1;
  fputs(text, f);
  return fclose(f);
}

int main(int argc, char **argv) {
  static char empty[600], vendors[600], icd[700], stub[700], stub2[700], layer[700], tool[700], lt[700];
  static char e_empty[700], e_vendors[700], e_files[800], e_stublog[700], e_applog[700], e_layers[800], out[600], line[800];
  const char *roots[] = { getenv("QNX_PORT_OPENCL_PREFIX"), "/usr/local", "/data/oss/usr/local" };
  char *cargv[4]; size_t i;

  if (argc == 3 && !strcmp(argv[1], "--child")) { setvbuf(stdout, NULL, _IONBF, 0); return child(argv[2]) ? 1 : 0; }

  QT_BEGIN("opencl-icd-loader");
  for (i = 0; i < sizeof roots / sizeof *roots && !L; i++) {
    if (!roots[i]) continue;
    snprintf(X, sizeof X, "%s/libexec/opencl-icd-loader", roots[i]);
    snprintf(stub, sizeof stub, "%s/libOpenCLDriverStub.so", X);
    if (access(stub, R_OK) == 0) L = roots[i];
  }
  QT_CHECK(L != NULL, "the stub drivers are installed (libexec/opencl-icd-loader)");
  if (!L) QT_END();
  snprintf(stub2, sizeof stub2, "%s/libOpenCLDriverStubICD2.so", X);
  snprintf(layer, sizeof layer, "%s/libPrintLayer.so", X);
  snprintf(lt, sizeof lt, "%s/icd_loader_test", X);
  snprintf(tool, sizeof tool, "%s/bin/cllayerinfo", L);

  snprintf(T, sizeof T, "%s/qnxport-ocl-XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  if (!mkdtemp(T)) { QT_CHECK(0, "scratch directory"); QT_END(); }
  snprintf(empty, sizeof empty, "%s/empty", T); snprintf(vendors, sizeof vendors, "%s/vendors", T);
  mkdir(empty, 0755); mkdir(vendors, 0755);
  snprintf(icd, sizeof icd, "%s/stub.icd", vendors);
  snprintf(line, sizeof line, "%s\n", stub);
  QT_CHECK(write_file(icd, line) == 0, "an ICD file naming the stub driver is written");
  snprintf(line, sizeof line, "%s/readme.txt", vendors);     /* not *.icd: must be ignored */
  write_file(line, "/nonexistent/libnothing.so\n");
  snprintf(e_empty, sizeof e_empty, "OCL_ICD_VENDORS=%s", empty);
  snprintf(e_vendors, sizeof e_vendors, "OCL_ICD_VENDORS=%s", vendors);
  snprintf(e_files, sizeof e_files, "OCL_ICD_FILENAMES=%s", stub2);
  snprintf(e_stublog, sizeof e_stublog, "APP_STUB_FILE=%s/stub.log", T);
  snprintf(e_applog, sizeof e_applog, "APP_LOG_FILE=%s/app.log", T);
  snprintf(out, sizeof out, "%s/out.txt", T);
  cargv[0] = argv[0]; cargv[1] = "--child"; cargv[3] = NULL;

  { const char *const env[] = { e_empty, NULL };
    cargv[2] = "none";
    QT_CHECK(run(cargv, env, NULL) == 0, "case: no driver"); }

  { const char *const env[] = { e_vendors, e_stublog, NULL };
    cargv[2] = "icdfile";
    QT_CHECK(run(cargv, env, NULL) == 0, "case: ICD file in the OCL_ICD_VENDORS directory");
    snprintf(line, sizeof line, "%s/stub.log", T);
    QT_CHECK(file_has(line, "clGetDeviceIDs(") && file_has(line, ", ffffffff, 1, ") && file_has(line, "Value returned: 0"),
             "the driver's log has the clGetDeviceIDs call with the arguments passed (CL_DEVICE_TYPE_ALL, 1 entry)");
    unlink(line); }

  { const char *const env[] = { e_empty, e_files, e_stublog, NULL };
    cargv[2] = "filenames";
    QT_CHECK(run(cargv, env, NULL) == 0, "case: ICD 2 driver named in OCL_ICD_FILENAMES"); }

  { const char *const env[] = { e_vendors, e_files, e_stublog, NULL };
    cargv[2] = "both";
    QT_CHECK(run(cargv, env, NULL) == 0, "case: ICD file and OCL_ICD_FILENAMES together"); }

  /* The prefix's own directory. The loader has <its prefix>/etc/OpenCL
     compiled in (/usr/local), so this needs the component installed there,
     and the right to write into it. */
  { char d0[560], d1[600], d2[640], f[700]; int made0, made1, made2;
    snprintf(d0, sizeof d0, "%.500s/etc", L); snprintf(d1, sizeof d1, "%s/OpenCL", d0); snprintf(d2, sizeof d2, "%s/vendors", d1);
    snprintf(f, sizeof f, "%s/zz-qnxport-test-stub.icd", d2);
    made0 = mkdir(d0, 0755) == 0; made1 = mkdir(d1, 0755) == 0; made2 = mkdir(d2, 0755) == 0;
    snprintf(line, sizeof line, "%s\n", stub);
    if ((!strcmp(L, "/usr/local") || getenv("QNX_PORT_OPENCL_PREFIX")) && write_file(f, line) == 0) {
      const char *const env[] = { e_stublog, NULL };
      int rc;
      cargv[2] = "prefix";
      rc = run(cargv, env, NULL);
      unlink(f);
      QT_CHECK(rc == 0, "case: ICD file in <prefix>/etc/OpenCL/vendors, no OCL_ICD_VENDORS");
    } else {
      unlink(f);
      printf("  info <prefix>/etc/OpenCL/vendors not checked: %s is not writable or not the loader's prefix\n", d2);
    }
    if (made2) rmdir(d2);
    if (made1) rmdir(d1);
    if (made0) rmdir(d0); }

  /* upstream's own test program, with each driver */
  if (access(lt, X_OK) == 0) {
    char *targv[] = { lt, NULL }; int rc;
    { char e1[800]; const char *env1[] = { e_empty, e_stublog, e_applog, e1, NULL };
      snprintf(e1, sizeof e1, "OCL_ICD_FILENAMES=%s", stub);
      rc = run(targv, env1, out);
      QT_CHECK(rc == 0 && file_has(out, "ICD Loader Test PASSED"), "upstream icd_loader_test with the stub driver: PASSED");
      if (rc != 0) printf("  info icd_loader_test exit status %d\n", rc); }
    { const char *const env2[] = { e_empty, e_stublog, e_applog, e_files, "APP_PLATFORM=" STUB2_NAME, NULL };
      rc = run(targv, env2, out);
      QT_CHECK(rc == 0 && file_has(out, "ICD Loader Test PASSED"), "upstream icd_loader_test with the ICD 2 driver: PASSED");
      if (rc != 0) printf("  info icd_loader_test (ICD 2) exit status %d\n", rc); }
  } else QT_CHECK(0, "libexec/opencl-icd-loader/icd_loader_test is installed");

  /* cllayerinfo and a layer */
  if (access(tool, X_OK) == 0) {
    char *targv[] = { tool, NULL }; int rc;
    const char *const env[] = { e_empty, e_layers, NULL };
    snprintf(e_layers, sizeof e_layers, "OPENCL_LAYERS=%s", layer);
    rc = run(targv, env, out);
    QT_CHECK(rc == 0 && file_has(out, "CL_LAYER_NAME: print_layer") && file_has(out, "CL_LAYER_API_VERSION: 100"),
             "cllayerinfo lists the test layer: print_layer, API version 100");
  } else QT_CHECK(0, "bin/cllayerinfo is installed");

  { const char *names[] = { "stub.log", "app.log", "out.txt", "vendors/stub.icd", "vendors/readme.txt" };
    for (i = 0; i < sizeof names / sizeof *names; i++) { snprintf(line, sizeof line, "%s/%s", T, names[i]); unlink(line); }
    rmdir(vendors); rmdir(empty); rmdir(T); }
  QT_END();
}
