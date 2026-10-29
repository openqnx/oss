# Lua 5.3, alongside the default Lua (5.4): library liblua5.3, interpreter
# lua5.3, compiler luac5.3, headers in include/lua5.3. Version as in the QNX
# Everywhere 8.0.4r1 image.
VERSION=5.3.6
TARBALL=https://www.lua.org/ftp/lua-5.3.6.tar.gz
SHA256=fc5fd69bb8736323f026672b1b7235da613d7177e72558893a0bdcd320466d60
BUILDSYS=custom
# Lua ships a plain Makefile with hardcoded platform recipes and no QNX target;
# build the library and the two programs directly.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -fPIC -D_QNX_SOURCE -DLUA_USE_POSIX -DLUA_USE_DLOPEN -I$S/src"
  for f in "$S"/src/*.c; do
    case "$(basename "$f")" in lua.c|luac.c|print.c) continue ;; esac
    $CC $CFLAGS -c "$f" -o "$(basename "${f%.c}").o" || return 1
  done
  $CC -shared -Wl,-soname,liblua5.3.so.0 -o liblua5.3.so.0.0.0 ./*.o -lm || return 1
  ln -sf liblua5.3.so.0.0.0 liblua5.3.so.0; ln -sf liblua5.3.so.0.0.0 liblua5.3.so
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs liblua5.3.a ./*.o
  $CC $CFLAGS -o lua5.3 "$S/src/lua.c" -L. -llua5.3 -lm || return 1
  # luac uses internal symbols, so it links the static library
  $CC $CFLAGS -o luac5.3 "$S/src/luac.c" $( [ -f "$S/src/print.c" ] && echo "$S/src/print.c" ) liblua5.3.a -lm
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/include/lua5.3" "$STAGE$PREFIX/lib/pkgconfig"
  install -m644 "$B"/liblua5.3.so.0.0.0 "$B"/liblua5.3.a "$STAGE$PREFIX/lib/"
  cp -a "$B"/liblua5.3.so.0 "$B"/liblua5.3.so "$STAGE$PREFIX/lib/"
  install -m755 "$B"/lua5.3 "$B"/luac5.3 "$STAGE$PREFIX/bin/"
  for h in lua.h luaconf.h lualib.h lauxlib.h lua.hpp; do [ -f "$S/src/$h" ] && install -m644 "$S/src/$h" "$STAGE$PREFIX/include/lua5.3/"; done
  cat > "$STAGE$PREFIX/lib/pkgconfig/lua5.3.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include/lua5.3
Name: Lua
Description: Lua language engine 5.3
Version: $VERSION
Libs: -L\${libdir} -llua5.3 -lm
Cflags: -I\${includedir}
PC
}
