# Lua 5.2, alongside the default Lua (5.4) and 5.1, 5.3: library liblua5.2, interpreter
# lua5.2, compiler luac5.2, headers in include/lua5.2. Version as in the QNX
# package list.
VERSION=5.2.4
TARBALL=https://www.lua.org/ftp/lua-5.2.4.tar.gz
SHA256=b9e2e4aad6789b3b63a056d442f7b39f0ecfca3ae0f1fc0ae4e9614401b69f4b
BUILDSYS=custom
# Lua ships a plain Makefile with hardcoded platform recipes and no QNX target;
# build the library and the two programs directly.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  # LUA_COMPAT_ALL: what upstream's Makefile builds 5.2 with (the 5.1
  # functions such as unpack, loadstring, math.pow, module()).
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -DLUA_USE_DLOPEN -DLUA_COMPAT_ALL -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c|print.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.2.so.0 -o liblua5.2.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.2.so.0.0.0 liblua5.2.so.0; ln -sf liblua5.2.so.0.0.0 liblua5.2.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.2.a ./*.o
  $CC $CFLAGS -o lua5.2 "$S/src/lua.c" -L. -llua5.2 -lm || return 1
  # luac uses internal symbols, so it links the static library
  $CC $CFLAGS -o luac5.2 "$S/src/luac.c" $( [ -f "$S/src/print.c" ] && echo "$S/src/print.c" ) liblua5.2.a -lm
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/include/lua5.2" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.2.so.0.0.0 "$B"/liblua5.2.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.2.so.0 "$B"/liblua5.2.so "$STAGE$PREFIX/lib/"
  install -m755 "$B"/lua5.2 "$B"/luac5.2 "$STAGE$PREFIX/bin/"
  for h in lua.h luaconf.h lualib.h lauxlib.h lua.hpp; do [ -f "$S/src/$h" ] && install -m644 "$S/src/$h" "$STAGE$PREFIX/include/lua5.2/"; done
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.2.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.2
Name: Lua
Description: Lua language engine 5.2
Version: $VERSION
Libs: -L\${libdir} -llua5.2 -lm
Cflags: -I\${includedir}
PC
}
