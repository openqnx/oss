/* LuaJIT and luv.
 *
 * LuaJIT's value is the trace compiler, and a JIT is exactly the kind of thing
 * a build cannot prove: it has to map memory PROT_EXEC on the target and jump
 * into code it generated there.  So this runs a hot loop long enough to be
 * traced, asks the VM whether the JIT is on, and checks the answer is right.
 * The FFI is exercised too -- it calls straight into libc by symbol lookup.
 * luv is loaded through require() and used to run a real libuv timer. */
#include "common.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <luajit.h>

int luaopen_luv(lua_State *L);

static int run(lua_State *L, const char *code, const char *what) {
  if (luaL_dostring(L, code) != 0) {
    printf("  info %s: %s\n", what, lua_tostring(L, -1));
    lua_pop(L, 1);
    return 0;
  }
  return 1;
}

int main(void) {
  QT_BEGIN("luajit / luv");
  printf("  info %s\n", LUAJIT_VERSION);

  lua_State *L = luaL_newstate();
  QT_CHECK(L != NULL, "luaL_newstate");
  if (!L) QT_END();
  luaL_openlibs(L);

  /* jit.status() returns true when the trace compiler is enabled, followed by
     the CPU feature flags it detected. */
  QT_CHECK(run(L, "local st = jit.status(); assert(st == true, 'JIT is off')", "jit.status"),
           "trace compiler is enabled");
  run(L, "io.write('  info jit: ', jit.arch, ' ', jit.os, ' ', table.concat({select(2, jit.status())}, ' '), '\\n')",
      "jit info");
  QT_CHECK(run(L, "assert(jit.os == 'POSIX', jit.os)", "jit.os"),
           "jit.os is POSIX (lj_arch.h saw __QNX__)");

  /* A loop hot enough to be compiled; the sum is checked, so a miscompiled
     trace fails the test rather than just running slowly. */
  QT_CHECK(run(L,
    "local s = 0 "
    "for i = 1, 2000000 do s = s + (i % 7) * 3 end "
    "assert(s == 17999991, 'bad sum ' .. s)", "hot loop"),
    "hot loop computes the right answer");
  /* jit.util exposes the compiled traces; after the loop there must be one. */
  QT_CHECK(run(L,
    "local util = require('jit.util') "
    "assert(util.traceinfo(1) ~= nil, 'no trace was recorded')", "traceinfo"),
    "the hot loop was compiled into a trace");

  /* FFI: resolve and call libc functions by name. */
  QT_CHECK(run(L,
    "local ffi = require('ffi') "
    "ffi.cdef[[ size_t strlen(const char *); int getpid(void); ]] "
    "assert(tonumber(ffi.C.strlen('qnx8')) == 4) "
    "assert(ffi.C.getpid() > 0)", "ffi"),
    "ffi calls strlen and getpid in libc");

  /* luv: register as a preloaded module, then drive a real libuv timer. */
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "preload");
  lua_pushcfunction(L, luaopen_luv);
  lua_setfield(L, -2, "luv");
  lua_pop(L, 2);
  QT_CHECK(run(L, "uv = require('luv')", "require luv"), "require('luv')");
  run(L, "io.write('  info luv ', uv.version_string(), '\\n')", "luv version");
  QT_CHECK(run(L,
    "local fired = 0 "
    "local t = uv.new_timer() "
    "t:start(5, 0, function() fired = fired + 1; t:close() end) "
    "uv.run() "
    "assert(fired == 1, 'timer fired ' .. fired .. ' times')", "luv timer"),
    "a libuv timer fired through luv");
  QT_CHECK(run(L,
    "local st = assert(uv.fs_stat('/')) assert(st.type == 'directory')", "luv fs_stat"),
    "uv.fs_stat('/') reports a directory");

  lua_close(L);
  QT_END();
}
