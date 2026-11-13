# pocl (Portable Computing Language): an OpenCL 3.0 implementation that runs
# on the CPU. OpenCL C kernels are compiled on the target when a program is
# built (clBuildProgram): Clang and LLVM -- the llvm component's
# libclang-cpp.so and libLLVM.so, inside libpocl -- compile the source,
# PoCL's passes turn each kernel into a loop over its work group, LLVM
# generates code for the CPU and lld (linked into libpocl) makes a shared
# object, which the driver loads and runs on one thread per CPU ("cpu"
# device, pthread driver; POCL_DEVICES=cpu-minimal selects the single
# threaded one). Compiled kernels are cached in $POCL_CACHE_DIR (default
# $HOME/.cache/pocl/kcache, /tmp/pocl/kcache without HOME).
#
# Installed as an ICD for opencl-icd-loader: libpocl.so.2 and
# etc/OpenCL/vendors/pocl.icd, which names the library without a directory,
# so the loader's dlopen() finds it wherever the package root is. Programs
# link libOpenCL.so.1 and find the platform "Portable Computing Language".
# Also poclcc (compiles a kernel file to a PoCL binary ahead of time) and
# the built-in function library the kernels are linked with
# (share/pocl/kernel-<triple>-<cpu>.bc, LLVM bitcode).
#
# 7.2 is the release for LLVM 18 to 23; the llvm component is 22.1.8.
#
# How it is built for QNX:
#  - cross build as upstream supports it: LLVM_DIR is the target's LLVM
#    (staging), WITH_LLVM_CONFIG a host LLVM of the same release whose clang
#    compiles the built-in function library for the target triple
#    (pocl_host_llvm below builds those host tools once, from the llvm
#    component's source). The target's LLVM programs are named, not searched
#    for (patches/0004): the staging tree is not inside CMAKE_SYSROOT.
#  - the target CPU is fixed: x86-64 (the SSE2 baseline) and generic
#    (ARMv8-A with NEON) -- LLC_HOST_CPU. PoCL otherwise asks llc on the
#    build machine for its CPU, and at run time LLVM for the host CPU, which
#    it cannot identify on QNX/aarch64 (no /proc/cpuinfo); kernels compiled
#    for the baseline run on every target and in the QEMU test machines.
#    POCL_LLVM_CPU_NAME is not consulted with a fixed CPU.
#  - the device's description comes from the QNX system page: number of
#    CPUs, RAM, cache size, CPU name and speed (patches/0001); hwloc is not
#    used (not ported).
#  - the compiler runs on an 8 MiB stack (patches/0002): QNX threads have
#    256-512 KiB by default, Clang and LLVM need more.
#  - <net/netbyte.h> for the byte order functions (patches/0003); the data
#    directory lookup tolerates a library name without a directory
#    (patches/0005).
#  - the drivers are linked into libpocl (ENABLE_LOADABLE_DRIVERS=OFF: one
#    library, nothing to find at run time); LLVM and Clang are the shared
#    libraries of the llvm component, lld's ELF linker is linked statically
#    (the component has no shared lld); zlib and zstd are lld's.
#  - ENABLE_POCL_BUILDING=OFF: the "run from the build tree" mode would
#    compile the build machine's source and build paths into libpocl.
#  - left out: SPIR-V input (needs the SPIRV-LLVM-Translator, not ported),
#    the optional built-in kernels on onnxruntime, OpenCV and libjpeg-turbo
#    (found in staging otherwise: undeclared dependencies), vectorised math
#    through the sleef component's library on aarch64 (PoCL would compile
#    the library's staging path into the kernel link line), the remote,
#    Vulkan, CUDA and Level Zero drivers, tests and examples (they run on the
#    build machine). PoCL's copies of the OpenCL headers are not installed:
#    opencl-headers has them.
VERSION=7.2; GIT=https://github.com/pocl/pocl.git; REF=v7.2
DEPS="llvm zlib zstd opencl-headers opencl-icd-loader"
BUILDSYS=custom
POCL_LLVM=22.1.8      # the llvm component's version: host tools and target libraries must match
# Host LLVM tools of the same release as the target's libLLVM/libclang-cpp:
# pocl's kernel library (the OpenCL C built-in functions, as LLVM bitcode)
# is compiled at build time, for the target triple, by a clang that runs on
# the build machine, and the target's LLVM reads it at run time. Built once,
# from the llvm component's (QNX-patched) source so that it knows the same
# QNX triples, with the X86 and AArch64 back ends only and without
# optimisation (-O0 halves the build time of some 3000 files; these tools
# only compile the few hundred small files of the kernel library). Without
# optimisation GCC keeps a debug-only helper (printSpillReloadChain in
# MachineCopyPropagation.cpp) that nothing calls in a release build but that
# refers to MachineInstr::dump(), which a release build does not define:
# the symbol is given the address 0 at link time (--defsym).
pocl_host_llvm() {
  local H="$BUILD_DIR/host/pocl-llvm-$POCL_LLVM" L="$SRC_DIR/llvm"
  if [ ! -x "$H/bin/clang" ] || [ ! -x "$H/bin/llvm-link" ] || [ ! -x "$H/bin/llvm-config" ]; then
    [ -f "$L/llvm/CMakeLists.txt" ] || { echo "pocl: src/llvm is missing (scripts/qnxbuild llvm fetches and patches it)"; return 1; }
    env -u CC -u CXX cmake -S "$L/llvm" -B "$H" -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=clang \
      -DCMAKE_C_FLAGS_RELEASE="-O0 -DNDEBUG" -DCMAKE_CXX_FLAGS_RELEASE="-O0 -DNDEBUG" \
      -DCMAKE_EXE_LINKER_FLAGS="-Wl,--defsym,_ZNK4llvm12MachineInstr4dumpEv=0" \
      -DLLVM_TARGETS_TO_BUILD="X86;AArch64" -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF \
      -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_TERMINFO=OFF \
      -DCLANG_ENABLE_STATIC_ANALYZER=OFF -DLLVM_APPEND_VC_REV=OFF -DLLVM_ENABLE_BINDINGS=OFF &&
    ninja -C "$H" -j4 clang clang-resource-headers opt llc llvm-as llvm-dis llvm-link llvm-config || return 1
  fi
}
do_configure() {
  pocl_host_llvm || return 1
  local cpu=x86-64; [ "$QNX_CPU" = aarch64 ] && cpu=generic
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DWITH_LLVM_CONFIG="$BUILD_DIR/host/pocl-llvm-$POCL_LLVM/bin/llvm-config" -DLLVM_DIR="$STAGE$PREFIX/lib/cmake/llvm" \
    -DPOCL_TARGET_LLVM_BINDIR="$PREFIX/bin" -DSTATIC_LLVM=OFF -DLLC_HOST_CPU="$cpu" -DHOST_CPU_CACHELINE_SIZE=64 \
    -DENABLE_HOST_CPU_DEVICES=ON -DENABLE_LOADABLE_DRIVERS=OFF -DENABLE_HWLOC=OFF -DENABLE_SPIRV=OFF -DENABLE_HOST_CPU_VECTORIZE_SLEEF=OFF -DCMAKE_DISABLE_FIND_PACKAGE_onnxruntime=ON -DCMAKE_DISABLE_FIND_PACKAGE_LIBJPEG_TURBO=ON -DCMAKE_DISABLE_FIND_PACKAGE_OpenCV=ON \
    -DENABLE_ICD=ON -DPOCL_ICD_ABSOLUTE_PATH=OFF -DPOCL_INSTALL_ICD_VENDORDIR="$PREFIX/etc/OpenCL/vendors" \
    -DINSTALL_OPENCL_HEADERS=OFF -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF -DENABLE_POCLCC=ON -DENABLE_POCL_BUILDING=OFF
}
do_build() { cmake --build "$B" -j4; }
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/pocl/LICENSE"
  install -m644 "$S/LICENSE.with.3rdparty" "$STAGE$PREFIX/share/licenses/pocl/"
}
