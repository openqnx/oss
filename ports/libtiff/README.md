# libtiff 4.7.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.com/libtiff/libtiff.git (tag `v4.7.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`xz`](../xz/) [`zstd`](../zstd/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libwebp`](../libwebp/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`libwebp`](../libwebp/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libtiff`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libtiff
oss-get install libtiff-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libtiff` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib xz zstd libjpeg-turbo libwebp
```

<details><summary>Full build chain (11)</summary>

[`gettext`](../gettext/) [`giflib`](../giflib/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v4.7.0 --recurse-submodules --shallow-submodules \
    https://gitlab.com/libtiff/libtiff.git src/libtiff
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libtiff -B build/$QNX_ARCH/libtiff -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -Dtiff-tools=OFF \
      -Dtiff-tests=OFF \
      -Dtiff-docs=OFF \
      -Dtiff-contrib=OFF \
      -Dlerc=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libtiff -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libtiff
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libtiff`.

### 5. What it installs

16 files under `/usr/local`: 6 shared-library entries, 5 headers.

```
/usr/local/lib/libtiff.so
/usr/local/lib/libtiff.so.6
/usr/local/lib/libtiff.so.6.1.0
/usr/local/lib/libtiffxx.so
/usr/local/lib/libtiffxx.so.6
/usr/local/lib/libtiffxx.so.6.1.0
```

The complete list is `stage/<arch>/.manifest/libtiff` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libtiff
ls repo/8.0/*/ | grep '^libtiff-'
```

This writes `libtiff-<ver>-r0.tar.gz` (run time) and `libtiff-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libtiff
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libtiff-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libtiff-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libtiff`](../../tests/src/) links libtiff the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libtiff root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libtiff'
```

```
== libtiff ==
  ok   ...
PASS libtiff
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.7.0; GIT=https://gitlab.com/libtiff/libtiff.git; REF=v4.7.0
DEPS="zlib xz zstd libjpeg-turbo libwebp"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -Dtiff-tools=OFF -Dtiff-tests=OFF
                   -Dtiff-docs=OFF -Dtiff-contrib=OFF -Dlerc=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
