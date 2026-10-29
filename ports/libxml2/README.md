# libxml2 2.13.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libxml2.git (tag `v2.13.5`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`xz`](../xz/) |
| Run-time dependencies | [`xz`](../xz/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxml2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libxml2
oss-get install libxml2-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxml2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib xz
```

<details><summary>Full build chain (6)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.13.5 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libxml2.git src/libxml2
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libxml2 -B build/$QNX_ARCH/libxml2 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DLIBXML2_WITH_PYTHON=OFF \
      -DLIBXML2_WITH_TESTS=OFF \
      -DLIBXML2_WITH_PROGRAMS=OFF \
      -DLIBXML2_WITH_ICONV=OFF \
      -DLIBXML2_WITH_LZMA=ON \
      -DLIBXML2_WITH_ZLIB=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libxml2 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libxml2
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxml2`.

### 5. What it installs

118 files under `/usr/local`: 3 shared-library entries, 1 programs, 46 headers.

```
/usr/local/lib/libxml2.so
/usr/local/lib/libxml2.so.2
/usr/local/lib/libxml2.so.2.13.5
/usr/local/bin/xml2-config
```

The complete list is `stage/<arch>/.manifest/libxml2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libxml2
ls repo/8.0/*/ | grep '^libxml2-'
```

This writes `libxml2-<ver>-r0.tar.gz` (run time) and `libxml2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libxml2
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libxml2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libxml2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libxml2`](../../tests/src/) links libxml2 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxml2 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libxml2'
```

```
== libxml2 ==
  ok   ...
PASS libxml2
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.13.5; GIT=https://gitlab.gnome.org/GNOME/libxml2.git; REF=v2.13.5
DEPS="zlib xz"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBXML2_WITH_PYTHON=OFF
                   -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_PROGRAMS=OFF
                   -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=ON -DLIBXML2_WITH_ZLIB=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
