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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xnnpack
apk add xnnpack-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xnnpack`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xnnpack 0_git20250209-r0 | ✅ PASS | 2026-10-30 |

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
