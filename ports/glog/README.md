# glog 0.7.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/google/glog.git (tag `v0.7.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`gflags`](../gflags/) |
| Run-time dependencies | [`gflags`](../gflags/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_glog`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install glog
oss-get install glog-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild glog` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gflags
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.7.1 --recurse-submodules --shallow-submodules \
    https://github.com/google/glog.git src/glog
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/glog < ports/glog/patches/0001-qnx-platform.patch
```

**[`0001-qnx-platform.patch`](patches/0001-qnx-platform.patch)** — glog: recognise QNX as a POSIX platform.

> Add GLOG_OS_QNX so QNX takes glog's generic POSIX paths rather than the Linux
> ones, which use /proc layouts and glibc-only system calls. Also use ::fdopen:
> QNX's libc++ <cstdio> does not put that POSIX function into namespace std.

Touches: `src/glog/platform.h`, `src/logging.cc` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/glog -B build/$QNX_ARCH/glog -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DWITH_GTEST=OFF \
      -DBUILD_TESTING=OFF \
      -DWITH_UNWIND=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/glog -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/glog
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH glog`.

### 5. What it installs

18 files under `/usr/local`: 3 shared-library entries, 9 headers.

```
/usr/local/lib/libglog.so
/usr/local/lib/libglog.so.0.7.1
/usr/local/lib/libglog.so.2
```

The complete list is `stage/<arch>/.manifest/glog` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH glog
ls repo/8.0/*/ | grep '^glog-'
```

This writes `glog-<ver>-r0.tar.gz` (run time) and `glog-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install glog
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/glog-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/glog-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_glog`](../../tests/src/) links glog the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_glog root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_glog'
```

```
== glog ==
  ok   ...
PASS glog
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.7.1; GIT=https://github.com/google/glog.git; REF=v0.7.1
DEPS="gflags"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DWITH_GTEST=OFF -DBUILD_TESTING=OFF
                   -DWITH_UNWIND=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
