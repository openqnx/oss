# GNU troff (needed by man-db). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.25.0
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/groff/groff-1.25.0.tar.gz
SHA256=24d103a07eb79e418963bce206c8771b649871f83bda2fd4c2e65529c1c56921
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-x --with-doc=no --without-gs --without-uchardet )
# gnulib's <locale.h> (new in 1.25) defines its own locale_t when the C
# <locale.h> has none -- QNX's has none; libc++ declares QNX's locale_t in
# <__support/qnx/xlocale.h> -- and every C++ file that reaches libc++'s
# <locale> then stops at "conflicting declaration ... locale_t". groff calls
# none of the *_l functions, so gnulib needs no locale_t of its own. The
# cross build also cannot run the localeconv probe and guesses "no", and
# gnulib's replacement returns a const struct lconv * that libc++'s
# localeconv_l fallback cannot return ("invalid conversion from 'const
# lconv*'"); QNX's localeconv is the standard one.
CONF_ENV=( gl_cv_header_locale_has_locale_t=yes gl_cv_func_localeconv_works=yes )
# The build formats groff's own documentation and examples with groff -- the
# one it has just built is a QNX binary. A distribution's groff often lacks
# the preprocessors this needs (grn, for instance), so a groff for the build
# host is built from the same source first and used for that step; since
# 1.25 that includes soelim, which makes man/groff.7.man (SOELIMBIN).
_host_env() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
_host_groff() {
  local h="$B/host-groff"
  [ -x "$h/groff" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc CXX=g++ "$S/configure" --without-x --with-doc=no \
      --without-gs --without-uchardet && _host_env make -C "$h" -j"$(nproc)"; } || return 1
}
do_build()   { _host_groff && make -C "$B" -j"$(nproc)" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff" SOELIMBIN="$B/host-groff/soelim"; }
do_install() { make -C "$B" DESTDIR="$STAGE" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff" SOELIMBIN="$B/host-groff/soelim" install; }
