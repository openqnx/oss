# scdoc: a small man page generator -- turns scdoc's markdown-like syntax
# into roff man pages (the manuals of sway, wlroots tools, aerc, ...).
# Version as in QNX's own package list.
VERSION=1.11.3
TARBALL=https://git.sr.ht/~sircmpwn/scdoc/archive/1.11.3.tar.gz
SHA256=4c5c6136540384e5455b250f768e7ca11b03fdba1a8efc2341ee0f1111e57612
DEPS=""
# A plain Makefile that builds in its source tree: build in copies (src/
# stays clean). The build generates scdoc's own manuals by running scdoc
# (HOST_SCDOC): that is a scdoc built for the build machine, from the same
# source, first. LDFLAGS is replaced: the Makefile links -static, and the
# target build is an ordinary dynamic QNX program like every other.
BUILDSYS=custom
do_build() {
  rm -rf "$B" && mkdir -p "$B/host" "$B/qnx" && cp -a "$S/." "$B/host/" && cp -a "$S/." "$B/qnx/" || return 1
  make -C "$B/host" CC=gcc LDFLAGS= scdoc || return 1
  make -C "$B/qnx" CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS=-O2 LDFLAGS= HOST_SCDOC="$B/host/scdoc" PREFIX="$PREFIX"
}
do_install() { make -C "$B/qnx" HOST_SCDOC="$B/host/scdoc" PREFIX="$PREFIX" DESTDIR="$STAGE" install; }
