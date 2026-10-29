# libarchive 3.7.7

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libarchive/libarchive.git (tag `v3.7.7`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`zstd`](../zstd/) [`openssl`](../openssl/) [`libxml2`](../libxml2/) [`expat`](../expat/) [`lz4`](../lz4/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`expat`](../expat/) [`lz4`](../lz4/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libarchive`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libarchive
oss-get install libarchive-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libarchive` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (12 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 xz zstd openssl libxml2 expat lz4
```

<details><summary>Full build chain (12)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v3.7.7 --recurse-submodules --shallow-submodules \
    https://github.com/libarchive/libarchive.git src/libarchive
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libarchive -B build/$QNX_ARCH/libarchive -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DENABLE_TEST=OFF \
      -DENABLE_CPIO=OFF \
      -DENABLE_CAT=OFF \
      -DENABLE_TAR=ON \
      -DENABLE_ACL=OFF \
      -DENABLE_ICONV=OFF \
      -DENABLE_PCREPOSIX=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libarchive -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libarchive
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): expat lz4

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libarchive`.

### 5. What it installs

52 files under `/usr/local`: 3 shared-library entries, 2 programs, 2 headers.

```
/usr/local/lib/libarchive.so
/usr/local/lib/libarchive.so.13
/usr/local/lib/libarchive.so.13.7.7
/usr/local/bin/bsdtar
/usr/local/bin/bsdunzip
```

The complete list is `stage/<arch>/.manifest/libarchive` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libarchive
ls repo/8.0/*/ | grep '^libarchive-'
```

This writes `libarchive-<ver>-r0.tar.gz` (run time) and `libarchive-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libarchive
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libarchive-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libarchive-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libarchive`](../../tests/src/) links libarchive the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libarchive root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libarchive'
```

```
== libarchive ==
  ok   ...
PASS libarchive
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.7.7; GIT=https://github.com/libarchive/libarchive.git; REF=v3.7.7
# Also linked at run time (found by scripts/gen-dependency-index): expat lz4
DEPS="zlib bzip2 xz zstd openssl libxml2 expat lz4"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_TEST=OFF -DENABLE_CPIO=OFF
                   -DENABLE_CAT=OFF -DENABLE_TAR=ON -DENABLE_ACL=OFF
                   -DENABLE_ICONV=OFF -DENABLE_PCREPOSIX=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
