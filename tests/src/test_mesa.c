#include "common.h"
#include <dlfcn.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl31.h>
#include <vulkan/vulkan.h>
/* mesa: software rendering with known answers, off-screen (EGL surfaceless
 * platform, a 64x64 pbuffer), through Mesa's own libEGL in its prefix:
 *  - OpenGL ES 3.1 on llvmpipe: glClear to (0.25, 0.5, 0.75, 1) reads back
 *    as (64, 128, 191, 255); a square from (-0.5,-0.5) to (0.5,0.5) drawn
 *    with a shader covers exactly pixels 16..47 in x and y (1024 pixels,
 *    pure red), everything else keeps the clear colour; a compute shader
 *    writes i*i+7 into a 256-entry storage buffer, read back exactly.
 *  - Desktop OpenGL 4.5+ core (4.6 on llvmpipe) through EGL (GLSL 4.50): the same draw and
 *    compute, with every entry point from eglGetProcAddress.
 *  - lavapipe (Vulkan), driven through its ICD entry point without a loader:
 *    one CPU-type physical device named "llvmpipe ...".
 * The GL work runs JIT-compiled LLVM code (llvmpipe's shaders and
 * rasterizer), so this also checks that executable memory works. */
#define W 64
#define H 64

typedef struct {
  void (*Clear)(GLbitfield); void (*ClearColor)(float, float, float, float);
  void (*Viewport)(GLint, GLint, GLsizei, GLsizei);
  void (*ReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void *);
  const GLubyte *(*GetString)(GLenum); GLenum (*GetError)(void);
  GLuint (*CreateShader)(GLenum); void (*ShaderSource)(GLuint, GLsizei, const GLchar *const *, const GLint *);
  void (*CompileShader)(GLuint); void (*GetShaderiv)(GLuint, GLenum, GLint *);
  void (*GetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *);
  GLuint (*CreateProgram)(void); void (*AttachShader)(GLuint, GLuint); void (*LinkProgram)(GLuint);
  void (*GetProgramiv)(GLuint, GLenum, GLint *); void (*UseProgram)(GLuint);
  void (*GenBuffers)(GLsizei, GLuint *); void (*BindBuffer)(GLenum, GLuint);
  void (*BufferData)(GLenum, GLsizeiptr, const void *, GLenum);
  void (*BindBufferBase)(GLenum, GLuint, GLuint);
  void (*GenVertexArrays)(GLsizei, GLuint *); void (*BindVertexArray)(GLuint);
  void (*VertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void *);
  void (*EnableVertexAttribArray)(GLuint); void (*DrawArrays)(GLenum, GLint, GLsizei);
  void (*DispatchCompute)(GLuint, GLuint, GLuint); void (*MemoryBarrier)(GLbitfield);
  void *(*MapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield); GLboolean (*UnmapBuffer)(GLenum);
} gl_t;

static int load_gl(gl_t *g) {
  static const char *names[] = { "glClear", "glClearColor", "glViewport", "glReadPixels", "glGetString",
    "glGetError", "glCreateShader", "glShaderSource", "glCompileShader", "glGetShaderiv",
    "glGetShaderInfoLog", "glCreateProgram", "glAttachShader", "glLinkProgram", "glGetProgramiv",
    "glUseProgram", "glGenBuffers", "glBindBuffer", "glBufferData", "glBindBufferBase",
    "glGenVertexArrays", "glBindVertexArray", "glVertexAttribPointer", "glEnableVertexAttribArray",
    "glDrawArrays", "glDispatchCompute", "glMemoryBarrier", "glMapBufferRange", "glUnmapBuffer" };
  void **p = (void **)g;
  for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
    if (!(p[i] = (void *)eglGetProcAddress(names[i]))) { printf("  no %s\n", names[i]); return 0; }
  return 1;
}

