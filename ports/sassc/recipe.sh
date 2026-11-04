# SassC: the command-line Sass compiler over LibSass: sassc input.scss
# [output.css], with --style (nested, expanded, compact, compressed),
# --sourcemap, --load-path, -s (stdin). Version as in QNX's own package list
# (the last release).
VERSION=3.6.2
TARBALL=https://github.com/sass/sassc/archive/3.6.2.tar.gz
FETCH_NAME=sassc-3.6.2.tar.gz
SHA256=608dc9002b45a91d11ed59e352469ecc05e4f58fc1259fc9a9f5b8f0f8348a03
BUILDSYS=autotools
DEPS="libsass"
# The archive's own name is 3.6.2.tar.gz: kept in dist/ under a name of its own.
do_fetch() {
  local tb="$WS/dist/$FETCH_NAME"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" "$TARBALL" || { rm -f "$tb"; return 1; }
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "checksum mismatch: $tb"; rm -f "$tb"; return 1; }
  tar xzf "$tb" -C "$SRCROOT" --strip-components=1
}
# version.sh asks git first (the archive has no .git, so git would answer
# for the repository around it); SASSC_VERSION answers first.
export SASSC_VERSION=$VERSION
CONF_ARGS_EXTRA=( --with-libsass="$STAGE_SYSROOT$PREFIX" )
# As in libsass: the generated sassc_version.h replaces the shipped "[NA]"
# one in the source tree, which the compiler would find first.
do_build() {
  cp "$B/sassc_version.h" "$S/sassc_version.h" && make -C "$B" -j"$(nproc)"
}
