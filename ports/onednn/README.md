# onednn 3.14.1

> oneAPI Deep Neural Network Library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/uxlfoundation/oneDNN.git (tag `v3.14.1`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_onednn`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `onednn` (programs, shared libraries, run-time data) and `onednn-dev` (headers, pkg-config/CMake files, static libraries; needs `onednn`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`onednn`](https://openqnx.github.io/oss/package/x86_64/onednn.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/onednn.html)) · [`onednn-dev`](https://openqnx.github.io/oss/package/x86_64/onednn-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/onednn-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add onednn` (dependencies come along); one version: `apk add onednn=<version>` | `apk del onednn` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install onednn` (dependencies first) | `oss-get remove onednn` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`onednn-dev` the same way, by its own name (`apk add onednn-dev`, `oss-get install onednn-dev`, by hand with its own file and list `onednn-dev.list`); remove it before `onednn`, which it needs.

```sh
# on the QNX target, as root
apk add onednn onednn-dev
apk add qnx-test-onednn && /usr/local/bin/qnx-port-tests/test_onednn    # its on-target test, from tests-8.0-<arch>-m-z
apk del onednn onednn-dev qnx-test-onednn
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/onednn.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/onednn.list
rm var/lib/manual/onednn.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild onednn` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v3.14.1 --recurse-submodules --shallow-submodules \
    https://github.com/uxlfoundation/oneDNN.git src/onednn
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/onednn < ports/onednn/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/onednn < ports/onednn/patches/0002-xbyak-aarch64-qnx.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — oneDNN on QNX, from QNX's port (github.com/qnx-ports/oneDNN, branch

> qnx-openvino-2026.1.2, commit 1a761da1 "Add QNX Support"): the OpenCL GPU
> runtime loads libOpenCL.so.1 on QNX as on Linux. Not built here (no OpenCL
> in the SDP: CPU only). QNX's other change on that branch (c1599bae, "Fix
> clang build": no -D_FORTIFY_SOURCE=2 on QNX) is for its clang build and is
> left out: QNX's GCC and C library support fortified builds (libc has the
> __*_chk functions).

Touches: `src/xpu/ocl/utils.hpp` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

**[`0002-xbyak-aarch64-qnx.patch`](patches/0002-xbyak-aarch64-qnx.patch)** — Xbyak_aarch64: CPU information on QNX from the system page.

> oneDNN's Arm JIT, Xbyak_aarch64, finds out the CPU's features, core count
> and caches with Linux's getauxval(AT_HWCAP) and /sys/devices/system/cpu,
> macOS's sysctl or Windows' API, and stops with #error "Unsupported OS"
> anywhere else, so oneDNN did not build for QNX on aarch64.
>
> On QNX the kernel describes every CPU in the system page
> (<sys/syspage.h>): cpuinfo.cpu holds MIDR_EL1 (the implementer), its flags
> the features (AARCH64_CPU_FLAG_SIMD, _LSE, _SVE in <aarch64/syspage.h>),
> _syspage_ptr->num_cpu the core count, and the cacheattr entries reached
> from cpuinfo.ins_cache/data_cache the cache hierarchy. util_impl_qnx.h
> (new) reads those. Features the system page does not report (FP16, BF16,
> SME, ...) count as absent, so oneDNN uses its Advanced SIMD (and, where
> the kernel reports it, SVE) code; the SVE vector length is read with
> CNTB. When the startup code describes no caches (QEMU's virt machine),
> 64 KiB L1 caches and a shared 1 MiB L2 are assumed for oneDNN's blocking.

Touches: `third_party/xbyak_aarch64/src/util_impl.cpp`, `third_party/xbyak_aarch64/xbyak_aarch64/xbyak_aarch64_util.h`, `third_party/xbyak_aarch64/src/util_impl_qnx.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/onednn -B build/$QNX_ARCH/onednn -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DONEDNN_CPU_RUNTIME=SEQ \
      -DONEDNN_GPU_RUNTIME=NONE \
      -DONEDNN_BUILD_EXAMPLES=OFF \
      -DONEDNN_BUILD_TESTS=OFF \
      -DONEDNN_BUILD_DOC=OFF \
      -DONEDNN_BUILD_GRAPH=ON \
      -DONEDNN_LIBRARY_TYPE=SHARED \
      -DONEDNN_WERROR=OFF
cmake --build build/$QNX_ARCH/onednn -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/onednn
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- oneDNN: the UXL Foundation's (formerly Intel's) deep-learning primitives library -- convolution, matmul, pooling, normalization and the rest, JIT compiled for the CPU (Xbyak on x86_64, Xbyak_aarch64 on Arm) -- with QNX's change (patches/0001, from github.com/qnx-ports/oneDNN, branch qnx-openvino-2026.1.2) and a QNX CPU-information backend for Xbyak_aarch64 (patches/0002: features, cores and caches from the system page; without it the Arm JIT does not build for QNX).
- Sequential CPU runtime: QNX's GCC has no OpenMP runtime and there is no TBB here. No GPU runtime (no OpenCL/SYCL in the SDP), no Arm Compute Library.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH onednn`.

