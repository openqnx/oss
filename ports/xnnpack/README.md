# xnnpack 0-git-20250209

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/google/XNNPACK.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`cpuinfo`](../cpuinfo/) [`pthreadpool`](../pthreadpool/) [`fxdiv`](../fxdiv/) |
| Run-time dependencies | [`pthreadpool`](../pthreadpool/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install xnnpack
oss-get install xnnpack-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xnnpack < ports/xnnpack/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Allow QNX build.

> Imported from qnx-ports/XNNPACK (e55b3998cadb...qnx_master); original authorship kept.
>
>
> ---
>  CMakeLists.txt | 2 +-
>  1 file changed, 1 insertion(+), 1 deletion(-)

Touches: `CMakeLists.txt` · Upstream status: QNX-specific (carried in the qnx-ports fork)

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
./scripts/qnxpkg -a $QNX_ARCH xnnpack
ls repo/8.0/*/ | grep '^xnnpack-'
```

This writes `xnnpack-<ver>-r0.tar.gz` (run time) and `xnnpack-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install xnnpack
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/xnnpack-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/xnnpack-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
