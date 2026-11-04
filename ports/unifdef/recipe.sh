# unifdef: removes #ifdef'ed lines from C/C++ source (preprocessor
# conditionals resolved for given -D/-U symbols, the rest left alone), and
# unifdefall. Version as in QNX's own package list.
VERSION=2.12
# The release tarball: it carries version.h (the git tree generates it with
# git describe).
TARBALL=https://dotat.at/prog/unifdef/unifdef-2.12.tar.xz
SHA256=43ce0f02ecdcdc723b2475575563ddb192e988c886d368260bc0a63aee3ac400
DEPS=""
# A plain Makefile that builds in its source tree: build in a copy (src/
# stays clean for scripts/mkpatch). Its prefix variable is "prefix" and
# defaults to $HOME.
BUILDSYS=custom
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2" unifdef
}
do_install() {
  make -C "$B" prefix="$PREFIX" DESTDIR="$STAGE" install || return 1
  # unifdefall (a #!/bin/sh script) runs "cc -E -dM": it needs a C compiler,
  # which a QNX target does not have; unifdef itself needs nothing.
}
