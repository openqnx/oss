# lua-compat53: Lua 5.3's standard-library behaviour for Lua 5.1 and 5.2
# (and LuaJIT, which loads 5.1 modules): require "compat53" patches the
# global functions; require "compat53.module" returns them without touching
# the globals. The string, table, utf8 and io parts are C modules, built
# here once per Lua version:
#   share/lua/5.<n>/compat53/{init,module,file_mt}.lua
#   lib/lua/5.<n>/compat53/{string,table,utf8,io}.so
# for 5.1, 5.2 and 5.3 (on 5.3 it is a no-op, so code can require it
# unconditionally), and the C API shim c-api/compat-5.3.{h,c} (C modules
# written against the 5.3 API) under include/lua-compat53. Version as in
# QNX's own package list.
VERSION=0.14.4; GIT=https://github.com/lunarmodules/lua-compat-5.3.git; REF=v0.14.4
BUILDSYS=custom
DEPS="lua5.1 lua5.2 lua5.3"
RUNTIME_DEPS=""
LC53_LUAS="5.1 5.2 5.3"
do_configure() { :; }
do_build() {
  local v m
  for v in $LC53_LUAS; do
    mkdir -p "$B/$v"
    # As luarocks' builtin type builds the rockspec's modules: one shared
    # object per C file, unresolved Lua API symbols left to the interpreter
    # that loads it (liblua5.<n>.so is already in the process).
    for m in string:lstrlib table:ltablib utf8:lutf8lib io:liolib; do
      qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$S" -I"$STAGE_SYSROOT$PREFIX/include/lua$v" \
          -o "$B/$v/${m%%:*}.so" "$S/${m#*:}.c" || return 1
    done
  done
}
do_install() {
  local v
  for v in $LC53_LUAS; do
    install -d "$STAGE$PREFIX/lib/lua/$v/compat53" "$STAGE$PREFIX/share/lua/$v/compat53"
    install -m755 "$B/$v"/*.so "$STAGE$PREFIX/lib/lua/$v/compat53/"
    install -m644 "$S"/compat53/*.lua "$STAGE$PREFIX/share/lua/$v/compat53/"
  done
  install -d "$STAGE$PREFIX/include/lua-compat53/c-api"
  install -m644 "$S"/c-api/compat-5.3.h "$S"/c-api/compat-5.3.c "$STAGE$PREFIX/include/lua-compat53/c-api/"
}
