# xnnpack 0-git-20250209

> Optimized neural network inference operators for ARM and x86

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/google/XNNPACK.git |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`cpuinfo`](../cpuinfo/) [`pthreadpool`](../pthreadpool/) [`fxdiv`](../fxdiv/) |
| Run-time dependencies | [`pthreadpool`](../pthreadpool/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xnnpack`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `xnnpack` (programs, shared libraries, run-time data) and `xnnpack-dev` (headers, pkg-config/CMake files, static libraries; needs `xnnpack`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xnnpack`](https://openqnx.github.io/oss/package/x86_64/xnnpack.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xnnpack.html)) · [`xnnpack-dev`](https://openqnx.github.io/oss/package/x86_64/xnnpack-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xnnpack-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xnnpack` (dependencies come along); one version: `apk add xnnpack=<version>` | `apk del xnnpack` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xnnpack` (dependencies first) | `oss-get remove xnnpack` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`xnnpack-dev` the same way, by its own name (`apk add xnnpack-dev`, `oss-get install xnnpack-dev`, by hand with its own file and list `xnnpack-dev.list`); remove it before `xnnpack`, which it needs.

```sh
# on the QNX target, as root
apk add xnnpack xnnpack-dev
apk add qnx-test-xnnpack && /usr/local/bin/qnx-port-tests/test_xnnpack    # its on-target test, from tests-8.0-<arch>-m-z
apk del xnnpack xnnpack-dev qnx-test-xnnpack
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xnnpack.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xnnpack.list
rm var/lib/manual/xnnpack.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xnnpack` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild cpuinfo pthreadpool fxdiv
```

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/google/XNNPACK.git src/xnnpack
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xnnpack < ports/xnnpack/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/xnnpack < ports/xnnpack/patches/0002-qnx-no-cpuinfo-sme.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Allow QNX build.

> Imported from qnx-ports/XNNPACK (e55b3998cadb...qnx_master); original authorship kept.
>
>
> ---
>  CMakeLists.txt | 2 +-
>  1 file changed, 1 insertion(+), 1 deletion(-)

Touches: `CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-no-cpuinfo-sme.patch`](patches/0002-qnx-no-cpuinfo-sme.patch)** — if defined(__QNX__)

Touches: `src/configs/hardware-config.c`

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/xnnpack -B build/$QNX_ARCH/xnnpack -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DXNNPACK_BUILD_TESTS=OFF \
      -DXNNPACK_BUILD_BENCHMARKS=OFF \
      -DXNNPACK_USE_SYSTEM_LIBS=ON \
      -DXNNPACK_LIBRARY_TYPE=shared \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCPUINFO_SOURCE_DIR="$STAGE$PREFIX" \
      -DPTHREADPOOL_SOURCE_DIR="$STAGE$PREFIX" \
      -DFXDIV_SOURCE_DIR="$STAGE$PREFIX" \
      -DXNNPACK_ENABLE_KLEIDIAI=OFF \
      # \
      GCC \
      12 \
      (the \
      QNX \
      8 \
      compiler) \
      does \
      not \
      know \
      these \
      newest \
      x86 \
      ISA \
      extensions. \
      -DXNNPACK_ENABLE_AVXVNNIINT8=OFF \
      -DXNNPACK_ENABLE_AVX512FP16=OFF \
      -DXNNPACK_ENABLE_AVX256VNNIGFNI=OFF \
      -DXNNPACK_ENABLE_AVX512BF16=OFF \
      -DXNNPACK_BUILD_ALL_MICROKERNELS=OFF
cmake --build build/$QNX_ARCH/xnnpack -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/xnnpack
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xnnpack`.

### 5. What it installs

4 files under `/usr/local`: 1 shared-library entries, 2 headers.

```
/usr/local/lib/libXNNPACK.so
```

The complete list is `stage/<arch>/.manifest/xnnpack` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xnnpack
ls dists/8.0/*/ | grep '^xnnpack-'
```

This writes `xnnpack-<ver>-r0.tar.gz` (run time) and `xnnpack-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xnnpack
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xnnpack-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xnnpack-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xnnpack`](../../tests/src/) links xnnpack the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xnnpack`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xnnpack
/usr/local/bin/qnx-port-tests/test_xnnpack
```

```
== xnnpack ==
  ok   ...
PASS xnnpack
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xnnpack` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xnnpack`](../../tests/src/) -- a program linked against xnnpack the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - xnn_initialize (CPU recognised)
   - add [2,3] + [3] (broadcast)
   - multiply [2,3] * [3] (broadcast)
   - create fully_connected f32
   - fully connected + bias + ReLU
   - square root unary op (within 1e-6 relative)
   - xnn_deinitialize

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xnnpack 0_git20250209-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xnnpack-0_git20250209-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xnnpack 0_git20250209-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/xnnpack-0_git20250209-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xnnpack 0_git20250209-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xnnpack-0_git20250209-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xnnpack 0_git20250209-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/xnnpack-0_git20250209-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0-git-20250209**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xnnpack=<version>`), sources in `ported/xnnpack/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xnnpack`; port one with `scripts/port-update xnnpack`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0-git-20250209; GIT=https://github.com/google/XNNPACK.git; COMMIT=e55b3998cadb320188759aaada27328cbacc3253
DEPS="cpuinfo pthreadpool fxdiv"
CMAKE_ARGS_EXTRA=( -DXNNPACK_BUILD_TESTS=OFF -DXNNPACK_BUILD_BENCHMARKS=OFF -DXNNPACK_USE_SYSTEM_LIBS=ON
                   -DXNNPACK_LIBRARY_TYPE=shared -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCPUINFO_SOURCE_DIR="$STAGE$PREFIX" -DPTHREADPOOL_SOURCE_DIR="$STAGE$PREFIX" -DFXDIV_SOURCE_DIR="$STAGE$PREFIX" -DXNNPACK_ENABLE_KLEIDIAI=OFF
                   # GCC 12 (the QNX 8 compiler) does not know these newest x86 ISA extensions.
                   -DXNNPACK_ENABLE_AVXVNNIINT8=OFF -DXNNPACK_ENABLE_AVX512FP16=OFF -DXNNPACK_ENABLE_AVX256VNNIGFNI=OFF -DXNNPACK_ENABLE_AVX512BF16=OFF -DXNNPACK_BUILD_ALL_MICROKERNELS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
