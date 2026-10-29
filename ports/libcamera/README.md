# libcamera 0.5.1+rpt20250722

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/raspberrypi/libcamera.git (tag `v0.5.1+rpt20250722`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`libyaml`](../libyaml/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libcamera
oss-get install libcamera-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libcamera` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libyaml
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.5.1+rpt20250722 --recurse-submodules --shallow-submodules \
    https://github.com/raspberrypi/libcamera.git src/libcamera
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libcamera < ports/libcamera/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Port RPi image processing algorithms to QNX

> Imported from qnx-ports/libcamera (v0.5.1+rpt20250722...qnx-v0.5.1_rpt20250722); original authorship kept.
>
>
> QNXCAM-3642
> ---
>  .gitignore                                    |   4 +
>  include/libcamera/base/log.h                  |  78 +++
>  include/libcamera/internal/yaml_parser.h      |   8 +
>  include/linux/bcm2835-isp.h                   |  12 +
>  qnx/LICENSE.md                                |  23 +
>  qnx/README.md                                 |   7 +
>  qnx/build/CMakeLists.txt                      |  82 +++
>  qnx/build/Makefile                            |   8 +
>  qnx/build/common.mk                           |  72 +++
>  qnx/build/nto-aarch64-le/Makefile             |   1 +
>  qnx/build/qnx.nto.toolchain.cmake             |   6 +
>  qnx/build/qnx800/dependencies.yml             |   2 +
>  qnx/build/qnx800dev/dependencies.yml          |   2 +
>  qnx/build/sdp_mainline/dependencies.yml       |   2 +
>  qnx/shiplist/rpi_camera_ipa.xml               |  18 +
>  qnx/shiplist/rpi_camera_ipa_dev.xml           |  15 +
>  src/ipa/libipa/pwl.cpp                        |   8 +
>  src/ipa/libipa/pwl.h                          |   9 +
>  src/ipa/rpi/cam_helper/cam_helper.cpp         |   8 +
>  src/ipa/rpi/cam_helper/cam_helper.h           |   2 +
>  src/ipa/rpi/cam_helper/cam_helper_imx500.cpp  |   2 +
>  src/ipa/rpi/cam_helper/md_parser.h            |   9 +
>  src/ipa/rpi/controller/agc_algorithm.h        |   8 +
>  src/ipa/rpi/controller/alsc_status.h          |  10 +
>  src/ipa/rpi/controller/cac_status.h           |   7 +
>  src/ipa/rpi/controller/controller.h           |   4 +
>  src/ipa/rpi/controller/hdr_algorithm.h        |   4 +
>  src/ipa/rpi/controller/histogram.h            |   8 +
>  src/ipa/rpi/controller/metadata.h             |  49 +-
>  src/ipa/rpi/controller/region_stats.h         |   8 +
>  src/ipa/rpi/controller/rpi/af.cpp             |   2 +
>  src/ipa/rpi/controller/rpi/agc.cpp            |  10 +
>  src/ipa/rpi/controller/rpi/agc.h              |  13 +
>  src/ipa/rpi/controller/rpi/agc_channel.cpp    |  14 +
>  src/ipa/rpi/controller/rpi/agc_channel.h      |  17 +
>  src/ipa/rpi/controller/rpi/awb.cpp            |   3 +
>  src/ipa/rpi/controller/rpi/cac.cpp            |   5 +
>  src/ipa/rpi/controller/rpi/hdr.cpp            |   4 +
>  src/ipa/rpi/controller/rpi/hdr.h              |  20 +
>  src/ipa/rpi/controller/statistics.h           |  15 +-
>  src/ipa/rpi/qnx/log_handler.cpp               |  69 +++
>  src/ipa/rpi/qnx/log_handler.h                 |  93 ++++
>  src/ipa/rpi/qnx/no_throw_allocator.h          |  67 +++
>  src/ipa/rpi/qnx/no_throw_any.h                | 233 ++++++++
>  src/ipa/rpi/qnx/pisp_statistics.h             |  70 +++
>  .../rpi/qnx/public/include/rpi_camera_ipa.h   |  81 +++
>  src/ipa/rpi/qnx/rpi_camera_ipa.cpp            | 505 ++++++++++++++++++
>  src/libcamera/yaml_parser.cpp                 |  22 +
>  48 files changed, 1712 insertions(+), 7 deletions(-)
>  create mode 100755 qnx/LICENSE.md
>  create mode 100644 qnx/README.md
>  create mode 100644 qnx/build/CMakeLists.txt
>  create mode 100755 qnx/build/Makefile
>  create mode 100644 qnx/build/common.mk
>  create mode 100644 qnx/build/nto-aarch64-le/Makefile
>  create mode 100644 qnx/build/qnx.nto.toolchain.cmake
>  create mode 100644 qnx/build/qnx800/dependencies.yml
>  create mode 100644 qnx/build/qnx800dev/dependencies.yml
>  create mode 100644 qnx/build/sdp_mainline/dependencies.yml
>  create mode 100644 qnx/shiplist/rpi_camera_ipa.xml
>  create mode 100644 qnx/shiplist/rpi_camera_ipa_dev.xml
>  create mode 100644 src/ipa/rpi/qnx/log_handler.cpp
>  create mode 100644 src/ipa/rpi/qnx/log_handler.h
>  create mode 100644 src/ipa/rpi/qnx/no_throw_allocator.h
>  create mode 100644 src/ipa/rpi/qnx/no_throw_any.h
>  create mode 100644 src/ipa/rpi/qnx/pisp_statistics.h
>  create mode 100644 src/ipa/rpi/qnx/public/include/rpi_camera_ipa.h
>  create mode 100644 src/ipa/rpi/qnx/rpi_camera_ipa.cpp

Touches: `.gitignore`, `include/libcamera/base/log.h`, `include/libcamera/internal/yaml_parser.h`, `include/linux/bcm2835-isp.h`, `qnx/LICENSE.md`, `qnx/README.md` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libcamera; B=build/$QNX_ARCH/libcamera; mkdir -p $B
```

```bash
do_configure() {
  cmake -S "$RECIPE_DIR/libcamera/files" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPROJECT_ROOT="$S"
}
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libcamera's Raspberry Pi image processing algorithms (auto white balance, auto exposure) for the Camera Module 3 on QNX -- the part qnx-ports ports, built as librpi_camera_ipa with qnx-ports' own CMakeLists (files/), not libcamera's Meson build (whose device layer is V4L2/Linux). patches/: the QNX changes from qnx-ports/libcamera. Raspberry Pi 4/5 only: aarch64le.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libcamera`.

### 5. What it installs

3 files under `/usr/local`: 3 shared-library entries.

```
/usr/local/lib/librpi_camera_ipa.so
/usr/local/lib/librpi_camera_ipa.so.0.5.1
/usr/local/lib/librpi_camera_ipa.so.1
```

The complete list is `stage/<arch>/.manifest/libcamera` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libcamera
ls repo/8.0/*/ | grep '^libcamera-'
```

This writes `libcamera-<ver>-r0.tar.gz` (run time) and `libcamera-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libcamera
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libcamera-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libcamera-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.5.1+rpt20250722; GIT=https://github.com/raspberrypi/libcamera.git; REF=v0.5.1+rpt20250722
# libcamera's Raspberry Pi image processing algorithms (auto white balance,
# auto exposure) for the Camera Module 3 on QNX -- the part qnx-ports ports,
# built as librpi_camera_ipa with qnx-ports' own CMakeLists (files/), not
# libcamera's Meson build (whose device layer is V4L2/Linux). patches/: the
# QNX changes from qnx-ports/libcamera. Raspberry Pi 4/5 only: aarch64le.
ARCHES_SUPPORTED="aarch64le"
DEPS="libyaml"
BUILDSYS=custom
do_configure() {
  cmake -S "$RECIPE_DIR/libcamera/files" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPROJECT_ROOT="$S"
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
