# freetype 2.13.3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/freetype/freetype.git (tag `VER-2-13-3`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`libpng`](../libpng/) [`brotli`](../brotli/) |
| Run-time dependencies | [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`libpng`](../libpng/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_freetype`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install freetype
oss-get install freetype-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild freetype` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib bzip2 libpng brotli
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b VER-2-13-3 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/freetype/freetype.git src/freetype
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/freetype -B build/$QNX_ARCH/freetype -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DFT_DISABLE_HARFBUZZ=ON \
      -DFT_REQUIRE_ZLIB=ON \
      -DFT_REQUIRE_PNG=ON \
      -DFT_REQUIRE_BZIP2=ON \
      -DZLIB_ROOT="$STAGE$PREFIX" \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/freetype -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/freetype
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH freetype`.

### 5. What it installs

62 files under `/usr/local`: 3 shared-library entries, 55 headers.

```
/usr/local/lib/libfreetype.so
/usr/local/lib/libfreetype.so.6
/usr/local/lib/libfreetype.so.6.20.2
```

The complete list is `stage/<arch>/.manifest/freetype` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH freetype
ls repo/8.0/*/ | grep '^freetype-'
```

This writes `freetype-<ver>-r0.tar.gz` (run time) and `freetype-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install freetype
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/freetype-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/freetype-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_freetype`](../../tests/src/) links freetype the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_freetype root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_freetype'
```

```
== freetype ==
  ok   ...
PASS freetype
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.13.3; GIT=https://gitlab.freedesktop.org/freetype/freetype.git; REF=VER-2-13-3
DEPS="zlib bzip2 libpng brotli"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DFT_DISABLE_HARFBUZZ=ON
                   -DFT_REQUIRE_ZLIB=ON -DFT_REQUIRE_PNG=ON -DFT_REQUIRE_BZIP2=ON
                   -DZLIB_ROOT="$STAGE$PREFIX" -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
