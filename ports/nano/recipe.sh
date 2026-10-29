VERSION=8.2
TARBALL=https://www.nano-editor.org/dist/v8/nano-8.2.tar.xz
SHA256=d5ad07dd862facae03051c54c6535e54c7ed7407318783fcad1ad2d7076fffeb
# Also linked at run time (found by scripts/gen-dependency-index): zlib
# Also linked at run time (found by scripts/gen-dependency-index): file
DEPS="ncurses zlib file"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --enable-utf8 )
