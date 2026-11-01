# computelibrary 53.0.0

> Arm Compute Library of optimized NEON/SVE kernels for Arm CPUs

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ARM-software/ComputeLibrary.git (tag `v53.0.0`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_computelibrary`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `computelibrary` and `computelibrary-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild computelibrary` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v53.0.0 --recurse-submodules --shallow-submodules \
    https://github.com/ARM-software/ComputeLibrary.git src/computelibrary
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/computelibrary < ports/computelibrary/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/computelibrary < ports/computelibrary/patches/0002-qnx-link-libregex.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX Support

> Imported from qnx-ports/ComputeLibrary (v53.0.0...qnx-v53.0.0); original authorship kept.
>
>
> ---
>  CMakeLists.txt | 6 ++++++
>  1 file changed, 6 insertions(+)

Touches: `CMakeLists.txt`, `CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-link-libregex.patch`](patches/0002-qnx-link-libregex.patch)** — Link libarm_compute against libregex on QNX.

> The CPU-info code (src/common/cpuinfo/CpuInfo.cpp) parses /proc/cpuinfo
> with POSIX regcomp/regexec. QNX keeps those in libregex.so.1, not in libc,
> so libarm_compute.so linked (undefined symbols are allowed in shared
> libraries) but recorded no NEEDED for libregex, and every program had to
> add -lregex itself.

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/computelibrary -B build/$QNX_ARCH/computelibrary -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DARM_COMPUTE_BUILD_SHARED_LIB=ON \
      -DARM_COMPUTE_ENABLE_OPENMP=OFF \
      -DARM_COMPUTE_BUILD_EXAMPLES=OFF \
      -DARM_COMPUTE_BUILD_TESTING=OFF \
      -DCMAKE_CXX_FLAGS=-std=gnu++17 \
      -isystem \
      $STAGE$PREFIX/include \
      -D_QNX_SOURCE=1
cmake --build build/$QNX_ARCH/computelibrary -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/computelibrary
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Arm Compute Library: NEON/SVE kernels for Arm CPUs -- aarch64 only.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH computelibrary`.

### 5. What it installs

2279 files under `/usr/local`: 2 shared-library entries, 2273 headers.

```
/usr/local/lib/libarm_compute.so
/usr/local/lib/libarm_compute_graph.so
```

The complete list is `stage/<arch>/.manifest/computelibrary` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa computelibrary
ls dists/8.0/*/ | grep '^computelibrary-'
```

This writes `computelibrary-<ver>-r0.tar.gz` (run time) and `computelibrary-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add computelibrary
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/computelibrary-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/computelibrary-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_computelibrary`](../../tests/src/) links computelibrary the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-computelibrary`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-computelibrary
/usr/local/bin/qnx-port-tests/test_computelibrary
```

```
== computelibrary ==
  ok   ...
PASS computelibrary
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_computelibrary` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_computelibrary`](../../tests/src/) -- a program linked against computelibrary the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - NEArithmeticAddition, 16 elements
   - NEGEMM: [[1,2,3],[4,5,6]] x [[7,8],[9,10],[11,12]]
   - NEActivationLayer ReLU
   - NEPoolingLayer 2x2 max, stride 2
   - NETranspose 2x3 -> 3x2

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | computelibrary 53.0.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/computelibrary-53.0.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | computelibrary 53.0.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/computelibrary-53.0.0-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **53.0.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add computelibrary=<version>`), sources in `ported/computelibrary/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates computelibrary`; port one with `scripts/port-update computelibrary`.

<details><summary>recipe.sh</summary>

```bash
VERSION=53.0.0; GIT=https://github.com/ARM-software/ComputeLibrary.git; REF=v53.0.0
# Arm Compute Library: NEON/SVE kernels for Arm CPUs -- aarch64 only.
ARCHES_SUPPORTED="aarch64le"
CMAKE_ARGS_EXTRA=( -DARM_COMPUTE_BUILD_SHARED_LIB=ON -DARM_COMPUTE_ENABLE_OPENMP=OFF
                   -DARM_COMPUTE_BUILD_EXAMPLES=OFF -DARM_COMPUTE_BUILD_TESTING=OFF
                   "-DCMAKE_CXX_FLAGS=-std=gnu++17 -isystem $STAGE$PREFIX/include -D_QNX_SOURCE=1" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
