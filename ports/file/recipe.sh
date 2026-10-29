# file(1) and libmagic. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.46
TARBALL=https://astron.com/pub/file/file-5.46.tar.gz
SHA256=c9cc77c7c560c543135edc555af609d5619dbef011997e988ce40a3d75d86088
DEPS="zlib bzip2 xz zstd"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-libseccomp )
# libmagic uses regcomp()/regexec(), which QNX keeps in libregex.
CONF_ENV=( LIBS="-lregex" )
# Cross-compiling the magic database needs a build-host file(1) of exactly
# this version; build one from the same source and put it first in PATH.
_host_env() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
do_build() {
  local h="$B/host-file"
  [ -x "$h/src/file" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc "$S/configure" --disable-shared --disable-libseccomp \
      --disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib && _host_env make -C "$h" -j"$(nproc)"; } || return 1
  PATH="$h/src:$PATH" make -C "$B" -j"$(nproc)"
}