### 5. What it installs

73 files under `/usr/local`: 3 shared-library entries, 58 headers.

```
/usr/local/lib/libdnnl.so
/usr/local/lib/libdnnl.so.3
/usr/local/lib/libdnnl.so.3.14
```

The complete list is `stage/<arch>/.manifest/onednn` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa onednn
ls dists/8.0/*/ | grep '^onednn-'
```

This writes `onednn-<ver>-r0.tar.gz` (run time) and `onednn-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add onednn
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/onednn-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/onednn-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_onednn`](../../tests/src/) links onednn the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-onednn`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-onednn
/usr/local/bin/qnx-port-tests/test_onednn
```

```
== onednn ==
  ok   ...
PASS onednn
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_onednn` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_onednn`](../../tests/src/) -- a program linked against onednn the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version 3.14.1
   - matmul 16x24 * 24x20 + bias, ReLU = reference
   - convolution 1x4x9x9 (*) 8x4x3x3 + bias = reference
   - max pooling 2x2 = reference

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | onednn 3.14.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/onednn-3.14.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | onednn 3.14.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/onednn-3.14.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.14.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add onednn=<version>`), sources in `ported/onednn/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates onednn`; port one with `scripts/port-update onednn`.

<details><summary>recipe.sh</summary>

```bash
# oneDNN: the UXL Foundation's (formerly Intel's) deep-learning primitives
# library -- convolution, matmul, pooling, normalization and the rest, JIT
# compiled for the CPU (Xbyak on x86_64, Xbyak_aarch64 on Arm) -- with QNX's
# change (patches/0001, from github.com/qnx-ports/oneDNN, branch
# qnx-openvino-2026.1.2) and a QNX CPU-information backend for Xbyak_aarch64
# (patches/0002: features, cores and caches from the system page; without it
# the Arm JIT does not build for QNX).
VERSION=3.14.1; GIT=https://github.com/uxlfoundation/oneDNN.git; REF=v3.14.1
DEPS=""
BUILDSYS=cmake
# Sequential CPU runtime: QNX's GCC has no OpenMP runtime and there is no TBB
# here. No GPU runtime (no OpenCL/SYCL in the SDP), no Arm Compute Library.
CMAKE_ARGS_EXTRA=( -DONEDNN_CPU_RUNTIME=SEQ -DONEDNN_GPU_RUNTIME=NONE -DONEDNN_BUILD_EXAMPLES=OFF -DONEDNN_BUILD_TESTS=OFF
                   -DONEDNN_BUILD_DOC=OFF -DONEDNN_BUILD_GRAPH=ON -DONEDNN_LIBRARY_TYPE=SHARED -DONEDNN_WERROR=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
