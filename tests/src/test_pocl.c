/* pocl: OpenCL on the CPU, reached the way an application reaches it --
 * through the ICD loader (libOpenCL.so.1) and pocl's ICD file -- with the
 * kernels compiled on the target at run time by the Clang/LLVM inside
 * libpocl and run on the CPU device. Every result is compared with the
 * value computed on the host side of the test, in integers or in floats
 * that are exact.
 *  - the platform "Portable Computing Language" is found, it has a CPU
 *    device (CL_DEVICE_TYPE_CPU, at least one compute unit, an "OpenCL C"
 *    version string) whose compiler is available;
 *  - clBuildProgram compiles a source with four kernels; a source with an
 *    undeclared identifier fails with CL_BUILD_PROGRAM_FAILURE and a build
 *    log naming it; a kernel name that does not exist is
 *    CL_INVALID_KERNEL_NAME;
 *  - saxpy over 4096 floats (y = a*x + y, small integers: exact);
 *  - an integer kernel over 10000 work items using get_global_id():
 *    out[i] = i*i + 7 (mod 2^32), and the sum of the results;
 *  - built-in functions: sqrt() of perfect squares, native vectors
 *    (dot() of float4), popcount() and rotate() on uint;
 *  - a 2-dimensional NDRange (64 x 48) with a global offset: every work item
 *    writes its own coordinates, so each item ran exactly once;
 *  - an explicit local work size (16), with get_local_id()/get_group_id()
 *    recombined to the global id;
 *  - clEnqueueCopyBuffer, clEnqueueFillBuffer and a mapped buffer
 *    (clEnqueueMapBuffer) carry the data they should;
 *  - the program binary can be fetched (clGetProgramInfo) and a program
 *    created from it (clCreateProgramWithBinary) builds and gives the same
 *    result without the source.
 * The kernel cache goes to a scratch directory (POCL_CACHE_DIR), removed at
 * the end. A watchdog ends the test after 170 s. */
#define CL_TARGET_OPENCL_VERSION 300
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS      /* clCreateCommandQueue */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/stat.h>
#include <CL/cl.h>

static const char *SRC =
  "__kernel void saxpy(__global const float *x, __global float *y, float a) {\n"
  "  size_t i = get_global_id(0);\n"
  "  y[i] = a * x[i] + y[i];\n"
  "}\n"
  "__kernel void squares(__global uint *out) {\n"
  "  uint i = (uint)get_global_id(0);\n"
  "  out[i] = i * i + 7u;\n"
  "}\n"
  "__kernel void builtins(__global const float *in, __global float *root, __global float *dots,\n"
  "                       __global uint *bits) {\n"
  "  size_t i = get_global_id(0);\n"
  "  root[i] = sqrt(in[i]);\n"
  "  float4 v = (float4)(1.0f, 2.0f, 3.0f, (float)i);\n"
  "  float4 w = (float4)(4.0f, 5.0f, 6.0f, 2.0f);\n"
  "  dots[i] = dot(v, w);\n"
  "  bits[2 * i] = popcount((uint)i);\n"
  "  bits[2 * i + 1] = rotate((uint)i + 0x80000001u, 4u);\n"
  "}\n"
  "__kernel void coords(__global uint *out, uint width, uint ox, uint oy) {\n"
  "  uint x = (uint)get_global_id(0), y = (uint)get_global_id(1);\n"
  "  out[(y - oy) * width + (x - ox)] = (y << 16) | x;\n"
  "}\n"
  "__kernel void groups(__global uint *out) {\n"
  "  size_t g = get_group_id(0) * get_local_size(0) + get_local_id(0);\n"
  "  out[get_global_id(0)] = (uint)g * 3u + (uint)get_local_size(0);\n"
  "}\n";

static const char *BAD =
  "__kernel void broken(__global int *out) { out[0] = no_such_identifier; }\n";

static char cache_dir[256];
static int rm_one(const char *p, const struct stat *st, int flag, struct FTW *f) {
  (void)st; (void)flag; (void)f;
  return remove(p);
}
static void cleanup(void) {
  if (cache_dir[0]) nftw(cache_dir, rm_one, 16, FTW_DEPTH | FTW_PHYS);
}
static void on_alarm(int sig) {
  static const char m[] = "  FAIL watchdog: still running after 170 s\nFAIL pocl (watchdog)\n";
  (void)sig;
  if (write(1, m, sizeof m - 1) < 0) { }
  _exit(1);
}

