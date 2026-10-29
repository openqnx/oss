# computelibrary 53.0.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ARM-software/ComputeLibrary.git (tag `v53.0.0`) |
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
oss-get install computelibrary
oss-get install computelibrary-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/computelibrary < ports/computelibrary/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX Support

> Imported from qnx-ports/ComputeLibrary (v53.0.0...qnx-v53.0.0); original authorship kept.
>
>
> ---
>  CMakeLists.txt | 6 ++++++
>  1 file changed, 6 insertions(+)

Touches: `CMakeLists.txt`, `CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

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
./scripts/qnxpkg -a $QNX_ARCH computelibrary
ls repo/8.0/*/ | grep '^computelibrary-'
```

This writes `computelibrary-<ver>-r0.tar.gz` (run time) and `computelibrary-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install computelibrary
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/computelibrary-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/computelibrary-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
