# llvm 22.1.8

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/llvm/llvm-project.git (tag `llvmorg-22.1.8`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`zstd`](../zstd/) [`libffi`](../libffi/) [`libxml2`](../libxml2/) |
| Run-time dependencies | [`libxml2`](../libxml2/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install llvm
oss-get install llvm-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild llvm` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib zstd libffi libxml2
```

<details><summary>Full build chain (9)</summary>

[`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b llvmorg-22.1.8 --recurse-submodules --shallow-submodules \
    https://github.com/llvm/llvm-project.git src/llvm
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/llvm < ports/llvm/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — qnx: add llvm target support

> Co-authored-by: Leo Shen <yishen@qnx.com>
>
> ---
>  llvm/cmake/modules/GetHostTriple.cmake  |  6 ++++++
>  llvm/include/llvm/ADT/bit.h             |  8 ++++++++
>  llvm/include/llvm/Support/ExitCodes.h   |  2 +-
>  llvm/include/llvm/TargetParser/Triple.h |  5 ++++-
>  llvm/lib/Support/Unix/Path.inc          | 20 +++++++++++++++++++-
>  llvm/lib/Support/Unix/Threading.inc     |  6 ++++++
>  llvm/lib/TargetParser/Triple.cpp        |  5 +++--
>  7 files changed, 47 insertions(+), 5 deletions(-)

Touches: `llvm/cmake/modules/GetHostTriple.cmake`, `llvm/include/llvm/ADT/bit.h`, `llvm/include/llvm/Support/ExitCodes.h`, `llvm/include/llvm/TargetParser/Triple.h`, `llvm/lib/Support/Unix/Path.inc`, `llvm/lib/Support/Unix/Threading.inc` … · Upstream status: QNX-specific (from qnx-ports)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/llvm; B=build/$QNX_ARCH/llvm; mkdir -p $B
```

```bash
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
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- LLVM/Clang/LLD running *on* QNX (a native compiler for the target), with qnx-ports' QNX support (patches/: the llvm-project fork's commits on llvmorg-22.1.8 -- QNX as a host OS, its target triples and driver). The QNX Everywhere 8.0.4r1 image has clang 21.1.3; qnx-ports maintains the QNX changes on LLVM 22 now (qnx-22.1.8_p3), so that release is used. Cross build: llvm-tblgen and clang-tblgen must run on the build host; a minimal native build makes them first. Options follow QNX's aports (core/llvm22/APKBUILD). Expect a few hours per architecture.
- CMake's FindBacktrace takes any <backtrace.h>; QNX's is its own bt_* API (libbacktrace.so), not glibc's backtrace(), so LLVM's Signals.inc failed: "'backtrace' was not declared". Without it LLVM prints no stack on a crash.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH llvm`.

### 5. What it installs

3910 files under `/usr/local`: 12 shared-library entries, 130 programs, 3233 headers.

```
/usr/local/lib/libLLVM-22.so
/usr/local/lib/libLLVM.so
/usr/local/lib/libLLVM.so.22.1
/usr/local/lib/libLTO.so
/usr/local/lib/libLTO.so.22.1
/usr/local/lib/libRemarks.so
/usr/local/lib/libRemarks.so.22.1
/usr/local/lib/libclang-cpp.so
/usr/local/lib/libclang-cpp.so.22.1
/usr/local/lib/libclang.so
/usr/local/lib/libclang.so.22.1
/usr/local/lib/libclang.so.22.1.8
/usr/local/bin/FileCheck
/usr/local/bin/UnicodeNameMappingGenerator
/usr/local/bin/amdgpu-arch
/usr/local/bin/analyze-build
...
```

The complete list is `stage/<arch>/.manifest/llvm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH llvm
ls repo/8.0/*/ | grep '^llvm-'
```

This writes `llvm-<ver>-r0.tar.gz` (run time) and `llvm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install llvm
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/llvm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/llvm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
