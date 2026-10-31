# cmake 4.1.1

> Cross-platform build system generator

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.kitware.com/cmake/cmake.git (tag `v4.1.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`curl`](../curl/) [`libuv`](../libuv/) |
| Run-time dependencies | [`curl`](../curl/) [`libuv`](../libuv/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cmake`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add cmake
apk add cmake-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-cmake && /usr/local/bin/qnx-port-tests/test_cmake    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install cmake`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cmake
ls dists/8.0/*/ | grep '^cmake-'
```

This writes `cmake-<ver>-r0.tar.gz` (run time) and `cmake-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cmake
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cmake-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cmake-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cmake`](../../tests/src/) links cmake the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cmake`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cmake
/usr/local/bin/qnx-port-tests/test_cmake
```

```
== cmake ==
  ok   ...
PASS cmake
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cmake` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cmake`](../../tests/src/) -- a program linked against cmake the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cmake 4.1.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/cmake-4.1.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cmake 4.1.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/cmake-4.1.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cmake 4.1.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/cmake-4.1.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cmake 4.1.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/cmake-4.1.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.1.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cmake=<version>`), sources in `ported/cmake/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cmake`; port one with `scripts/port-update cmake`.

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
