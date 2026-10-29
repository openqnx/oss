/* lua: run a script, call back into C, and read a result off the stack. */
#include "common.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
static int l_double(lua_State *L) {
  lua_pushinteger(L, 2 * luaL_checkinteger(L, 1));
  return 1;
}
int main(void) {
  QT_BEGIN("lua");
  lua_State *L = luaL_newstate();
  QT_CHECK(L != NULL, "luaL_newstate");
  luaL_openlibs(L);
  lua_pushcfunction(L, l_double);
  lua_setglobal(L, "dbl");
  QT_CHECK(luaL_dostring(L, "result = dbl(21) .. '/' .. _VERSION") == LUA_OK,
           "run script that calls a C function");
  lua_getglobal(L, "result");
  const char *s = lua_tostring(L, -1);
  QT_CHECK(s && strncmp(s, "42/Lua", 6) == 0, "C function result reached Lua");
  printf("  info %s\n", s ? s : "(null)");
  lua_pop(L, 1);
  QT_CHECK(luaL_dostring(L, "error('boom')") != LUA_OK, "script error is reported");
  /* string.format exercises Lua's own number formatting through libc. */
  QT_CHECK(luaL_dostring(L, "assert(string.format('%.3f', 1/8) == '0.125')") == LUA_OK,
           "string.format");
  lua_close(L);
  QT_END();
}
