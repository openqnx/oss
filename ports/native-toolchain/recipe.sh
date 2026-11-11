# native-toolchain: cc, c++, gcc and g++ on the target, and the configuration
# that makes the ported clang (llvm) a ready compiler for QNX programs -- as
# on QNX Everywhere's images, where cc is clang and gcc/g++ call it.
#
# clang needs QNX's headers, start files and link libraries; they are QNX's
# own files and no package carries them. scripts/target-sysroot copies them
# from the user's SDP to <prefix>/qnx-sysroot on the target, and clang.cfg /
# clang++.cfg here (read by clang from its own directory) point --sysroot
# there. C++ is libc++ on the SDP's libc++.so.2, the ABI of every C++
# component here, so programs built on the target link with them.
# This component is only the wrappers and the configuration (files/); the
# compiler (llvm), linker (binutils), make and pkg-config (a link to pkgconf)
# are the components below. cmake and ninja are packages of their own.
VERSION=1.0
# pkg-config: pkgconf installs only "pkgconf"; build systems call pkg-config.
DEPS="llvm binutils pkgconf make"
RUNTIME_DEPS="llvm binutils pkgconf make"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/native-toolchain/files/." "$SRCROOT/"; }
do_build() { :; }
do_install() {
  local f
  for f in cc c++ gcc g++; do install -Dm755 "$S/bin/$f" "$STAGE$PREFIX/bin/$f" || return 1; done
  ln -sf pkgconf "$STAGE$PREFIX/bin/pkg-config"
  for f in clang.cfg clang++.cfg; do install -Dm644 "$S/bin/$f" "$STAGE$PREFIX/bin/$f" || return 1; done
}
