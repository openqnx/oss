# hailort 4.14.0

> Light-weight, high-performance inference framework for Hailo devices

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/hailo-ai/hailort.git (tag `v4.14.0`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_hailort`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `hailort` (programs, shared libraries, run-time data) and `hailort-dev` (headers, pkg-config/CMake files, static libraries; needs `hailort`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`hailort`](https://openqnx.github.io/oss/package/x86_64/hailort.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hailort.html)) · [`hailort-dev`](https://openqnx.github.io/oss/package/x86_64/hailort-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hailort-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add hailort` (dependencies come along); one version: `apk add hailort=<version>` | `apk del hailort` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install hailort` (dependencies first) | `oss-get remove hailort` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`hailort-dev` the same way, by its own name (`apk add hailort-dev`, `oss-get install hailort-dev`, by hand with its own file and list `hailort-dev.list`); remove it before `hailort`, which it needs.

```sh
# on the QNX target, as root
apk add hailort hailort-dev
apk add qnx-test-hailort && /usr/local/bin/qnx-port-tests/test_hailort    # its on-target test, from tests-8.0-<arch>
apk del hailort hailort-dev qnx-test-hailort
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/hailort.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/hailort.list
rm var/lib/manual/hailort.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild hailort` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v4.14.0 --recurse-submodules --shallow-submodules \
    https://github.com/hailo-ai/hailort.git src/hailort
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/hailort < ports/hailort/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/hailort < ports/hailort/patches/0002-cmake-if-qnx.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — hailort on QNX, from QNX's port (github.com/qnx-ports/hailort, branch

> qnx-4.14, commit 50d78d2c): HailoRT's CMake adds drivers/qnx, the QNX PCIe
> driver, when it builds for QNX, but the public repository has no
> drivers/qnx (the driver ships separately), so configuring for QNX stopped
> at the missing directory. QNX removed the line; libhailort talks to the
> installed driver through /dev/hailo*.

Touches: `hailort/CMakeLists.txt` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

**[`0002-cmake-if-qnx.patch`](patches/0002-cmake-if-qnx.patch)** — CMake: test if(QNX), not CMAKE_SYSTEM_NAME STREQUAL QNX (which compares with the variable QNX's value).

> HailoRT picks its QNX sources with if(CMAKE_SYSTEM_NAME STREQUAL QNX).
> CMake defines a variable QNX (1) when it targets QNX, and an unquoted
> argument that names a variable is replaced by its value, so the test
> compared "QNX" with "1" and was false: the Linux event, driver-scan and
> PCIe sources were compiled instead of the QNX ones (eventfd/poll code that
> does not build against QNX's pevents handles), and libhailort was not
> linked with QNX's libpci. With these files' cmake_minimum_required(3.0)
> policies even a quoted "QNX" is dereferenced (CMP0054 OLD), so the tests
> use the QNX variable itself, as CMake intends: if(QNX), if(NOT QNX).

Touches: `hailort/CMakeLists.txt`, `hailort/hailortcli/CMakeLists.txt`, `hailort/libhailort/bindings/CMakeLists.txt`, `hailort/libhailort/examples/c/CMakeLists.txt`, `hailort/libhailort/examples/cpp/CMakeLists.txt`, `hailort/libhailort/src/CMakeLists.txt` … · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/hailort -B build/$QNX_ARCH/hailort -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DHAILO_BUILD_PYBIND=OFF \
      -DHAILO_BUILD_EXAMPLES=OFF \
      -DHAILO_BUILD_SERVICE=OFF \
      -DHAILO_BUILD_GSTREAMER=OFF \
      -DHAILO_BUILD_UT=OFF \
      -DHAILO_BUILD_HW_DEBUG_TOOL=OFF \
      -DHAILO_COMPILE_WARNING_AS_ERROR=OFF
cmake --build build/$QNX_ARCH/hailort -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/hailort
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- hailort: Hailo's runtime for the Hailo-8/8L AI accelerators -- libhailort (the C/C++ API: devices over PCIe or Ethernet, HEF network files, vstreams) and the hailortcli command -- with QNX's change (patches/0001, from github.com/qnx-ports/hailort, branch qnx-4.14). HailoRT has a QNX OS layer of its own (events on pevents, PCIe through QNX's libpci and the driver's /dev/hailo* devctl interface); the PCIe driver itself is not public and not part of this package. 4.14.0, the release QNX's port is based on: in later releases (4.18 on) the QNX branch of the driver interface (hailo_ioctl_common.h) no longer defines the ioctl groups the newer runtime uses (HAILO_NNC_IOCTL_MAGIC and others), so they do not build for QNX, and the device-side numbers would have to come from a matching QNX driver. The runtime, the driver and the device firmware go together: use this with QNX's 4.14 driver.
- The build fetches its own dependencies at configure time, each pinned to a commit (hailort/pre_build/external/CMakeLists.txt: spdlog, CLI11 with Hailo's completion changes, nlohmann json, protobuf with a build-machine protoc for its generated code, readerwriterqueue, DotWriter, pevents, ...), and links them statically: libhailort carries the versions it was tested with and exports only its own API. No HailoRT service (multi-process sharing over gRPC), no Python bindings, no GStreamer plugin (none on QNX upstream).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH hailort`.

### 5. What it installs

27 files under `/usr/local`: 2 shared-library entries, 1 programs, 20 headers.

```
/usr/local/lib/libhailort.so
/usr/local/lib/libhailort.so.4.14.0
/usr/local/bin/hailortcli
```

The complete list is `stage/<arch>/.manifest/hailort` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa hailort
ls dists/8.0/*/ | grep '^hailort-'
```

This writes `hailort-<ver>-r0.tar.gz` (run time) and `hailort-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add hailort
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/hailort-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/hailort-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_hailort`](../../tests/src/) links hailort the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-hailort`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-hailort
/usr/local/bin/qnx-port-tests/test_hailort
```

```
== hailort ==
  ok   ...
PASS hailort
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_hailort` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_hailort`](../../tests/src/) -- a program linked against hailort the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - library version 4.14.0
   - device scan: no Hailo device found, no error
   - virtual device refused without hardware
   - ... with HAILO_OUT_OF_PHYSICAL_DEVICES
   - HEF parsed from memory (header, MD5, protobuf body)
   - the HEF has no network groups
   - HEF parsed from a file
   - damaged body (MD5 mismatch): HAILO_INVALID_HEF
   - wrong magic: HAILO_INVALID_HEF
   - missing file: HAILO_OPEN_FILE_FAILURE
   - hailortcli --version: 4.14.0
   - hailortcli scan: \"Hailo devices not found\"

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | hailort 4.14.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/hailort-4.14.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | hailort 4.14.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/hailort-4.14.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.14.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add hailort=<version>`), sources in `ported/hailort/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates hailort`; port one with `scripts/port-update hailort`.

<details><summary>recipe.sh</summary>

```bash
# hailort: Hailo's runtime for the Hailo-8/8L AI accelerators -- libhailort
# (the C/C++ API: devices over PCIe or Ethernet, HEF network files, vstreams)
# and the hailortcli command -- with QNX's change (patches/0001, from
# github.com/qnx-ports/hailort, branch qnx-4.14). HailoRT has a QNX OS layer of
# its own (events on pevents, PCIe through QNX's libpci and the driver's
# /dev/hailo* devctl interface); the PCIe driver itself is not public and not
# part of this package.
#
# 4.14.0, the release QNX's port is based on: in later releases (4.18 on) the
# QNX branch of the driver interface (hailo_ioctl_common.h) no longer defines
# the ioctl groups the newer runtime uses (HAILO_NNC_IOCTL_MAGIC and others),
# so they do not build for QNX, and the device-side numbers would have to come
# from a matching QNX driver. The runtime, the driver and the device firmware
# go together: use this with QNX's 4.14 driver.
VERSION=4.14.0; GIT=https://github.com/hailo-ai/hailort.git; REF=v4.14.0
DEPS=""
BUILDSYS=cmake
# The build fetches its own dependencies at configure time, each pinned to a
# commit (hailort/pre_build/external/CMakeLists.txt: spdlog, CLI11 with Hailo's
# completion changes, nlohmann json, protobuf with a build-machine protoc for
# its generated code, readerwriterqueue, DotWriter, pevents, ...), and links
# them statically: libhailort carries the versions it was tested with and
# exports only its own API. No HailoRT service (multi-process sharing over
# gRPC), no Python bindings, no GStreamer plugin (none on QNX upstream).
CMAKE_ARGS_EXTRA=( -DHAILO_BUILD_PYBIND=OFF -DHAILO_BUILD_EXAMPLES=OFF -DHAILO_BUILD_SERVICE=OFF
                   -DHAILO_BUILD_GSTREAMER=OFF -DHAILO_BUILD_UT=OFF -DHAILO_BUILD_HW_DEBUG_TOOL=OFF
                   -DHAILO_COMPILE_WARNING_AS_ERROR=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
