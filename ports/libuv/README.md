# libuv 1.51.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/libuv/libuv.git (tag `v1.51.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libuv`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libuv
oss-get install libuv-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libuv` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.51.0 --recurse-submodules --shallow-submodules \
    https://github.com/libuv/libuv.git src/libuv
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libuv < ports/libuv/patches/0001-qnx8-syspage-layout.patch
```

**[`0001-qnx8-syspage-layout.patch`](patches/0001-qnx8-syspage-layout.patch)** — libuv: uptime on QNX 8.0 from CLOCK_MONOTONIC.

> QNX 8.0 removed qtime_entry::nsec from the system page, so uv_uptime() no
> longer compiled. CLOCK_MONOTONIC counts from boot on QNX and needs no system
> page access.

Touches: `src/unix/qnx.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libuv -B build/$QNX_ARCH/libuv -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DLIBUV_BUILD_TESTS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/libuv -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libuv
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- 1.51: the version Node.js 24 bundles and needs (uv_thread_setname is 1.50+).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libuv`.

### 5. What it installs

24 files under `/usr/local`: 3 shared-library entries, 14 headers.

```
/usr/local/lib/libuv.so
/usr/local/lib/libuv.so.1
/usr/local/lib/libuv.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/libuv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libuv
ls repo/8.0/*/ | grep '^libuv-'
```

This writes `libuv-<ver>-r0.tar.gz` (run time) and `libuv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libuv
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libuv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libuv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_libuv`](../../tests/src/) links libuv the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libuv root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_libuv'
```

```
== libuv ==
  ok   ...
PASS libuv
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.51.0; GIT=https://github.com/libuv/libuv.git; REF=v1.51.0
# 1.51: the version Node.js 24 bundles and needs (uv_thread_setname is 1.50+).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBUV_BUILD_TESTS=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