#define N1 4096
#define N2 10000
#define N3 1024
#define W 64
#define H 48
static float fx[N1], fy[N1], fin[N3], froot[N3], fdots[N3];
static uint32_t u2[N2], ubits[2 * N3], ucoord[W * H], ugroup[N3];

static uint32_t popc(uint32_t v) { uint32_t n = 0; while (v) { n += v & 1; v >>= 1; } return n; }

int main(void) {
  cl_platform_id plats[16], plat = NULL; cl_device_id dev = NULL; cl_uint np = 0, nd = 0, i;
  cl_int rc; char s[1024]; size_t len = 0; cl_context ctx; cl_command_queue q; cl_program prog, bad;
  cl_kernel k; cl_mem a, b, c, d; size_t g[2], l[2], off[2]; int okv; uint64_t sum;

  QT_BEGIN("pocl");
  signal(SIGALRM, on_alarm); alarm(170);

  /* pocl's ICD file is in <prefix>/etc/OpenCL/vendors, which the loader
     reads by itself when the prefix is /usr/local. Name the directory when
     the components are installed elsewhere. */
  if (!getenv("OCL_ICD_VENDORS") && access("/usr/local/etc/OpenCL/vendors/pocl.icd", R_OK) != 0 &&
      access("/data/oss/usr/local/etc/OpenCL/vendors/pocl.icd", R_OK) == 0)
    setenv("OCL_ICD_VENDORS", "/data/oss/usr/local/etc/OpenCL/vendors", 1);
  snprintf(cache_dir, sizeof cache_dir, "%s/qnxport-pocl-XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  if (!mkdtemp(cache_dir)) { cache_dir[0] = 0; QT_CHECK(0, "scratch directory"); QT_END(); }
  atexit(cleanup);
  setenv("POCL_CACHE_DIR", cache_dir, 1);

  /* ---- platform and device ---- */
  rc = clGetPlatformIDs(16, plats, &np);
  QT_CHECK(rc == CL_SUCCESS && np >= 1, "clGetPlatformIDs finds a platform");
  if (rc != CL_SUCCESS) { printf("  info clGetPlatformIDs: %d\n", rc); QT_END(); }
  for (i = 0; i < np && i < 16 && !plat; i++)
    if (clGetPlatformInfo(plats[i], CL_PLATFORM_NAME, sizeof s, s, NULL) == CL_SUCCESS && !strcmp(s, "Portable Computing Language"))
      plat = plats[i];
  QT_CHECK(plat != NULL, "the platform \"Portable Computing Language\" is there");
  if (!plat) QT_END();
  s[0] = 0; clGetPlatformInfo(plat, CL_PLATFORM_VERSION, sizeof s, s, NULL);
  QT_CHECK(!strncmp(s, "OpenCL ", 7) && strstr(s, "PoCL") != NULL, "CL_PLATFORM_VERSION is \"OpenCL <version> PoCL ...\"");
  printf("  info %s\n", s);
  rc = clGetDeviceIDs(plat, CL_DEVICE_TYPE_CPU, 1, &dev, &nd);
  QT_CHECK(rc == CL_SUCCESS && nd >= 1 && dev != NULL, "it has a CPU device");
  if (rc != CL_SUCCESS || !dev) { printf("  info clGetDeviceIDs: %d\n", rc); QT_END(); }
  { cl_device_type t = 0; cl_uint cu = 0; cl_bool comp = CL_FALSE, avail = CL_FALSE;
    clGetDeviceInfo(dev, CL_DEVICE_TYPE, sizeof t, &t, NULL);
    clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof cu, &cu, NULL);
    clGetDeviceInfo(dev, CL_DEVICE_COMPILER_AVAILABLE, sizeof comp, &comp, NULL);
    clGetDeviceInfo(dev, CL_DEVICE_AVAILABLE, sizeof avail, &avail, NULL);
    QT_CHECK((t & CL_DEVICE_TYPE_CPU) != 0, "CL_DEVICE_TYPE has CL_DEVICE_TYPE_CPU");
    QT_CHECK(cu >= 1 && cu <= 4096, "CL_DEVICE_MAX_COMPUTE_UNITS is at least 1");
    QT_CHECK(avail == CL_TRUE && comp == CL_TRUE, "the device and its compiler are available");
    s[0] = 0; clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof s, s, NULL);
    printf("  info device \"%s\", %u compute unit(s)\n", s, (unsigned)cu);
    s[0] = 0; clGetDeviceInfo(dev, CL_DEVICE_OPENCL_C_VERSION, sizeof s, s, NULL);
    QT_CHECK(!strncmp(s, "OpenCL C ", 9), "CL_DEVICE_OPENCL_C_VERSION is \"OpenCL C <version> ...\""); }

  ctx = clCreateContext(NULL, 1, &dev, NULL, NULL, &rc);
  QT_CHECK(ctx != NULL && rc == CL_SUCCESS, "clCreateContext");
  if (!ctx) QT_END();
  q = clCreateCommandQueue(ctx, dev, 0, &rc);
  QT_CHECK(q != NULL && rc == CL_SUCCESS, "clCreateCommandQueue");
  if (!q) QT_END();

  /* ---- compiling ---- */
  bad = clCreateProgramWithSource(ctx, 1, &BAD, NULL, &rc);
  rc = clBuildProgram(bad, 1, &dev, "", NULL, NULL);
  QT_CHECK(rc == CL_BUILD_PROGRAM_FAILURE, "a source with an undeclared identifier: CL_BUILD_PROGRAM_FAILURE");
  s[0] = 0; clGetProgramBuildInfo(bad, dev, CL_PROGRAM_BUILD_LOG, sizeof s - 1, s, &len); s[sizeof s - 1] = 0;
  QT_CHECK(strstr(s, "no_such_identifier") != NULL && strstr(s, "error") != NULL, "the build log has the compiler's error naming the identifier");
  clReleaseProgram(bad);

  prog = clCreateProgramWithSource(ctx, 1, &SRC, NULL, &rc);
  QT_CHECK(prog != NULL && rc == CL_SUCCESS, "clCreateProgramWithSource");
  rc = clBuildProgram(prog, 1, &dev, "", NULL, NULL);
  QT_CHECK(rc == CL_SUCCESS, "clBuildProgram compiles five kernels");
  if (rc != CL_SUCCESS) {
    s[0] = 0; clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, sizeof s - 1, s, NULL); s[sizeof s - 1] = 0;
    printf("  info clBuildProgram: %d\n%s\n", rc, s); QT_END();
  }
  { cl_uint nk = 0; clGetProgramInfo(prog, CL_PROGRAM_NUM_KERNELS, sizeof nk, &nk, NULL);
    QT_CHECK(nk == 5, "CL_PROGRAM_NUM_KERNELS is 5"); }
  k = clCreateKernel(prog, "no_such_kernel", &rc);
  QT_CHECK(k == NULL && rc == CL_INVALID_KERNEL_NAME, "an unknown kernel name: CL_INVALID_KERNEL_NAME");

  /* ---- saxpy ---- */
  for (i = 0; i < N1; i++) { fx[i] = (float)i; fy[i] = (float)(N1 - i); }
  a = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof fx, fx, &rc);
  b = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof fy, fy, &rc);
  k = clCreateKernel(prog, "saxpy", &rc);
  QT_CHECK(k != NULL && rc == CL_SUCCESS, "clCreateKernel(saxpy)");
  if (!k) QT_END();
  { cl_float fa = 3.0f;
    rc = clSetKernelArg(k, 0, sizeof a, &a); rc |= clSetKernelArg(k, 1, sizeof b, &b); rc |= clSetKernelArg(k, 2, sizeof fa, &fa); }
  g[0] = N1;
  rc |= clEnqueueNDRangeKernel(q, k, 1, NULL, g, NULL, 0, NULL, NULL);
  rc |= clFinish(q);
  QT_CHECK(rc == CL_SUCCESS, "saxpy is enqueued and finishes");
  memset(fy, 0, sizeof fy);
  rc = clEnqueueReadBuffer(q, b, CL_TRUE, 0, sizeof fy, fy, 0, NULL, NULL);
  for (okv = 1, i = 0; i < N1; i++) if (fy[i] != 3.0f * (float)i + (float)(N1 - i)) okv = 0;
  QT_CHECK(rc == CL_SUCCESS && okv, "saxpy: y[i] = 3*i + (4096 - i) for all 4096 elements");
  QT_CHECK(fy[1000] == 6096.0f, "saxpy: y[1000] = 6096");

  /* ---- copy, fill, map ---- */
  c = clCreateBuffer(ctx, CL_MEM_READ_WRITE, sizeof fy, NULL, &rc);
  rc = clEnqueueCopyBuffer(q, b, c, 0, 0, sizeof fy, 0, NULL, NULL);
  { cl_float pat = 2.5f;
    rc |= clEnqueueFillBuffer(q, c, &pat, sizeof pat, 0, 100 * sizeof(float), 0, NULL, NULL); }
  { float *m = clEnqueueMapBuffer(q, c, CL_TRUE, CL_MAP_READ, 0, sizeof fy, 0, NULL, NULL, &rc);
    okv = m != NULL;
    for (i = 0; m && i < N1; i++) if (m[i] != (i < 100 ? 2.5f : fy[i])) okv = 0;
    QT_CHECK(rc == CL_SUCCESS && okv, "copy + fill + map: 100 elements 2.5, the rest the copied saxpy result");
    if (m) clEnqueueUnmapMemObject(q, c, m, 0, NULL, NULL);
    clFinish(q); }
  clReleaseMemObject(a); clReleaseMemObject(b); clReleaseMemObject(c); clReleaseKernel(k);

  /* ---- integer kernel ---- */
  a = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof u2, NULL, &rc);
  k = clCreateKernel(prog, "squares", &rc);
  rc = clSetKernelArg(k, 0, sizeof a, &a);
  g[0] = N2;
  rc |= clEnqueueNDRangeKernel(q, k, 1, NULL, g, NULL, 0, NULL, NULL);
  rc |= clEnqueueReadBuffer(q, a, CL_TRUE, 0, sizeof u2, u2, 0, NULL, NULL);
  for (okv = 1, sum = 0, i = 0; i < N2; i++) { if (u2[i] != i * i + 7u) okv = 0; sum += u2[i]; }
  QT_CHECK(rc == CL_SUCCESS && okv, "squares: out[i] = i*i + 7 for 10000 work items");
  QT_CHECK(sum == 333283405000ull, "squares: the sum is 333283405000");
  clReleaseMemObject(a); clReleaseKernel(k);

  /* ---- built-in functions and vectors ---- */
  for (i = 0; i < N3; i++) fin[i] = (float)i * (float)i;
  a = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof fin, fin, &rc);
  b = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof froot, NULL, &rc);
  c = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof fdots, NULL, &rc);
  d = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof ubits, NULL, &rc);
  k = clCreateKernel(prog, "builtins", &rc);
  rc = clSetKernelArg(k, 0, sizeof a, &a); rc |= clSetKernelArg(k, 1, sizeof b, &b);
  rc |= clSetKernelArg(k, 2, sizeof c, &c); rc |= clSetKernelArg(k, 3, sizeof d, &d);
  g[0] = N3;
  rc |= clEnqueueNDRangeKernel(q, k, 1, NULL, g, NULL, 0, NULL, NULL);
  rc |= clEnqueueReadBuffer(q, b, CL_TRUE, 0, sizeof froot, froot, 0, NULL, NULL);
  rc |= clEnqueueReadBuffer(q, c, CL_TRUE, 0, sizeof fdots, fdots, 0, NULL, NULL);
  rc |= clEnqueueReadBuffer(q, d, CL_TRUE, 0, sizeof ubits, ubits, 0, NULL, NULL);
  QT_CHECK(rc == CL_SUCCESS, "the builtins kernel runs");
  for (okv = 1, i = 0; i < N3; i++) if (froot[i] != (float)i) okv = 0;
  QT_CHECK(okv, "sqrt(i*i) = i for i = 0 .. 1023");
  for (okv = 1, i = 0; i < N3; i++) if (fdots[i] != 32.0f + 2.0f * (float)i) okv = 0;
  QT_CHECK(okv, "dot((1,2,3,i), (4,5,6,2)) = 32 + 2*i");
  for (okv = 1, i = 0; i < N3; i++) if (ubits[2 * i] != popc(i)) okv = 0;
  QT_CHECK(okv && ubits[2 * 1023] == 10, "popcount(i); popcount(1023) = 10");
  for (okv = 1, i = 0; i < N3; i++) { uint32_t v = i + 0x80000001u; if (ubits[2 * i + 1] != ((v << 4) | (v >> 28))) okv = 0; }
  QT_CHECK(okv && ubits[1] == 0x00000018u, "rotate(i + 0x80000001, 4); for i = 0 it is 0x18");
  clReleaseMemObject(a); clReleaseMemObject(b); clReleaseMemObject(c); clReleaseMemObject(d); clReleaseKernel(k);

  /* ---- two dimensions with an offset ---- */
  memset(ucoord, 0xff, sizeof ucoord);
  a = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof ucoord, ucoord, &rc);
  k = clCreateKernel(prog, "coords", &rc);
  { cl_uint w = W, ox = 5, oy = 9;
    rc = clSetKernelArg(k, 0, sizeof a, &a); rc |= clSetKernelArg(k, 1, sizeof w, &w);
    rc |= clSetKernelArg(k, 2, sizeof ox, &ox); rc |= clSetKernelArg(k, 3, sizeof oy, &oy); }
  g[0] = W; g[1] = H; off[0] = 5; off[1] = 9;
  rc |= clEnqueueNDRangeKernel(q, k, 2, off, g, NULL, 0, NULL, NULL);
  rc |= clEnqueueReadBuffer(q, a, CL_TRUE, 0, sizeof ucoord, ucoord, 0, NULL, NULL);
  for (okv = 1, i = 0; i < W * H; i++) if (ucoord[i] != (((i / W + 9u) << 16) | (i % W + 5u))) okv = 0;
  QT_CHECK(rc == CL_SUCCESS && okv, "64 x 48 work items with offset (5, 9): each wrote its own coordinates");
  clReleaseMemObject(a); clReleaseKernel(k);

  /* ---- explicit local size ---- */
  a = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof ugroup, NULL, &rc);
  k = clCreateKernel(prog, "groups", &rc);
  rc = clSetKernelArg(k, 0, sizeof a, &a);
  g[0] = N3; l[0] = 16;
  rc |= clEnqueueNDRangeKernel(q, k, 1, NULL, g, l, 0, NULL, NULL);
  rc |= clEnqueueReadBuffer(q, a, CL_TRUE, 0, sizeof ugroup, ugroup, 0, NULL, NULL);
  for (okv = 1, i = 0; i < N3; i++) if (ugroup[i] != i * 3u + 16u) okv = 0;
  QT_CHECK(rc == CL_SUCCESS && okv, "local size 16: group id * 16 + local id is the global id in all 64 groups");
  clReleaseMemObject(a); clReleaseKernel(k);

  /* ---- the program binary ---- */
  { size_t bsz = 0; unsigned char *bin = NULL; cl_program p2 = NULL; cl_int st = -1;
    rc = clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES, sizeof bsz, &bsz, NULL);
    QT_CHECK(rc == CL_SUCCESS && bsz > 0, "the program has a binary for the device");
    if (rc == CL_SUCCESS && bsz > 0 && (bin = malloc(bsz)) != NULL) {
      rc = clGetProgramInfo(prog, CL_PROGRAM_BINARIES, sizeof bin, &bin, NULL);
      p2 = clCreateProgramWithBinary(ctx, 1, &dev, &bsz, (const unsigned char **)&bin, &st, &rc);
      QT_CHECK(p2 != NULL && rc == CL_SUCCESS && st == CL_SUCCESS, "clCreateProgramWithBinary accepts it");
      rc = p2 ? clBuildProgram(p2, 1, &dev, "", NULL, NULL) : -1;
      QT_CHECK(rc == CL_SUCCESS, "the program from the binary builds");
      if (rc == CL_SUCCESS) {
        memset(u2, 0, sizeof u2);
        a = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, sizeof u2, NULL, &rc);
        k = clCreateKernel(p2, "squares", &rc);
        rc = clSetKernelArg(k, 0, sizeof a, &a);
        g[0] = N2;
        rc |= clEnqueueNDRangeKernel(q, k, 1, NULL, g, NULL, 0, NULL, NULL);
        rc |= clEnqueueReadBuffer(q, a, CL_TRUE, 0, sizeof u2, u2, 0, NULL, NULL);
        for (okv = 1, i = 0; i < N2; i++) if (u2[i] != i * i + 7u) okv = 0;
        QT_CHECK(rc == CL_SUCCESS && okv, "its squares kernel gives the same 10000 results");
        clReleaseMemObject(a); clReleaseKernel(k);
      }
      if (p2) clReleaseProgram(p2);
      free(bin);
    } }

  clReleaseProgram(prog); clReleaseCommandQueue(q); clReleaseContext(ctx);
  alarm(0);
  QT_END();
}
