# LPeg, parsing expression grammars for Lua, built for LuaJIT (the Lua 5.1
# ABI Neovim uses). Installs lib/lua/5.1/lpeg.so. Version as in the QNX
# Everywhere 8.0.4r1 image.
VERSION=1.1.0
TARBALL=https://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.1.0.tar.gz
SHA256=4b155d67d2246c1ffa7ad7bc466c1ea899bbc40fef0257cc9c03cecbaed4352a
DEPS="luajit"
BUILDSYS=custom
do_build() { cd "$B" && qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$STAGE$PREFIX/include/luajit-2.1" \
  -o lpeg.so "$S"/lpcap.c "$S"/lpcode.c "$S"/lpcset.c "$S"/lpprint.c "$S"/lptree.c "$S"/lpvm.c; }
do_install() { install -d "$STAGE$PREFIX/lib/lua/5.1" "$STAGE$PREFIX/share/lua/5.1" && install -m755 "$B/lpeg.so" "$STAGE$PREFIX/lib/lua/5.1/" && \
  install -m644 "$S/re.lua" "$STAGE$PREFIX/share/lua/5.1/"; }
