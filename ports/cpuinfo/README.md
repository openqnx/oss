# cpuinfo 0-git-20240417

> C library for detecting CPU features relevant to performance tuning

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/pytorch/cpuinfo.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cpuinfo`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add cpuinfo
apk add cpuinfo-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-cpuinfo && /usr/local/bin/qnx-port-tests/test_cpuinfo    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install cpuinfo`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cpuinfo` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/pytorch/cpuinfo.git src/cpuinfo
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cpuinfo < ports/cpuinfo/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Import QNX port

> Imported from qnx-ports/cpuinfo (3c8b1533ac03dd6531ab6e7b9245d488f13a82a5...qnx); original authorship kept.
>
>
> ---
>  CMakeLists.txt    |   18 +-
>  include/cpuinfo.h |  252 +++---
>  src/api.c         |   98 ++-
>  src/init.c        |   14 +-
>  src/qnx/api.c     | 1876 +++++++++++++++++++++++++++++++++++++++++++++
>  src/qnx/api.h     |   36 +
>  6 files changed, 2183 insertions(+), 111 deletions(-)
>  create mode 100644 src/qnx/api.c
>  create mode 100644 src/qnx/api.h

Touches: `CMakeLists.txt`, `include/cpuinfo.h`, `src/api.c`, `src/init.c`, `src/qnx/api.c`, `src/qnx/api.h` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/cpuinfo -B build/$QNX_ARCH/cpuinfo -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCPUINFO_BUILD_TOOLS=OFF \
      -DCPUINFO_BUILD_UNIT_TESTS=OFF \
      -DCPUINFO_BUILD_MOCK_TESTS=OFF \
      -DCPUINFO_BUILD_BENCHMARKS=OFF \
      -DCPUINFO_LIBRARY_TYPE=shared \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/cpuinfo -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/cpuinfo
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pytorch/cpuinfo has no releases; pinned to the commit the qnx-ports QNX branch is based on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cpuinfo`.

### 5. What it installs

6 files under `/usr/local`, 1 headers.

The complete list is `stage/<arch>/.manifest/cpuinfo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cpuinfo
ls dists/8.0/*/ | grep '^cpuinfo-'
```

This writes `cpuinfo-<ver>-r0.tar.gz` (run time) and `cpuinfo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cpuinfo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cpuinfo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cpuinfo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cpuinfo`](../../tests/src/) links cpuinfo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cpuinfo`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cpuinfo
/usr/local/bin/qnx-port-tests/test_cpuinfo
```

```
== cpuinfo ==
  ok   ...
PASS cpuinfo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cpuinfo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cpuinfo`](../../tests/src/) -- a program linked against cpuinfo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - cpuinfo_initialize
   - processor count = syspage num_cpu
   - processor count = sysconf(_SC_NPROCESSORS_ONLN)
   - 1 <= packages <= clusters <= cores <= processors
   - package 0 has processors
   - processors of all packages add up
   - every processor has a core, cluster and package
   - current processor is one of them
   - x86_64: SSE2 and RDTSC
   - aarch64: NEON (Advanced SIMD)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cpuinfo 0_git20240417-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/cpuinfo-0_git20240417-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cpuinfo 0_git20240417-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/cpuinfo-0_git20240417-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0-git-20240417**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cpuinfo=<version>`), sources in `ported/cpuinfo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cpuinfo`; port one with `scripts/port-update cpuinfo`.

<details><summary>recipe.sh</summary>

```bash
# pytorch/cpuinfo has no releases; pinned to the commit the qnx-ports QNX branch is based on.
VERSION=0-git-20240417; GIT=https://github.com/pytorch/cpuinfo.git; COMMIT=3c8b1533ac03dd6531ab6e7b9245d488f13a82a5
CMAKE_ARGS_EXTRA=( -DCPUINFO_BUILD_TOOLS=OFF -DCPUINFO_BUILD_UNIT_TESTS=OFF -DCPUINFO_BUILD_MOCK_TESTS=OFF
                   -DCPUINFO_BUILD_BENCHMARKS=OFF -DCPUINFO_LIBRARY_TYPE=shared -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
