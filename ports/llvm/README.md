# llvm 22.1.8

> LLVM compiler infrastructure with the Clang compiler and LLD linker

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/llvm/llvm-project.git (tag `llvmorg-22.1.8`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`zstd`](../zstd/) [`libffi`](../libffi/) [`libxml2`](../libxml2/) |
| Run-time dependencies | [`libxml2`](../libxml2/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_llvm`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `llvm` (programs, shared libraries, run-time data) and `llvm-dev` (headers, pkg-config/CMake files, static libraries; needs `llvm`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`llvm`](https://openqnx.github.io/oss/package/x86_64/llvm.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/llvm.html)) · [`llvm-dev`](https://openqnx.github.io/oss/package/x86_64/llvm-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/llvm-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add llvm` (dependencies come along); one version: `apk add llvm=<version>` | `apk del llvm` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install llvm` (dependencies first) | `oss-get remove llvm` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`llvm-dev` the same way, by its own name (`apk add llvm-dev`, `oss-get install llvm-dev`, by hand with its own file and list `llvm-dev.list`); remove it before `llvm`, which it needs.

```sh
# on the QNX target, as root
apk add llvm llvm-dev
apk add qnx-test-llvm && /usr/local/bin/qnx-port-tests/test_llvm    # its on-target test, from tests-8.0-<arch>
apk del llvm llvm-dev qnx-test-llvm
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/llvm.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/llvm.list
rm var/lib/manual/llvm.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

**Build the dependencies first** (16 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib zstd libffi libxml2
```

<details><summary>Full build chain (16)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa llvm
ls dists/8.0/*/ | grep '^llvm-'
```

This writes `llvm-<ver>-r0.tar.gz` (run time) and `llvm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add llvm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/llvm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/llvm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_llvm`](../../tests/src/) links llvm the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-llvm`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-llvm
/usr/local/bin/qnx-port-tests/test_llvm
```

```
== llvm ==
  ok   ...
PASS llvm
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_llvm` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_llvm`](../../tests/src/) -- a program linked against llvm the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | llvm 22.1.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/llvm-22.1.8-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | llvm 22.1.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/llvm-22.1.8-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | llvm 22.1.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/llvm-22.1.8-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | llvm 22.1.8-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/llvm-22.1.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **22.1.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add llvm=<version>`), sources in `ported/llvm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates llvm`; port one with `scripts/port-update llvm`.

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
