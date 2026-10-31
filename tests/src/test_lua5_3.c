/* Lua 5.3: the C API runs scripts with known results (closures, string
 * patterns, table.sort, coroutines, pcall errors, and the 5.3 additions:
 * integers, //, bitwise operators, utf8, string.pack, goto),
 * and the lua5.3 / luac5.3 programs run a script, compile it to bytecode
 * and run the bytecode with the same output. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>

static lua_State *L;
static int c_mul(lua_State *l) {
  lua_pushinteger(l, luaL_checkinteger(l, 1) * luaL_checkinteger(l, 2));
  return 1;
}

/* Run a chunk that returns one value; its tostring() goes to out. */
static int eval(const char *code, char *out, size_t n) {
  out[0] = 0;
  int top = lua_gettop(L);
  if (luaL_loadstring(L, code) || lua_pcall(L, 0, 1, 0)) {
    const char *e = lua_tostring(L, -1);
    snprintf(out, n, "error: %s", e ? e : "(nil)");
    lua_settop(L, top);
    return 0;
  }
  lua_getglobal(L, "tostring"); lua_insert(L, -2); lua_call(L, 1, 1);
  const char *s = lua_tostring(L, -1);
  snprintf(out, n, "%s", s ? s : "(nil)");
  lua_settop(L, top);
  return 1;
}
static void expect(const char *code, const char *want, const char *what) {
  char got[512];
  eval(code, got, sizeof got);
  int ok = strcmp(got, want) == 0;
  if (!ok) printf("  info %s: got \"%s\", want \"%s\"\n", what, got, want);
  QT_CHECK(ok, what);
}
/* Run a shell command, capture stdout+stderr, return the exit status. */
static int run(const char *cmd, char *out, size_t n) {
  out[0] = 0;
  FILE *p = popen(cmd, "r");
  if (!p) return -1;
  size_t len = fread(out, 1, n - 1, p);
  out[len] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

static const char *SCRIPT =
  "local t = {}\n"
  "for i = 1, 10 do t[#t+1] = i * i end\n"
  "local s = 0 for _, v in ipairs(t) do s = s + v end\n"
  "print('sum', s)\n"
  "print(string.format('%05.2f|%x|%s', math.pi, 255, 'qnx'))\n"
  "print((string.gsub('hello world', '(%w+)', '<%1>')))\n";
static const char *SCRIPT_OUT = "sum\t385\n03.14|ff|qnx\n<hello> <world>\n";

int main(void) {
  QT_BEGIN("lua5.3");
  L = luaL_newstate();
  QT_CHECK(L != NULL, "luaL_newstate");
  if (!L) QT_END();
  luaL_openlibs(L);
  printf("  info header %s\n", LUA_RELEASE);
  expect("return _VERSION", "Lua 5.3", "_VERSION is Lua 5.3 (the 5.3 library is linked)");
  expect("local function mk() local n = 0 return function() n = n + 1 return n end end "
         "local c = mk() c() c() return c()", "3", "closures keep upvalues");
  expect("local t = {5, 3, 9, 1, 7} table.sort(t) return table.concat(t, ',')",
         "1,3,5,7,9", "table.sort / table.concat");
  expect("return (string.gsub('a1b22c333', '%d+', function(d) return '[' .. #d .. ']' end))",
         "a[1]b[2]c[3]", "string.gsub with a function");
  expect("return string.format('%d %5.1f %q', 42, 2.5, 'x\"y')", "42   2.5 \"x\\\"y\"",
         "string.format");
  expect("local co = coroutine.create(function(a) local b = coroutine.yield(a + 1) return b * 2 end) "
         "local _, x = coroutine.resume(co, 1) local _, y = coroutine.resume(co, 10) return x .. ':' .. y",
         "2:20", "coroutines yield and resume");
  expect("local ok, e = pcall(error, {code = 7}) return tostring(ok) .. e.code", "false7",
         "pcall catches error objects");
  expect("return math.type(7 // 2) .. ' ' .. 7 // 2 .. ' ' .. 7 / 2 .. ' ' .. 2^10", "integer 3 3.5 1024.0",
         "integer and float arithmetic (//, /, ^)");
  expect("return (0xF0 | 0x0F) .. ' ' .. (0xFF & 0x3C) .. ' ' .. (1 << 10) .. ' ' .. (~0)", "255 60 1024 -1",
         "bitwise operators");
  expect("return math.maxinteger .. ' ' .. math.tointeger(3.0)", "9223372036854775807 3", "64-bit integers");
  expect("return utf8.char(72, 228, 8364) .. ' ' .. utf8.len('h\xc3\xa4\xe2\x82\xac')", "H\xc3\xa4\xe2\x82\xac 3", "utf8 library");
  expect("local s = string.pack('<i4 z', 258, 'ab') local n, z = string.unpack('<i4 z', s) "
         "return #s .. ' ' .. n .. ' ' .. z .. ' ' .. s:byte(1) .. s:byte(2)", "7 258 ab 21", "string.pack / unpack");
  expect("local n = 0 for i = 1, 5 do if i == 4 then goto done end n = n + i end ::done:: return n", "6", "goto");
  expect("return select('#', table.unpack({1, 2, 3}))", "3", "table.unpack");
  expect("return load('return 6 * 7')()", "42", "load");
  /* C function registered into Lua and called back. */
  lua_register(L, "c_mul", c_mul);
  expect("return c_mul(6, 7) + 0", "42", "a C function registered with lua_register");
  expect("return #string.rep('ab', 1000)", "2000", "string.rep");
  lua_close(L);

  /* The programs. */
  char dir[] = "/tmp/qnxport-lua53.XXXXXX", cmd[1024], out[2048];
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  snprintf(cmd, sizeof cmd, "%s/t.lua", dir);
  FILE *f = fopen(cmd, "w");
  if (f) { fputs(SCRIPT, f); fclose(f); }
  snprintf(cmd, sizeof cmd, "lua5.3 %s/t.lua 2>&1", dir);
  int rc = run(cmd, out, sizeof out);
  if (rc != 0 || strcmp(out, SCRIPT_OUT)) printf("  info lua5.3 rc=%d output: %s\n", rc, out);
  QT_CHECK(rc == 0 && strcmp(out, SCRIPT_OUT) == 0, "lua5.3 runs a script with the known output");
  snprintf(cmd, sizeof cmd, "lua5.3 -e 'print(1 + 2, _VERSION)' 2>&1");
  rc = run(cmd, out, sizeof out);
  QT_CHECK(rc == 0 && strcmp(out, "3\tLua 5.3\n") == 0, "lua5.3 -e");
  snprintf(cmd, sizeof cmd, "luac5.3 -o %s/t.luac %s/t.lua 2>&1", dir, dir);
  rc = run(cmd, out, sizeof out);
  QT_CHECK(rc == 0, "luac5.3 compiles the script");
  snprintf(cmd, sizeof cmd, "%s/t.luac", dir);
  unsigned char hdr[5] = {0};
  f = fopen(cmd, "rb");
  if (f) { if (fread(hdr, 1, 5, f) != 5) hdr[0] = 0; fclose(f); }
  QT_CHECK(memcmp(hdr, "\033Lua\x53", 5) == 0, "bytecode has the Lua 5.3 header (ESC Lua 0x53)");
  snprintf(cmd, sizeof cmd, "lua5.3 %s/t.luac 2>&1", dir);
  rc = run(cmd, out, sizeof out);
  QT_CHECK(rc == 0 && strcmp(out, SCRIPT_OUT) == 0, "lua5.3 runs the bytecode with the same output");
  snprintf(cmd, sizeof cmd, "%s/bad.lua", dir);
  f = fopen(cmd, "w");
  if (f) { fputs("local x = = 1\n", f); fclose(f); }
  snprintf(cmd, sizeof cmd, "luac5.3 -p %s/bad.lua 2>&1", dir);
  rc = run(cmd, out, sizeof out);
  QT_CHECK(rc != 0 && strstr(out, "unexpected symbol") != NULL, "luac5.3 -p reports a syntax error");
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  if (system(cmd) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
