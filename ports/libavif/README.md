# libavif 1.1.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/AOMediaCodec/libavif.git (tag `v1.1.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`aom`](../aom/) [`dav1d`](../dav1d/) [`libyuv`](../libyuv/) |
| Run-time dependencies | [`aom`](../aom/) [`dav1d`](../dav1d/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libavif`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libavif
oss-get install libavif-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libavif` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild aom dav1d libyuv
```

<details><summary>Full build chain (4)</summary>

[`aom`](../aom/) [`dav1d`](../dav1d/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libyuv`](../libyuv/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.1.1 --recurse-submodules --shallow-submodules \
    https://github.com/AOMediaCodec/libavif.git src/libavif
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libavif < ports/libavif/patches/0001-qnx-qcc-compiler-id.patch
```

**[`0001-qnx-qcc-compiler-id.patch`](patches/0001-qnx-qcc-compiler-id.patch)** — libavif: treat qcc as GCC.

> CMake identifies qcc, a GCC driver, as "QCC"; take the GNU flag branch.

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libavif -B build/$QNX_ARCH/libavif -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DAVIF_CODEC_AOM=SYSTEM \
      -DAVIF_CODEC_DAV1D=SYSTEM \
      -DAVIF_LIBYUV=LOCAL \
      -DAVIF_BUILD_APPS=OFF \
      -DAVIF_BUILD_TESTS=OFF \
      -DAVIF_BUILD_EXAMPLES=OFF \
      -DAVIF_ENABLE_WERROR=OFF
cmake --build build/$QNX_ARCH/libavif -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libavif
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libavif`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libavif.so
/usr/local/lib/libavif.so.16
/usr/local/lib/libavif.so.16.1.1
```

The complete list is `stage/<arch>/.manifest/libavif` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libavif
ls repo/8.0/*/ | grep '^libavif-'
```

This writes `libavif-<ver>-r0.tar.gz` (run time) and `libavif-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libavif
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libavif-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libavif-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libavif`](../../tests/src/) links libavif the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libavif root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libavif'
```

```
== libavif ==
  ok   ...
PASS libavif
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.1.1; GIT=https://github.com/AOMediaCodec/libavif.git; REF=v1.1.1
DEPS="aom dav1d libyuv"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DAVIF_CODEC_AOM=SYSTEM
                   -DAVIF_CODEC_DAV1D=SYSTEM -DAVIF_LIBYUV=LOCAL
                   -DAVIF_BUILD_APPS=OFF -DAVIF_BUILD_TESTS=OFF
                   -DAVIF_BUILD_EXAMPLES=OFF -DAVIF_ENABLE_WERROR=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
