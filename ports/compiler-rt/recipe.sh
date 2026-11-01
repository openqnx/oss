VERSION=22.1.8; GIT=https://github.com/llvm/llvm-project.git; REF=llvmorg-22.1.8
# LLVM's compiler runtime: the builtins library (libclang_rt.builtins.a --
# 128-bit arithmetic, soft-float and the rest of what code generators call
# instead of inlining) and the profiling runtime (clang -fprofile-generate /
# -fprofile-instr-generate, -fcoverage-mapping). Same llvm-project tree and
# QNX patches as the llvm component (ports/llvm: qnx-ports' fork, whose
# "qnx/compiler-rt: add qnx os support" commit is the QNX port), as a
# component of its own so that llvm is not rebuilt. Built standalone from
# compiler-rt/ with the SDP's GCC.
SRCROOT="$SRC_DIR/llvm"
# The llvm component's source tree is shared (it has these patches applied).
# Fetched here only when it is not there yet: llvm-project at the same tag
# with ports/llvm/patches -- the tree the llvm component builds from.
do_fetch() {
  git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT" || return 1
  local p; for p in "$WS"/ports/llvm/patches/*.patch; do
    echo "-- patch (llvm) $(basename "$p")"; patch -p1 -d "$SRCROOT" < "$p" || { rm -rf "$SRCROOT"; return 1; }
  done
}
SUBDIR=compiler-rt
_triple() { [ "$QNX_CPU" = aarch64 ] && echo aarch64-unknown-qnx || echo x86_64-pc-qnx; }
# Installed into the resource directory of the llvm component's clang
# (lib/clang/22/lib/<its default triple>/), where clang --rtlib=compiler-rt
# and -fprofile-instr-generate look for them on the target.
# The sanitizers, XRay, libFuzzer, memprof, ORC, ctx_profile and GWP-ASan have
# no QNX support (compiler-rt's sanitizer_common knows no QNX); only builtins
# and profile do.
# compiler-rt takes its target from CMAKE_C_COMPILER_TARGET (a triple), which
# for qcc is the -V variant (gcc_ntox86_64): a toolchain file of its own uses
# the cross GCC that qcc drives instead, for which CMake passes no target flag,
# and names the triple.
do_configure() {
  mkdir -p "$B" && cat > "$B/qnx-crt.cmake" <<TC || return 1
include("$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake")
set(CMAKE_C_COMPILER   "$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc")
set(CMAKE_CXX_COMPILER "$QNX_HOST/usr/bin/$QNX_TRIPLE-g++")
set(CMAKE_C_COMPILER_TARGET   $(_triple))
set(CMAKE_CXX_COMPILER_TARGET $(_triple))
set(CMAKE_ASM_COMPILER_TARGET $(_triple))
TC
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$B/qnx-crt.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCOMPILER_RT_INSTALL_PATH="$PREFIX/lib/clang/22" -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON \
    -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
    -DCOMPILER_RT_BUILD_BUILTINS=ON -DCOMPILER_RT_BUILD_PROFILE=ON -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
    -DCOMPILER_RT_BUILD_XRAY=OFF -DCOMPILER_RT_BUILD_LIBFUZZER=OFF -DCOMPILER_RT_BUILD_MEMPROF=OFF \
    -DCOMPILER_RT_BUILD_ORC=OFF -DCOMPILER_RT_BUILD_CTX_PROFILE=OFF -DCOMPILER_RT_BUILD_GWP_ASAN=OFF \
    -DCOMPILER_RT_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_TESTS=OFF -DPython3_EXECUTABLE=/usr/bin/python3
}
do_build() { cmake --build "$B" -j6; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
