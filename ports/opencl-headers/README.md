# opencl-headers 2026.05.29

> Khronos OpenCL C API headers (CL/cl.h and extensions, OpenCL 1.0 to 3.0)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/KhronosGroup/OpenCL-Headers.git (tag `v2026.05.29`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_opencl_headers`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `opencl-headers-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`opencl-headers-dev`](https://openqnx.github.io/oss/package/x86_64/opencl-headers-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/opencl-headers-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add opencl-headers-dev` (dependencies come along); one version: `apk add opencl-headers-dev=<version>` | `apk del opencl-headers-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install opencl-headers-dev` (dependencies first) | `oss-get remove opencl-headers-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-opencl-headers && /usr/local/bin/qnx-port-tests/test_opencl_headers    # its on-target test, from tests-8.0-<arch>-m-z
apk del opencl-headers-dev qnx-test-opencl-headers
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/opencl-headers-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/opencl-headers-dev.list
rm var/lib/manual/opencl-headers-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild opencl-headers` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2026.05.29 --recurse-submodules --shallow-submodules \
    https://github.com/KhronosGroup/OpenCL-Headers.git src/opencl-headers
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/opencl-headers -B build/$QNX_ARCH/opencl-headers -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_TESTING=OFF \
      -DOPENCL_HEADERS_BUILD_TESTING=OFF \
      -DOPENCL_HEADERS_BUILD_CXX_TESTS=OFF
cmake --build build/$QNX_ARCH/opencl-headers -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/opencl-headers
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- opencl-headers: the Khronos OpenCL C API headers (CL/cl.h, CL/cl_ext.h, CL/cl_icd.h, CL/cl_layer.h ... for OpenCL 1.0 to 3.0), with their CMake package (OpenCL::Headers) and OpenCL-Headers.pc. Header only: what opencl-icd-loader (libOpenCL.so), pocl and every OpenCL program compile against. An application selects the API version it is written for with -DCL_TARGET_OPENCL_VERSION=<100..300>; without it the headers default to 3.0 and say so in a #pragma message. Nothing had to change for QNX: cl_platform.h takes its fixed-width types from <stdint.h> on every non-Windows compiler. The upstream tests are host programs and are left out.
- computelibrary bundles an older copy of these headers; its recipe leaves them out and depends on this component.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH opencl-headers`.

### 5. What it installs

22 files under `/usr/local`, 18 headers.

The complete list is `stage/<arch>/.manifest/opencl-headers` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa opencl-headers
ls dists/8.0/*/ | grep '^opencl-headers-'
```

This writes `opencl-headers-<ver>-r0.tar.gz` (run time) and `opencl-headers-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add opencl-headers
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/opencl-headers-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/opencl-headers-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_opencl_headers`](../../tests/src/) links opencl-headers the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-opencl-headers`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-opencl-headers
/usr/local/bin/qnx-port-tests/test_opencl_headers
```

```
== opencl-headers ==
  ok   ...
PASS opencl-headers
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_opencl_headers` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_opencl_headers`](../../tests/src/) -- a program linked against opencl-headers the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - half(1.0) = 0x3C00
   - half(0.5) = 0x3800
   - half(-2.0) = 0xC000
   - half(65504) = 0x7BFF, the largest finite half
   - half(0.1) = 0x2E66 to nearest, towards zero, downwards
   - half(0.1) = 0x2E67 upwards
   - half(1/3) = 0x3555 to nearest, 0x3556 upwards
   - the tie 1 + 2^-11 rounds to even (0x3C00), upwards to 0x3C01
   - overflow gives +infinity (0x7C00) to nearest and upwards
   - overflow gives the largest finite half towards zero and downwards
   - half(2^-24) = 0x0001, the smallest subnormal
   - from double: 0.1 = 0x2E66, -65520 rounds to -infinity (0xFC00)
   - half -> float: 0x3C00 = 1, 0xC000 = -2, 0xFBFF = -65504
   - half -> float: 0x3555 = 0.333251953125
   - half -> float: 0x0001 = 2^-24
   - half -> float: 0x7C00 is infinity, 0x7E00 a NaN
   - scalar types: 1, 2, 4, 8 bytes; cl_half 2
   - vector types: cl_float4 16, cl_int3 16 (as int4), cl_double8 64, cl_uchar16 16 bytes
   - cl_float4 and cl_long2 are aligned to 16 bytes
   - vector components are reachable as .s[i]
   - limits: CL_UINT_MAX, CL_LONG_MIN, CL_FLT_EPSILON = 2^-23
   - CL_TARGET_OPENCL_VERSION 300 enables CL_VERSION_1_0 ... CL_VERSION_3_0
   - CL_MAKE_VERSION(1, 2, 3) = 1<<22 | 2<<12 | 3 = 4202499
   - CL_VERSION_MAJOR/MINOR/PATCH unpack 3.0.19
   - cl_name_version: a cl_version and 64 bytes of name
   - ... and 7 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | opencl-headers 2026.05.29-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/opencl-headers-2026.05.29-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | opencl-headers 2026.05.29-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/opencl-headers-2026.05.29-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2026.05.29**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add opencl-headers=<version>`), sources in `ported/opencl-headers/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates opencl-headers`; port one with `scripts/port-update opencl-headers`.

<details><summary>recipe.sh</summary>

```bash
# opencl-headers: the Khronos OpenCL C API headers (CL/cl.h, CL/cl_ext.h,
# CL/cl_icd.h, CL/cl_layer.h ... for OpenCL 1.0 to 3.0), with their CMake
# package (OpenCL::Headers) and OpenCL-Headers.pc. Header only: what
# opencl-icd-loader (libOpenCL.so), pocl and every OpenCL program compile
# against. An application selects the API version it is written for with
# -DCL_TARGET_OPENCL_VERSION=<100..300>; without it the headers default to
# 3.0 and say so in a #pragma message.
#
# Nothing had to change for QNX: cl_platform.h takes its fixed-width types
# from <stdint.h> on every non-Windows compiler. The upstream tests are host
# programs and are left out.
VERSION=2026.05.29; GIT=https://github.com/KhronosGroup/OpenCL-Headers.git; REF=v2026.05.29
DEPS=""
# computelibrary bundles an older copy of these headers; its recipe leaves
# them out and depends on this component.
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DOPENCL_HEADERS_BUILD_TESTING=OFF -DOPENCL_HEADERS_BUILD_CXX_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
