# libyuv stable

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://chromium.googlesource.com/libyuv/libyuv (tag `stable`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libyuv`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libyuv
oss-get install libyuv-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libyuv` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libjpeg-turbo
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b stable --recurse-submodules --shallow-submodules \
    https://chromium.googlesource.com/libyuv/libyuv src/libyuv
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libyuv < ports/libyuv/patches/0001-qnx-neon-asm-and-jpeg-link.patch
```

**[`0001-qnx-neon-asm-and-jpeg-link.patch`](patches/0001-qnx-neon-asm-and-jpeg-link.patch)** — libyuv: link libjpeg into the libraries; fix an aarch64 asm operand.

> With HAVE_JPEG the MJPEG decoder is compiled into libyuv itself, but only the
> yuvconvert tool linked libjpeg, leaving libyuv.so with unresolved jpeg_*
> symbols. In row_neon64.cc, ld1 takes its shuffle table through a register
> address operand: the "m" constraint gave GCC an addressing form ld1 does not
> accept.

Touches: `CMakeLists.txt`, `source/row_neon64.cc`, `source/scale_neon64.cc` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libyuv -B build/$QNX_ARCH/libyuv -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libyuv -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libyuv
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libyuv`.

### 5. What it installs

25 files under `/usr/local`: 1 shared-library entries, 1 programs, 22 headers.

```
/usr/local/lib/libyuv.so
/usr/local/bin/yuvconvert
```

The complete list is `stage/<arch>/.manifest/libyuv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libyuv
ls repo/8.0/*/ | grep '^libyuv-'
```

This writes `libyuv-<ver>-r0.tar.gz` (run time) and `libyuv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libyuv
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libyuv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libyuv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libyuv`](../../tests/src/) links libyuv the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libyuv root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libyuv'
```

```
== libyuv ==
  ok   ...
PASS libyuv
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=stable; GIT=https://chromium.googlesource.com/libyuv/libyuv; REF=stable
DEPS="libjpeg-turbo"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
