# libjxl 0.11.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libjxl/libjxl.git (tag `v0.11.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`brotli`](../brotli/) [`highway`](../highway/) [`lcms2`](../lcms2/) [`libpng`](../libpng/) [`giflib`](../giflib/) [`libjpeg-turbo`](../libjpeg-turbo/) |
| Run-time dependencies | [`brotli`](../brotli/) [`highway`](../highway/) [`lcms2`](../lcms2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libjxl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libjxl
oss-get install libjxl-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libjxl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (7 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild brotli highway lcms2 libpng giflib libjpeg-turbo
```

<details><summary>Full build chain (7)</summary>

[`brotli`](../brotli/) [`giflib`](../giflib/) [`highway`](../highway/) [`lcms2`](../lcms2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.11.1 --recurse-submodules --shallow-submodules \
    https://github.com/libjxl/libjxl.git src/libjxl
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libjxl -B build/$QNX_ARCH/libjxl -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DJPEGXL_ENABLE_TOOLS=OFF \
      -DJPEGXL_ENABLE_EXAMPLES=OFF \
      -DJPEGXL_ENABLE_BENCHMARK=OFF \
      -DJPEGXL_ENABLE_FUZZERS=OFF \
      -DJPEGXL_ENABLE_DOXYGEN=OFF \
      -DJPEGXL_ENABLE_MANPAGES=OFF \
      -DJPEGXL_ENABLE_JNI=OFF \
      -DJPEGXL_ENABLE_SJPEG=OFF \
      -DJPEGXL_ENABLE_OPENEXR=OFF \
      -DJPEGXL_ENABLE_SKCMS=OFF \
      -DJPEGXL_FORCE_SYSTEM_BROTLI=ON \
      -DJPEGXL_FORCE_SYSTEM_HWY=ON \
      -DJPEGXL_FORCE_SYSTEM_LCMS2=ON \
      -DJPEGXL_BUNDLE_LIBPNG=OFF
cmake --build build/$QNX_ARCH/libjxl -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libjxl
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libjxl`.

### 5. What it installs

34 files under `/usr/local`: 9 shared-library entries, 22 headers.

```
/usr/local/lib/libjxl.so
/usr/local/lib/libjxl.so.0.11
/usr/local/lib/libjxl.so.0.11.1
/usr/local/lib/libjxl_cms.so
/usr/local/lib/libjxl_cms.so.0.11
/usr/local/lib/libjxl_cms.so.0.11.1
/usr/local/lib/libjxl_threads.so
/usr/local/lib/libjxl_threads.so.0.11
/usr/local/lib/libjxl_threads.so.0.11.1
```

The complete list is `stage/<arch>/.manifest/libjxl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libjxl
ls repo/8.0/*/ | grep '^libjxl-'
```

This writes `libjxl-<ver>-r0.tar.gz` (run time) and `libjxl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libjxl
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libjxl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libjxl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libjxl`](../../tests/src/) links libjxl the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libjxl root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libjxl'
```

```
== libjxl ==
  ok   ...
PASS libjxl
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.11.1; GIT=https://github.com/libjxl/libjxl.git; REF=v0.11.1
DEPS="brotli highway lcms2 libpng giflib libjpeg-turbo"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DJPEGXL_ENABLE_TOOLS=OFF -DJPEGXL_ENABLE_EXAMPLES=OFF
                   -DJPEGXL_ENABLE_BENCHMARK=OFF -DJPEGXL_ENABLE_FUZZERS=OFF
                   -DJPEGXL_ENABLE_DOXYGEN=OFF -DJPEGXL_ENABLE_MANPAGES=OFF
                   -DJPEGXL_ENABLE_JNI=OFF -DJPEGXL_ENABLE_SJPEG=OFF
                   -DJPEGXL_ENABLE_OPENEXR=OFF -DJPEGXL_ENABLE_SKCMS=OFF
                   -DJPEGXL_FORCE_SYSTEM_BROTLI=ON -DJPEGXL_FORCE_SYSTEM_HWY=ON
                   -DJPEGXL_FORCE_SYSTEM_LCMS2=ON -DJPEGXL_BUNDLE_LIBPNG=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
