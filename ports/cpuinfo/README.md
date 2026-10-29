# cpuinfo 0-git-20240417

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/pytorch/cpuinfo.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install cpuinfo
oss-get install cpuinfo-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH cpuinfo
ls repo/8.0/*/ | grep '^cpuinfo-'
```

This writes `cpuinfo-<ver>-r0.tar.gz` (run time) and `cpuinfo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install cpuinfo
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/cpuinfo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/cpuinfo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