static GLuint shader(gl_t *g, GLenum type, const char *src) {
  GLuint s = g->CreateShader(type); GLint ok = 0;
  g->ShaderSource(s, 1, &src, NULL); g->CompileShader(s); g->GetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) { char log[1024] = ""; g->GetShaderInfoLog(s, sizeof log, NULL, log); printf("  shader: %s\n", log); return 0; }
  return s;
}
static GLuint program(gl_t *g, GLuint a, GLuint b) {
  GLuint p = g->CreateProgram(); GLint ok = 0;
  if (a) g->AttachShader(p, a);
  if (b) g->AttachShader(p, b);
  g->LinkProgram(p); g->GetProgramiv(p, GL_LINK_STATUS, &ok);
  return ok ? p : 0;
}

/* Clear, draw the square, check the pixels; run the compute shader. */
static void render_checks(gl_t *g, const char *ver, const char *tag) {
  char vs[512], fs[512], cs[512], what[160];
  snprintf(vs, sizeof vs, "%s\nlayout(location=0) in vec2 p;\nvoid main(){ gl_Position = vec4(p, 0.0, 1.0); }\n", ver);
  snprintf(fs, sizeof fs, "%s\nprecision mediump float;\nout vec4 c;\nvoid main(){ c = vec4(1.0, 0.0, 0.0, 1.0); }\n", ver);
  snprintf(cs, sizeof cs, "%s\nlayout(local_size_x=64) in;\nlayout(std430, binding=0) buffer B { uint v[]; };\n"
           "void main(){ uint i = gl_GlobalInvocationID.x; v[i] = i*i + 7u; }\n", ver);
  static unsigned char px[W * H * 4];
  g->Viewport(0, 0, W, H);
  g->ClearColor(0.25f, 0.5f, 0.75f, 1.0f); g->Clear(GL_COLOR_BUFFER_BIT);
  memset(px, 0, sizeof px);
  g->ReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px);
  int okc = 1;
  for (int i = 0; i < W * H; i++)
    if (abs(px[4*i] - 64) > 1 || abs(px[4*i+1] - 128) > 1 || abs(px[4*i+2] - 191) > 1 || px[4*i+3] != 255) { okc = 0; break; }
  snprintf(what, sizeof what, "%s: glClear(0.25,0.5,0.75,1) reads back (64,128,191,255) everywhere", tag);
  QT_CHECK(okc, what);

  GLuint prog = program(g, shader(g, GL_VERTEX_SHADER, vs), shader(g, GL_FRAGMENT_SHADER, fs));
  snprintf(what, sizeof what, "%s: vertex+fragment shaders compile and link", tag);
  QT_CHECK(prog != 0, what);
  static const float quad[] = { -0.5f,-0.5f, 0.5f,-0.5f, 0.5f,0.5f,  -0.5f,-0.5f, 0.5f,0.5f, -0.5f,0.5f };
  GLuint vao, vbo;
  g->GenVertexArrays(1, &vao); g->BindVertexArray(vao);
  g->GenBuffers(1, &vbo); g->BindBuffer(GL_ARRAY_BUFFER, vbo);
  g->BufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
  g->VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0); g->EnableVertexAttribArray(0);
  g->UseProgram(prog); g->DrawArrays(GL_TRIANGLES, 0, 6);
  g->ReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px);
  int red = 0, inside_ok = 1, outside_ok = 1;
  for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
    unsigned char *q = px + 4 * (y * W + x);
    int in = x >= 16 && x < 48 && y >= 16 && y < 48;
    int isred = q[0] == 255 && q[1] == 0 && q[2] == 0 && q[3] == 255;
    red += isred;
    if (in && !isred) inside_ok = 0;
    if (!in && (abs(q[0] - 64) > 1 || abs(q[1] - 128) > 1 || abs(q[2] - 191) > 1)) outside_ok = 0;
  }
  snprintf(what, sizeof what, "%s: square covers exactly 1024 pixels (got %d)", tag, red);
  QT_CHECK(red == 1024, what);
  snprintf(what, sizeof what, "%s: pixels 16..47 are red, the rest keep the clear colour", tag);
  QT_CHECK(inside_ok && outside_ok, what);

  GLuint cprog = program(g, shader(g, GL_COMPUTE_SHADER, cs), 0);
  snprintf(what, sizeof what, "%s: compute shader compiles and links", tag);
  QT_CHECK(cprog != 0, what);
  GLuint ssbo; g->GenBuffers(1, &ssbo); g->BindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  g->BufferData(GL_SHADER_STORAGE_BUFFER, 256 * 4, NULL, GL_DYNAMIC_READ);
  g->BindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
  g->UseProgram(cprog); g->DispatchCompute(4, 1, 1);
  g->MemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
  const uint32_t *v = g->MapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 256 * 4, GL_MAP_READ_BIT);
  int cok = v != NULL;
  for (uint32_t i = 0; cok && i < 256; i++) if (v[i] != i * i + 7) cok = 0;
  if (v) g->UnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  snprintf(what, sizeof what, "%s: compute shader wrote i*i+7 for i = 0..255", tag);
  QT_CHECK(cok, what);
  snprintf(what, sizeof what, "%s: no GL error", tag);
  QT_CHECK(g->GetError() == GL_NO_ERROR, what);
}

