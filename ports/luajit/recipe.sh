# LuaJIT is a rolling release off the v2.1 branch; pin the commit that was
# built and tested here, not the moving branch head.
VERSION=2.1.c6ffc141; GIT=https://github.com/LuaJIT/LuaJIT.git; REF=v2.1
PIN=c6ffc141a8762b41703f9287d63d93622a13dd8f
BUILDSYS=custom
# Upstream already knows QNX on the target side: lj_arch.h maps __QNX__ to
# LUAJIT_OS_POSIX and lj_prng.c / lj_profile.c have QNX branches.
#
# The build runs buildvm on THIS machine to generate the target's VM, and for a
# TARGET_SYS the Makefile does not list it compiles buildvm with LUAJIT_OS=OTHER.
# That is NOT harmless (an earlier version of this comment said it was): OTHER
# disables LJ_HASPROFILE, which shrinks jit_State, which moves the dispatch
# table the assembler VM addresses global state from.  Every such access was
# 16 bytes off and LuaJIT crashed on the first line of any script.
# files/0001 makes buildvm use POSIX for TARGET_SYS=QNX.
#
# The patch lives in files/, not patches/: the driver applies patches/ right
# after cloning the moving v2.1 head, before do_patch() checks out the pin --
# so do_patch() applies it itself, to the pinned tree.
do_patch() {
  cd "$SRCROOT" || return 1
  git checkout -q -- . && git fetch -q --depth 1 origin "$PIN" && git checkout -q "$PIN" || return 1
  patch -p1 < "$RECIPE_DIR/luajit/files/0001-cross-qnx-buildvm-posix-layout.patch"
}
do_configure() { :; }
_ljmake() {
  make -C "$S" -j"$(nproc)" \
    HOST_CC="gcc" \
    CROSS="$QNX_HOST/usr/bin/$QNX_TRIPLE-" \
    TARGET_SYS=QNX \
    TARGET_FLAGS="-D_QNX_SOURCE" \
    PREFIX="$PREFIX" \
    LDCONFIG=: \
    "$@"
}
do_build()   { _ljmake clean >/dev/null 2>&1; _ljmake amalg; }
do_install() { _ljmake install DESTDIR="$STAGE"; }
