VERSION=22.1.8; GIT=https://github.com/llvm/llvm-project.git; REF=llvmorg-22.1.8
# LLVM/Clang/LLD running *on* QNX (a native compiler for the target), with
# qnx-ports' QNX support (patches/: the llvm-project fork's commits on
# llvmorg-22.1.8 -- QNX as a host OS, its target triples and driver). The QNX
# Everywhere 8.0.4r1 image has clang 21.1.3; qnx-ports maintains the QNX
# changes on LLVM 22 now (qnx-22.1.8_p3), so that release is used.
#
# Cross build: llvm-tblgen and clang-tblgen must run on the build host; a
# minimal native build makes them first. Options follow QNX's aports
# (core/llvm22/APKBUILD). Expect a few hours per architecture.
DEPS="zlib zstd libffi libxml2"
BUILDSYS=custom
do_configure() {
  local H="$BUILD_DIR/host/llvm-$VERSION"
  if [ ! -x "$H/bin/clang-tblgen" ]; then
    env -u CC -u CXX cmake -S "$S/llvm" -B "$H" -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=clang \
      -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF && \
      ninja -C "$H" llvm-tblgen clang-tblgen llvm-min-tblgen || return 1
  fi
  local triple=$([ "$QNX_CPU" = aarch64 ] && echo aarch64-unknown-qnx || echo x86_64-pc-qnx)
  cmake -S "$S/llvm" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_HOST_TRIPLE="$triple" \
    -DLLVM_DEFAULT_TARGET_TRIPLE="$triple" -DLLVM_TARGETS_TO_BUILD="AArch64;X86;WebAssembly" \
    -DLLVM_TABLEGEN="$H/bin/llvm-tblgen" -DCLANG_TABLEGEN="$H/bin/clang-tblgen" -DLLVM_NATIVE_TOOL_DIR="$H/bin" \
    -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_LINK_LLVM_DYLIB=ON -DLLVM_INSTALL_UTILS=ON -DLLVM_ENABLE_RTTI=ON \
    -DLLVM_ENABLE_PIC=ON -DLLVM_ENABLE_ZLIB=FORCE_ON -DLLVM_ENABLE_ZSTD=FORCE_ON -DLLVM_ENABLE_LIBEDIT=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_BUILD_DOCS=OFF -DLLVM_APPEND_VC_REV=OFF -DCMAKE_DISABLE_FIND_PACKAGE_Backtrace=ON
}
# CMake's FindBacktrace takes any <backtrace.h>; QNX's is its own bt_* API
# (libbacktrace.so), not glibc's backtrace(), so LLVM's Signals.inc failed:
# "'backtrace' was not declared". Without it LLVM prints no stack on a crash.
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
