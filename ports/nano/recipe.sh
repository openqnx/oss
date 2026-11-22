VERSION=8.7.1
TARBALL=https://www.nano-editor.org/dist/v8/nano-8.7.1.tar.xz
SHA256=76f0dcb248f2e2f1251d4ecd20fd30fb400a360a3a37c6c340e0a52c2d1cdedf
# Also linked at run time (found by scripts/gen-dependency-index): zlib
# Also linked at run time (found by scripts/gen-dependency-index): file
DEPS="ncurses zlib file"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --enable-utf8 )
