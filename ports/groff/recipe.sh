# GNU troff (needed by man-db). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.23.0
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/groff/groff-1.23.0.tar.gz
SHA256=6b9757f592b7518b4902eb6af7e54570bdccba37a871fddb2d30ae3863511c13
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-x --with-doc=no --without-gs --without-uchardet )
# The build formats groff's own documentation and examples with groff -- the
# one it has just built is a QNX binary. A distribution's groff often lacks
# the preprocessors this needs (grn, for instance), so a groff for the build
# host is built from the same source first and used for that step.
_host_env() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
_host_groff() {
  local h="$B/host-groff"
  [ -x "$h/groff" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc CXX=g++ "$S/configure" --without-x --with-doc=no \
      --without-gs --without-uchardet && _host_env make -C "$h" -j"$(nproc)"; } || return 1
}
do_build()   { _host_groff && make -C "$B" -j"$(nproc)" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff"; }
do_install() { make -C "$B" DESTDIR="$STAGE" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff" install; }
