# libwebp 1.4.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://chromium.googlesource.com/webm/libwebp (tag `v1.4.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`giflib`](../giflib/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libwebp`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libwebp
oss-get install libwebp-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libwebp` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libpng libjpeg-turbo giflib
```

<details><summary>Full build chain (4)</summary>

[`giflib`](../giflib/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.4.0 --recurse-submodules --shallow-submodules \
    https://chromium.googlesource.com/webm/libwebp src/libwebp
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libwebp -B build/$QNX_ARCH/libwebp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DWEBP_BUILD_ANIM_UTILS=OFF \
      -DWEBP_BUILD_CWEBP=OFF \
      -DWEBP_BUILD_DWEBP=OFF \
      -DWEBP_BUILD_GIF2WEBP=OFF \
      -DWEBP_BUILD_IMG2WEBP=OFF \
      -DWEBP_BUILD_VWEBP=OFF \
      -DWEBP_BUILD_WEBPINFO=OFF \
      -DWEBP_BUILD_WEBPMUX=OFF \
      -DWEBP_BUILD_EXTRAS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libwebp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libwebp
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libwebp`.

### 5. What it installs

32 files under `/usr/local`: 15 shared-library entries, 8 headers.

```
/usr/local/lib/libsharpyuv.so
/usr/local/lib/libsharpyuv.so.0
/usr/local/lib/libsharpyuv.so.0.1.0
/usr/local/lib/libwebp.so
/usr/local/lib/libwebp.so.7
/usr/local/lib/libwebp.so.7.1.9
/usr/local/lib/libwebpdecoder.so
/usr/local/lib/libwebpdecoder.so.3
/usr/local/lib/libwebpdecoder.so.3.1.9
/usr/local/lib/libwebpdemux.so
/usr/local/lib/libwebpdemux.so.2
/usr/local/lib/libwebpdemux.so.2.0.15
/usr/local/lib/libwebpmux.so
/usr/local/lib/libwebpmux.so.3
/usr/local/lib/libwebpmux.so.3.1.0
```

The complete list is `stage/<arch>/.manifest/libwebp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libwebp
ls repo/8.0/*/ | grep '^libwebp-'
```

This writes `libwebp-<ver>-r0.tar.gz` (run time) and `libwebp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libwebp
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libwebp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libwebp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libwebp`](../../tests/src/) links libwebp the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libwebp root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libwebp'
```

```
== libwebp ==
  ok   ...
PASS libwebp
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.4.0; GIT=https://chromium.googlesource.com/webm/libwebp; REF=v1.4.0
DEPS="libpng libjpeg-turbo giflib"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DWEBP_BUILD_ANIM_UTILS=OFF
                   -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF -DWEBP_BUILD_GIF2WEBP=OFF
                   -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF -DWEBP_BUILD_WEBPINFO=OFF
                   -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
