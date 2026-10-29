# fastdds 2.14.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/eProsima/Fast-DDS.git (tag `v2.14.4`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`fastcdr`](../fastcdr/) [`foonathan_memory`](../foonathan_memory/) [`tinyxml2`](../tinyxml2/) [`openssl`](../openssl/) |
| Run-time dependencies | [`fastcdr`](../fastcdr/) [`foonathan_memory`](../foonathan_memory/) [`openssl`](../openssl/) [`tinyxml2`](../tinyxml2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_fastdds`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install fastdds
oss-get install fastdds-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild fastdds` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild fastcdr foonathan_memory tinyxml2 openssl
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.14.4 --recurse-submodules --shallow-submodules \
    https://github.com/eProsima/Fast-DDS.git src/fastdds
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/fastdds < ports/fastdds/patches/0001-qnx-threadctl-affinity.patch
patch -p1 -d src/fastdds < ports/fastdds/patches/0002-qnx-no-inotify-filewatch.patch
```

**[`0001-qnx-threadctl-affinity.patch`](patches/0001-qnx-threadctl-affinity.patch)** — Fast DDS: CPU count and thread affinity on QNX.

> The pthread threading code uses glibc's <sys/sysinfo.h> and cpu_set_t. On QNX
> the CPU count comes from sysconf() and affinity is set with ThreadCtl(); the
> Linux-only SCHED_BATCH and SCHED_IDLE policies map to SCHED_OTHER.

Touches: `src/cpp/utils/threading/threading_pthread.ipp` · Upstream status: QNX-specific

**[`0002-qnx-no-inotify-filewatch.patch`](patches/0002-qnx-no-inotify-filewatch.patch)** — Fast DDS: no inotify file watch on QNX.

> QNX defines __unix__, but the file-watch backend behind those guards is
> inotify, which QNX lacks. Use the existing no-watch fallback on QNX: XML
> profiles are read once at start-up rather than re-read on change.

Touches: `src/cpp/utils/SystemInfo.cpp`, `thirdparty/filewatch/FileWatch.hpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/fastdds -B build/$QNX_ARCH/fastdds -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DTHIRDPARTY=ON \
      -DCOMPILE_EXAMPLES=OFF \
      -DBUILD_DOCUMENTATION=OFF \
      -DSECURITY=ON \
      -DTHIRDPARTY_Asio=FORCE \
      -DTHIRDPARTY_TinyXML2=OFF \
      -DOPENSSL_ROOT_DIR="$STAGE$PREFIX" \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/fastdds -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/fastdds
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH fastdds`.

### 5. What it installs

458 files under `/usr/local`: 3 shared-library entries, 4 programs, 435 headers.

```
/usr/local/lib/libfastrtps.so
/usr/local/lib/libfastrtps.so.2.14
/usr/local/lib/libfastrtps.so.2.14.4
/usr/local/bin/fast-discovery-server
/usr/local/bin/fast-discovery-server-1.0.1
/usr/local/bin/fastdds
/usr/local/bin/ros-discovery
```

The complete list is `stage/<arch>/.manifest/fastdds` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH fastdds
ls repo/8.0/*/ | grep '^fastdds-'
```

This writes `fastdds-<ver>-r0.tar.gz` (run time) and `fastdds-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install fastdds
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/fastdds-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/fastdds-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_fastdds`](../../tests/src/) links fastdds the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fastdds root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_fastdds'
```

```
== fastdds ==
  ok   ...
PASS fastdds
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.14.4; GIT=https://github.com/eProsima/Fast-DDS.git; REF=v2.14.4
DEPS="fastcdr foonathan_memory tinyxml2 openssl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DTHIRDPARTY=ON -DCOMPILE_EXAMPLES=OFF
                   -DBUILD_DOCUMENTATION=OFF -DSECURITY=ON
                   -DTHIRDPARTY_Asio=FORCE -DTHIRDPARTY_TinyXML2=OFF
                   -DOPENSSL_ROOT_DIR="$STAGE$PREFIX"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
