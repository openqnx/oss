# cmake 4.1.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.kitware.com/cmake/cmake.git (tag `v4.1.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`curl`](../curl/) [`libuv`](../libuv/) |
| Run-time dependencies | [`curl`](../curl/) [`libuv`](../libuv/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install cmake
oss-get install cmake-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cmake` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib curl libuv
```

<details><summary>Full build chain (8)</summary>

[`brotli`](../brotli/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`libuv`](../libuv/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v4.1.1 --recurse-submodules --shallow-submodules \
    https://gitlab.kitware.com/cmake/cmake.git src/cmake
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/cmake -B build/$QNX_ARCH/cmake -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMake_HAVE_CXX_MAKE_UNIQUE=ON \
      -DCMake_HAVE_CXX_UNIQUE_PTR=ON \
      -DCMake_HAVE_CXX_FILESYSTEM=ON \
      -DCMake_HAVE_CXX_ATOMIC_BUILTIN=ON \
      -DBUILD_TESTING=OFF \
      -DBUILD_CursesDialog=OFF \
      -DBUILD_QtDialog=OFF \
      -DCMAKE_USE_OPENSSL=ON \
      -DCMAKE_USE_SYSTEM_ZLIB=ON \
      -DCMAKE_USE_SYSTEM_CURL=ON \
      -DCMAKE_USE_SYSTEM_LIBUV=ON \
      -DSPHINX_MAN=OFF \
      -DSPHINX_HTML=OFF
cmake --build build/$QNX_ARCH/cmake -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/cmake
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- CMake for the target (to build software on the board). Built with the host CMake and the QNX toolchain file; its bundled third-party libraries are used except where this tree provides them. Version as in the QNX Everywhere image.
- CMake's own C++ feature probes treat any compiler output as failure, and qcc always prints "cc: warning - lang-c++ is deprecated" (CMake's QCC module passes -lang-c++), so every probe "fails". The QNX 8 toolchain (GCC 12, libc++) has all of them; answer them.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cmake`.

### 5. What it installs

3907 files under `/usr/local`, 3 programs.

```
/usr/local/bin/cmake
/usr/local/bin/cpack
/usr/local/bin/ctest
```

The complete list is `stage/<arch>/.manifest/cmake` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH cmake
ls repo/8.0/*/ | grep '^cmake-'
```

This writes `cmake-<ver>-r0.tar.gz` (run time) and `cmake-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install cmake
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/cmake-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/cmake-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# CMake for the target (to build software on the board). Built with the host
# CMake and the QNX toolchain file; its bundled third-party libraries are used
# except where this tree provides them. Version as in the QNX Everywhere image.
VERSION=4.1.1; GIT=https://gitlab.kitware.com/cmake/cmake.git; REF=v4.1.1
DEPS="openssl zlib curl libuv"
# CMake's own C++ feature probes treat any compiler output as failure, and
# qcc always prints "cc: warning - lang-c++ is deprecated" (CMake's QCC module
# passes -lang-c++), so every probe "fails". The QNX 8 toolchain (GCC 12,
# libc++) has all of them; answer them.
CMAKE_ARGS_EXTRA=( -DCMake_HAVE_CXX_MAKE_UNIQUE=ON -DCMake_HAVE_CXX_UNIQUE_PTR=ON -DCMake_HAVE_CXX_FILESYSTEM=ON
                   -DCMake_HAVE_CXX_ATOMIC_BUILTIN=ON -DBUILD_TESTING=OFF -DBUILD_CursesDialog=OFF -DBUILD_QtDialog=OFF -DCMAKE_USE_OPENSSL=ON
                   -DCMAKE_USE_SYSTEM_ZLIB=ON -DCMAKE_USE_SYSTEM_CURL=ON -DCMAKE_USE_SYSTEM_LIBUV=ON
                   -DSPHINX_MAN=OFF -DSPHINX_HTML=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
