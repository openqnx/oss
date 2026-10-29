# dlt-daemon 2.18.10

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/COVESA/dlt-daemon.git (tag `v2.18.10`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`libjson`](../libjson/) |
| Run-time dependencies | [`libjson`](../libjson/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install dlt-daemon
oss-get install dlt-daemon-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dlt-daemon` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib libjson
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.18.10 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/dlt-daemon.git src/dlt-daemon
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dlt-daemon < ports/dlt-daemon/patches/0001-qnx-syslog-header.patch
```

**[`0001-qnx-syslog-header.patch`](patches/0001-qnx-syslog-header.patch)** — DLT: include <syslog.h> on QNX.

> <sys/syslog.h> is a glibc/BSD alias that QNX does not ship; QNX has the POSIX
> <syslog.h>.

Touches: `src/offlinelogstorage/dlt_offline_logstorage.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/dlt-daemon -B build/$QNX_ARCH/dlt-daemon -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DWITH_DLT_QNX_SYSTEM=ON \
      -DWITH_DLT_CXX11_EXT=ON \
      -DDLT_IPC=UNIX_SOCKET \
      -DWITH_DLT_ADAPTOR=ON \
      -DWITH_DLT_USE_IPv6=OFF \
      -DWITH_LIB_SHORT_VERSION=ON \
      -DWITH_SYSTEMD=OFF \
      -DWITH_DLT_UNIT_TESTS=OFF \
      -DWITH_DLT_INSTALLED_TESTS=OFF \
      -DWITH_DLT_EXAMPLES=OFF \
      # \
      upstream \
      adds \
      dlt-test-cpp-extension \
      twice \
      when \
      CXX11_EXT \
      is \
      on \
      -DWITH_DLT_TESTS=OFF
cmake --build build/$QNX_ARCH/dlt-daemon -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/dlt-daemon
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libjson

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dlt-daemon`.

### 5. What it installs

35 files under `/usr/local`: 2 shared-library entries, 10 programs, 14 headers.

```
/usr/local/lib/libdlt.so
/usr/local/lib/libdlt.so.2
/usr/local/bin/dlt-adaptor-stdin
/usr/local/bin/dlt-adaptor-udp
/usr/local/bin/dlt-control
/usr/local/bin/dlt-convert
/usr/local/bin/dlt-daemon
/usr/local/bin/dlt-logstorage-ctrl
/usr/local/bin/dlt-passive-node-ctrl
/usr/local/bin/dlt-qnx-system
/usr/local/bin/dlt-receive
/usr/local/bin/dlt-sortbytimestamp
```

The complete list is `stage/<arch>/.manifest/dlt-daemon` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH dlt-daemon
ls repo/8.0/*/ | grep '^dlt-daemon-'
```

This writes `dlt-daemon-<ver>-r0.tar.gz` (run time) and `dlt-daemon-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install dlt-daemon
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/dlt-daemon-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/dlt-daemon-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.18.10; GIT=https://github.com/COVESA/dlt-daemon.git; REF=v2.18.10
# Also linked at run time (found by scripts/gen-dependency-index): libjson
DEPS="zlib libjson"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DWITH_DLT_QNX_SYSTEM=ON
                   -DWITH_DLT_CXX11_EXT=ON -DDLT_IPC=UNIX_SOCKET
                   -DWITH_DLT_ADAPTOR=ON -DWITH_DLT_USE_IPv6=OFF
                   -DWITH_LIB_SHORT_VERSION=ON -DWITH_SYSTEMD=OFF
                   -DWITH_DLT_UNIT_TESTS=OFF -DWITH_DLT_INSTALLED_TESTS=OFF
                   -DWITH_DLT_EXAMPLES=OFF
                   # upstream adds dlt-test-cpp-extension twice when CXX11_EXT is on
                   -DWITH_DLT_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
