# http-parser: Node.js's original HTTP request/response parser in C (a
# callback parser that neither allocates nor copies; also a URL parser).
# Deprecated upstream in favour of llhttp; QNX ships it.
VERSION=2.9.4; GIT=https://github.com/nodejs/http-parser.git; REF=v2.9.4
BUILDSYS=custom
# A plain Makefile that builds in the source directory: copy the tree to the
# build directory and build there. PLATFORM is set rather than taken from
# the build machine's uname (only darwin/wine differ). The shared library
# keeps upstream's Linux soname, libhttp_parser.so.2.9; built with
# HTTP_PARSER_STRICT=0 as upstream's "library" target does.
hp_make() {
  make -C "$B" PLATFORM=qnx CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" AR="$QNX_HOST/usr/bin/$QNX_TRIPLE-ar" \
    LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state" PREFIX="$PREFIX" "$@"
}
do_configure() { cp -a "$S/." "$B/"; }
do_build() { hp_make library; }
do_install() { hp_make DESTDIR="$STAGE" install; }
