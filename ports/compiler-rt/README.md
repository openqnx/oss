# compiler-rt 22.1.8

> LLVM compiler runtime: builtins and profiling libraries

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/llvm/llvm-project.git (tag `llvmorg-22.1.8`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_compiler_rt`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `compiler-rt` (programs, shared libraries, run-time data) and `compiler-rt-dev` (headers, pkg-config/CMake files, static libraries; needs `compiler-rt`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`compiler-rt`](https://openqnx.github.io/oss/package/x86_64/compiler-rt.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/compiler-rt.html)) · [`compiler-rt-dev`](https://openqnx.github.io/oss/package/x86_64/compiler-rt-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/compiler-rt-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add compiler-rt` (dependencies come along); one version: `apk add compiler-rt=<version>` | `apk del compiler-rt` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install compiler-rt` (dependencies first) | `oss-get remove compiler-rt` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`compiler-rt-dev` the same way, by its own name (`apk add compiler-rt-dev`, `oss-get install compiler-rt-dev`, by hand with its own file and list `compiler-rt-dev.list`); remove it before `compiler-rt`, which it needs.

```sh
# on the QNX target, as root
apk add compiler-rt compiler-rt-dev
apk add qnx-test-compiler-rt && /usr/local/bin/qnx-port-tests/test_compiler_rt    # its on-target test, from tests-8.0-<arch>
apk del compiler-rt compiler-rt-dev qnx-test-compiler-rt
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/compiler-rt.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/compiler-rt.list
rm var/lib/manual/compiler-rt.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild compiler-rt` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 -b llvmorg-22.1.8 --recurse-submodules --shallow-submodules \
    https://github.com/llvm/llvm-project.git src/compiler-rt
```

The project lives in the `compiler-rt/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/compiler-rt/compiler-rt; B=build/$QNX_ARCH/compiler-rt; mkdir -p $B
```

```bash
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
```

```bash
do_build() { cmake --build "$B" -j6; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- LLVM's compiler runtime: the builtins library (libclang_rt.builtins.a -- 128-bit arithmetic, soft-float and the rest of what code generators call instead of inlining) and the profiling runtime (clang -fprofile-generate / -fprofile-instr-generate, -fcoverage-mapping). Same llvm-project tree and QNX patches as the llvm component (ports/llvm: qnx-ports' fork, whose "qnx/compiler-rt: add qnx os support" commit is the QNX port), as a component of its own so that llvm is not rebuilt. Built standalone from compiler-rt/ with the SDP's GCC.
- The llvm component's source tree is shared (it has these patches applied). Fetched here only when it is not there yet: llvm-project at the same tag with ports/llvm/patches -- the tree the llvm component builds from.
- Installed into the resource directory of the llvm component's clang (lib/clang/22/lib/<its default triple>/), where clang --rtlib=compiler-rt and -fprofile-instr-generate look for them on the target. The sanitizers, XRay, libFuzzer, memprof, ORC, ctx_profile and GWP-ASan have no QNX support (compiler-rt's sanitizer_common knows no QNX); only builtins and profile do. compiler-rt takes its target from CMAKE_C_COMPILER_TARGET (a triple), which for qcc is the -V variant (gcc_ntox86_64): a toolchain file of its own uses the cross GCC that qcc drives instead, for which CMake passes no target flag, and names the triple.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH compiler-rt`.

### 5. What it installs

4 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/compiler-rt` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa compiler-rt
ls dists/8.0/*/ | grep '^compiler-rt-'
```

This writes `compiler-rt-<ver>-r0.tar.gz` (run time) and `compiler-rt-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add compiler-rt
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/compiler-rt-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/compiler-rt-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_compiler_rt`](../../tests/src/) links compiler-rt the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-compiler-rt`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-compiler-rt
/usr/local/bin/qnx-port-tests/test_compiler_rt
```

```
== compiler-rt ==
  ok   ...
PASS compiler-rt
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_compiler_rt` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_compiler_rt`](../../tests/src/) -- a program linked against compiler-rt the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - __udivti3/__umodti3: q*b + r == a, r < b
   - (2^100+12345) / 1000000007 = 1267650591354675262013 r 976383630
   - (2^128-1) / 3 = 0x5555...5555
   - __divti3(-1000000, 7) = -142857 (truncates)
   - __popcountdi2: 0 -> 0, -1 -> 64, 0x00F0F0F0F0F0F0F0 -> 28
   - __clzdi2: 1 -> 63, 2^47 -> 16
   - __ctzdi2: 0x100 -> 8, 2^63 -> 63
   - __bswapsi2(0x11223344) = 0x44332211
   - __ashldi3(3, 40) = 3298534883328
   - __mulodi4(3037000499^2) = 9223372030926249001, no overflow
   - __mulodi4(3037000500^2) overflows int64
   - __muloti4(INT64_MAX^2) = 0x3FFF...FFFF_0000...0001, no overflow
   - __muloti4(2^126 * 2) overflows int128
   - __floatuntidf(2^64) = 18446744073709551616.0

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | compiler-rt 22.1.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/compiler-rt-22.1.8-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | compiler-rt 22.1.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/compiler-rt-22.1.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **22.1.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add compiler-rt=<version>`), sources in `ported/compiler-rt/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates compiler-rt`; port one with `scripts/port-update compiler-rt`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
