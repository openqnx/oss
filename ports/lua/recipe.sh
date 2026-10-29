VERSION=5.4.7
TARBALL=https://www.lua.org/ftp/lua-5.4.7.tar.gz
SHA256=9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30
BUILDSYS=custom
# Lua ships a plain Makefile with hardcoded platform recipes and no QNX target;
# build the library directly.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.4.so.0 -o liblua5.4.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.4.so.0.0.0 liblua5.4.so.0; ln -sf liblua5.4.so.0.0.0 liblua5.4.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.4.a ./*.o
}
do_install() {
  # Headers in include/lua5.4, as the lua5.1 and lua5.3 components (and
  # Debian) do: in include/ itself Lua 5.4's lua.h shadowed LuaJIT's
  # (include/luajit-2.1) for anything built against LuaJIT -- Neovim compiled
  # its Lua 5.3+ code paths and failed.
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/lua5.4" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.4.so.0.0.0 "$B"/liblua5.4.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.4.so.0 "$B"/liblua5.4.so "$STAGE$PREFIX/lib/"
  install -m644 "$S"/src/lua.h "$S"/src/luaconf.h "$S"/src/lualib.h "$S"/src/lauxlib.h "$S"/src/lua.hpp "$STAGE$PREFIX/include/lua5.4/"
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.4.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.4
Name: Lua
Description: Lua language engine
Version: $VERSION
Libs: -L\${libdir} -llua5.4 -lm
Cflags: -I\${includedir}
PC
}
