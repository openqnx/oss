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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add glog
apk add glog-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-glog && /usr/local/bin/qnx-port-tests/test_glog    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install glog`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa glog
ls dists/8.0/*/ | grep '^glog-'
```

This writes `glog-<ver>-r0.tar.gz` (run time) and `glog-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add glog
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/glog-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/glog-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_glog`](../../tests/src/) links glog the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-glog`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-glog
/usr/local/bin/qnx-port-tests/test_glog
```

```
== glog ==
  ok   ...
PASS glog
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_glog` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_glog`](../../tests/src/) -- a program linked against glog the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - InitGoogleLogging + LOG() did not abort
   - LOG_IF evaluated its condition once

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | glog 0.7.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | glog 0.7.1-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.7.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add glog=<version>`), sources in `ported/glog/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates glog`; port one with `scripts/port-update glog`.

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