static void vulkan_checks(void) {
  void *h = dlopen("libvulkan_lvp.so", RTLD_NOW | RTLD_LOCAL);
  if (!h) h = dlopen("/usr/local/opt/mesa/lib/libvulkan_lvp.so", RTLD_NOW | RTLD_LOCAL);
  QT_CHECK(h != NULL, "lavapipe: libvulkan_lvp.so loads");
  if (!h) { printf("  %s\n", dlerror()); return; }
  PFN_vkGetInstanceProcAddr gipa = (PFN_vkGetInstanceProcAddr)dlsym(h, "vk_icdGetInstanceProcAddr");
  QT_CHECK(gipa != NULL, "lavapipe: vk_icdGetInstanceProcAddr exported");
  if (!gipa) return;
  PFN_vkCreateInstance ci = (PFN_vkCreateInstance)gipa(NULL, "vkCreateInstance");
  VkApplicationInfo ai = { VK_STRUCTURE_TYPE_APPLICATION_INFO, NULL, "test_mesa", 1, NULL, 0, VK_API_VERSION_1_3 };
  VkInstanceCreateInfo ici = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, NULL, 0, &ai, 0, NULL, 0, NULL };
  VkInstance inst = VK_NULL_HANDLE;
  QT_CHECK(ci && ci(&ici, NULL, &inst) == VK_SUCCESS, "lavapipe: vkCreateInstance");
  if (!inst) return;
  PFN_vkEnumeratePhysicalDevices epd = (PFN_vkEnumeratePhysicalDevices)gipa(inst, "vkEnumeratePhysicalDevices");
  PFN_vkGetPhysicalDeviceProperties gpdp = (PFN_vkGetPhysicalDeviceProperties)gipa(inst, "vkGetPhysicalDeviceProperties");
  uint32_t n = 0; VkPhysicalDevice pd[4];
  QT_CHECK(epd && epd(inst, &n, NULL) == VK_SUCCESS && n == 1, "lavapipe: one physical device");
  n = 1;
  if (epd && epd(inst, &n, pd) == VK_SUCCESS && n == 1 && gpdp) {
    VkPhysicalDeviceProperties pr; gpdp(pd[0], &pr);
    printf("  device: %s, Vulkan %u.%u\n", pr.deviceName, VK_API_VERSION_MAJOR(pr.apiVersion), VK_API_VERSION_MINOR(pr.apiVersion));
    QT_CHECK(strncmp(pr.deviceName, "llvmpipe", 8) == 0, "lavapipe: device is llvmpipe");
    QT_CHECK(pr.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU, "lavapipe: device type CPU");
    QT_CHECK(VK_API_VERSION_MINOR(pr.apiVersion) >= 3, "lavapipe: Vulkan >= 1.3");
  }
  PFN_vkDestroyInstance di = (PFN_vkDestroyInstance)gipa(inst, "vkDestroyInstance");
  if (di) di(inst, NULL);
}

