# libzip 1.11.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/nih-at/libzip.git (tag `v1.11.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`zstd`](../zstd/) [`openssl`](../openssl/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libzip`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libzip
oss-get install libzip-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libzip` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 xz zstd openssl
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.11.1 --recurse-submodules --shallow-submodules \
    https://github.com/nih-at/libzip.git src/libzip
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libzip -B build/$QNX_ARCH/libzip -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_DOC=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_REGRESS=OFF \
      -DBUILD_OSSFUZZ=OFF \
      -DBUILD_TOOLS=ON \
      -DENABLE_GNUTLS=OFF \
      -DENABLE_MBEDTLS=OFF \
      -DENABLE_OPENSSL=ON
cmake --build build/$QNX_ARCH/libzip -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libzip
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libzip`.

### 5. What it installs

16 files under `/usr/local`: 3 shared-library entries, 3 programs, 2 headers.

```
/usr/local/lib/libzip.so
/usr/local/lib/libzip.so.5
/usr/local/lib/libzip.so.5.5
/usr/local/bin/zipcmp
/usr/local/bin/zipmerge
/usr/local/bin/ziptool
```

The complete list is `stage/<arch>/.manifest/libzip` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libzip
ls repo/8.0/*/ | grep '^libzip-'
```

This writes `libzip-<ver>-r0.tar.gz` (run time) and `libzip-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libzip
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libzip-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libzip-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libzip`](../../tests/src/) links libzip the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libzip root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libzip'
```

```
== libzip ==
  ok   ...
PASS libzip
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.11.1; GIT=https://github.com/nih-at/libzip.git; REF=v1.11.1
DEPS="zlib bzip2 xz zstd openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_DOC=OFF -DBUILD_EXAMPLES=OFF -DBUILD_REGRESS=OFF
                   -DBUILD_OSSFUZZ=OFF -DBUILD_TOOLS=ON -DENABLE_GNUTLS=OFF -DENABLE_MBEDTLS=OFF -DENABLE_OPENSSL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
