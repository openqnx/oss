# civetweb 1.16

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/civetweb/civetweb.git (tag `v1.16`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_civetweb`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install civetweb
oss-get install civetweb-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild civetweb` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.16 --recurse-submodules --shallow-submodules \
    https://github.com/civetweb/civetweb.git src/civetweb
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/civetweb < ports/civetweb/patches/0001-qnx-target-architecture.patch
```

**[`0001-qnx-target-architecture.patch`](patches/0001-qnx-target-architecture.patch)** — Take the target architecture from the toolchain on QNX.

> DetermineTargetArchitecture runs "$CC -dumpmachine", which qcc does not
> support, and configure stops with "Failed to determine target architecture
> triplet: 1". CMAKE_SYSTEM_PROCESSOR from the QNX toolchain file is the same
> answer.

Touches: `cmake/DetermineTargetArchitecture.cmake` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/civetweb -B build/$QNX_ARCH/civetweb -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DCIVETWEB_BUILD_TESTING=OFF \
      -DCIVETWEB_ENABLE_CXX=ON \
      -DCIVETWEB_ENABLE_SSL=ON \
      -DCIVETWEB_ENABLE_SSL_DYNAMIC_LOADING=OFF \
      -DCIVETWEB_SSL_OPENSSL_API_3_0=ON \
      -DCIVETWEB_SSL_OPENSSL_API_1_1=OFF \
      -DCIVETWEB_ENABLE_SERVER_EXECUTABLE=ON \
      -DCIVETWEB_ENABLE_ASAN=OFF
cmake --build build/$QNX_ARCH/civetweb -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/civetweb
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH civetweb`.

### 5. What it installs

16 files under `/usr/local`: 4 shared-library entries, 1 programs, 2 headers.

```
/usr/local/lib/libcivetweb-cpp.so
/usr/local/lib/libcivetweb-cpp.so.1.15.0
/usr/local/lib/libcivetweb.so
/usr/local/lib/libcivetweb.so.1.15.0
/usr/local/bin/civetweb
```

The complete list is `stage/<arch>/.manifest/civetweb` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH civetweb
ls repo/8.0/*/ | grep '^civetweb-'
```

This writes `civetweb-<ver>-r0.tar.gz` (run time) and `civetweb-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install civetweb
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/civetweb-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/civetweb-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_civetweb`](../../tests/src/) links civetweb the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_civetweb root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_civetweb'
```

```
== civetweb ==
  ok   ...
PASS civetweb
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.16; GIT=https://github.com/civetweb/civetweb.git; REF=v1.16
DEPS="openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCIVETWEB_BUILD_TESTING=OFF -DCIVETWEB_ENABLE_CXX=ON
                   -DCIVETWEB_ENABLE_SSL=ON -DCIVETWEB_ENABLE_SSL_DYNAMIC_LOADING=OFF -DCIVETWEB_SSL_OPENSSL_API_3_0=ON -DCIVETWEB_SSL_OPENSSL_API_1_1=OFF
                   -DCIVETWEB_ENABLE_SERVER_EXECUTABLE=ON -DCIVETWEB_ENABLE_ASAN=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