static EGLContext make_ctx(EGLDisplay d, EGLenum api, EGLSurface *surf) {
  const EGLint cfg_attr[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
    EGL_RENDERABLE_TYPE, api == EGL_OPENGL_API ? EGL_OPENGL_BIT : EGL_OPENGL_ES3_BIT, EGL_NONE };
  EGLConfig cfg; EGLint n = 0;
  if (!eglBindAPI(api) || !eglChooseConfig(d, cfg_attr, &cfg, 1, &n) || n < 1) return EGL_NO_CONTEXT;
  const EGLint pb[] = { EGL_WIDTH, W, EGL_HEIGHT, H, EGL_NONE };
  *surf = eglCreatePbufferSurface(d, cfg, pb);
  const EGLint es[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE };
  const EGLint gl[] = { EGL_CONTEXT_MAJOR_VERSION, 4, EGL_CONTEXT_MINOR_VERSION, 5,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT, EGL_NONE };
  EGLContext c = eglCreateContext(d, cfg, EGL_NO_CONTEXT, api == EGL_OPENGL_API ? gl : es);
  if (c == EGL_NO_CONTEXT || *surf == EGL_NO_SURFACE || !eglMakeCurrent(d, *surf, *surf, c)) return EGL_NO_CONTEXT;
  return c;
}

int main(void) {
  QT_BEGIN("mesa");
  /* No shader cache in $HOME during the test: every run compiles. */
  setenv("MESA_SHADER_CACHE_DISABLE", "true", 0);
  const char *cexts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
  QT_CHECK(cexts && strstr(cexts, "EGL_MESA_platform_surfaceless"), "EGL client extensions list EGL_MESA_platform_surfaceless");
  PFNEGLGETPLATFORMDISPLAYEXTPROC gpd = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
  EGLDisplay d = gpd ? gpd(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, NULL) : EGL_NO_DISPLAY;
  EGLint maj = 0, min = 0;
  QT_CHECK(d != EGL_NO_DISPLAY && eglInitialize(d, &maj, &min), "surfaceless display initialises");
  if (d == EGL_NO_DISPLAY) QT_END();
  printf("  EGL %d.%d, vendor %s\n", maj, min, eglQueryString(d, EGL_VENDOR));
  QT_CHECK(maj == 1 && min >= 5, "EGL 1.5");
  QT_CHECK(strstr(eglQueryString(d, EGL_VENDOR), "Mesa") != NULL, "EGL vendor is Mesa");
  QT_CHECK(strstr(eglQueryString(d, EGL_CLIENT_APIS), "OpenGL_ES") && strstr(eglQueryString(d, EGL_CLIENT_APIS), "OpenGL "),
           "client APIs: OpenGL and OpenGL_ES");

  gl_t g;
  EGLSurface s = EGL_NO_SURFACE;
  EGLContext c = make_ctx(d, EGL_OPENGL_ES_API, &s);
  QT_CHECK(c != EGL_NO_CONTEXT, "GLES 3.1 context on a 64x64 pbuffer");
  if (c != EGL_NO_CONTEXT && load_gl(&g)) {
    printf("  GLES: %s / %s\n", g.GetString(GL_RENDERER), g.GetString(GL_VERSION));
    QT_CHECK(strstr((const char *)g.GetString(GL_RENDERER), "llvmpipe") != NULL, "GLES renderer is llvmpipe");
    QT_CHECK(strncmp((const char *)g.GetString(GL_VERSION), "OpenGL ES 3.", 12) == 0, "GL_VERSION is OpenGL ES 3.x");
    render_checks(&g, "#version 310 es", "GLES");
    eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(d, c); eglDestroySurface(d, s);
  }
  c = make_ctx(d, EGL_OPENGL_API, &s);
  QT_CHECK(c != EGL_NO_CONTEXT, "desktop OpenGL 4.5 core context on a pbuffer");
  if (c != EGL_NO_CONTEXT && load_gl(&g)) {
    printf("  GL: %s / %s\n", g.GetString(GL_RENDERER), g.GetString(GL_VERSION));
    int gmaj = 0, gmin = 0;
    sscanf((const char *)g.GetString(GL_VERSION), "%d.%d", &gmaj, &gmin);
    QT_CHECK(gmaj == 4 && gmin >= 5 && strstr((const char *)g.GetString(GL_VERSION), "Core Profile"),
             "GL_VERSION is 4.5 or later, core profile (llvmpipe: 4.6)");
    render_checks(&g, "#version 450 core", "GL");
    eglMakeCurrent(d, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(d, c); eglDestroySurface(d, s);
  }
  eglTerminate(d);
  vulkan_checks();
  QT_END();
}
