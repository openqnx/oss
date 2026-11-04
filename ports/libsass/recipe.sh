# LibSass: the C/C++ implementation of the Sass CSS preprocessor
# (libsass.so.1 and its C API, sass.h / sass/context.h), as used by sassc,
# GTK theme builds and language bindings. Version as in QNX's own package
# list (the last LibSass release; the project is deprecated in favour of
# Dart Sass, which needs a Dart VM).
VERSION=3.6.6
TARBALL=https://github.com/sass/libsass/archive/3.6.6.tar.gz
FETCH_NAME=libsass-3.6.6.tar.gz
SHA256=11f0bb3709a4f20285507419d7618f3877a425c0131ea8df40fe6196129df15d
BUILDSYS=autotools
# The archive's own name is 3.6.6.tar.gz: kept in dist/ under a name of its own.
do_fetch() {
  local tb="$WS/dist/$FETCH_NAME"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" "$TARBALL" || { rm -f "$tb"; return 1; }
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "checksum mismatch: $tb"; rm -f "$tb"; return 1; }
  tar xzf "$tb" -C "$SRCROOT" --strip-components=1
}
DEPS=""
# configure.ac takes the version from version.sh, which asks git first:
# the GitHub archive has no .git, so git would answer with the version of
# whatever repository the source sits in. LIBSASS_VERSION answers first.
export LIBSASS_VERSION=$VERSION
CONF_ARGS_EXTRA=( --disable-tests )
# configure writes include/sass/version.h (LIBSASS_VERSION) into the build
# tree, but the archive ships one in the source tree that says "[NA]", and
# -I$(top_srcdir)/include finds that one first (upstream builds in the
# source tree): libsass_version() answered "[NA]". The generated header
# replaces the shipped one before compiling, and is the one installed.
do_build() {
  cp "$B/include/sass/version.h" "$S/include/sass/version.h" && make -C "$B" -j"$(nproc)"
}
