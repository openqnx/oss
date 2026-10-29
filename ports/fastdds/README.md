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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add fastdds
apk add fastdds-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-fastdds && /usr/local/bin/qnx-port-tests/test_fastdds    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install fastdds`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa fastdds
ls dists/8.0/*/ | grep '^fastdds-'
```

This writes `fastdds-<ver>-r0.tar.gz` (run time) and `fastdds-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add fastdds
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/fastdds-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/fastdds-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_fastdds`](../../tests/src/) links fastdds the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-fastdds`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-fastdds
/usr/local/bin/qnx-port-tests/test_fastdds
```

```
== fastdds ==
  ok   ...
PASS fastdds
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_fastdds` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_fastdds`](../../tests/src/) -- a program linked against fastdds the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - get participant factory
   - create participant on domain 0
   - domain id readback
   - create publisher
   - create subscriber

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | fastdds 2.14.4-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | fastdds 2.14.4-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.14.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add fastdds=<version>`), sources in `ported/fastdds/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates fastdds`; port one with `scripts/port-update fastdds`.

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
