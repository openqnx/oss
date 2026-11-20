# kiss-icp 1.3.0

> KISS-ICP LiDAR odometry C++ core: point-to-point ICP with adaptive threshold on a voxel hash map

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/PRBonn/kiss-icp.git (tag `v1.3.0`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`eigen`](../eigen/) [`sophus`](../sophus/) [`onetbb`](../onetbb/) [`tsl-robin-map`](../tsl-robin-map/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_kiss_icp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `kiss-icp` (programs, shared libraries, run-time data) and `kiss-icp-dev` (headers, pkg-config/CMake files, static libraries; needs `kiss-icp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`kiss-icp`](https://openqnx.github.io/oss/package/x86_64/kiss-icp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/kiss-icp.html)) · [`kiss-icp-dev`](https://openqnx.github.io/oss/package/x86_64/kiss-icp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/kiss-icp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add kiss-icp` (dependencies come along); one version: `apk add kiss-icp=<version>` | `apk del kiss-icp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install kiss-icp` (dependencies first) | `oss-get remove kiss-icp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`kiss-icp-dev` the same way, by its own name (`apk add kiss-icp-dev`, `oss-get install kiss-icp-dev`, by hand with its own file and list `kiss-icp-dev.list`); remove it before `kiss-icp`, which it needs.

```sh
# on the QNX target, as root
apk add kiss-icp kiss-icp-dev
apk add qnx-test-kiss-icp && /usr/local/bin/qnx-port-tests/test_kiss_icp    # its on-target test, from tests-8.0-<arch>
apk del kiss-icp kiss-icp-dev qnx-test-kiss-icp
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/kiss-icp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/kiss-icp.list
rm var/lib/manual/kiss-icp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild kiss-icp` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild eigen sophus onetbb tsl-robin-map
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.3.0 --recurse-submodules --shallow-submodules \
    https://github.com/PRBonn/kiss-icp.git src/kiss-icp
```

The project lives in the `cpp/kiss_icp/` subdirectory of the repository.

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/kiss-icp < ports/kiss-icp/patches/0001-install-rules.patch
```

**[`0001-install-rules.patch`](patches/0001-install-rules.patch)** — Add an optional install step and a CMake package (KISS_ICP_INSTALL).

> Upstream's C++ core is written to be pulled into another CMake project with
> add_subdirectory() (its Python bindings and ROS wrapper do that) and
> installs nothing. A QNX package needs files: the three static libraries,
> the headers under include/kiss_icp/{core,metrics,pipeline}/ -- the layout
> the headers already use to include each other ("kiss_icp/core/...") and the
> one the targets' INSTALL_INTERFACE points at -- and a CMake package
> "kiss_icp" with the targets kiss_icp::core, kiss_icp::pipeline and
> kiss_icp::metrics that looks up Eigen3, Sophus, TBB and tsl-robin-map
> again for the consumer.
>
> It is an option (KISS_ICP_INSTALL, off by default) so that the
> add_subdirectory() use is unchanged, and it is refused when a dependency
> was fetched and built inside the project: an installed package cannot name
> such a target. GNUInstallDirs is included before the targets are defined
> because CompilerOptions.cmake uses CMAKE_INSTALL_INCLUDEDIR, which was
> empty until now.

Touches: `cpp/kiss_icp/CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/kiss-icp/cpp/kiss_icp -B build/$QNX_ARCH/kiss-icp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DKISS_ICP_INSTALL=ON \
      -DUSE_CCACHE=OFF \
      -DUSE_SYSTEM_EIGEN3=ON \
      -DUSE_SYSTEM_SOPHUS=ON \
      -DUSE_SYSTEM_TBB=ON \
      -DUSE_SYSTEM_TSL-ROBIN-MAP=ON \
      -DFETCHCONTENT_FULLY_DISCONNECTED=ON
cmake --build build/$QNX_ARCH/kiss-icp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/kiss-icp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- kiss-icp: KISS-ICP LiDAR odometry -- point-to-point ICP with an adaptive correspondence threshold, motion compensation (deskewing) and a voxel hash map as local map. This is the C++ core (cpp/kiss_icp of the repository): libkiss_icp_core (voxel map, registration, preprocessing, threshold), libkiss_icp_pipeline (kiss_icp::pipeline::KissICP, the odometry itself) and libkiss_icp_metrics (KITTI sequence errors), on Eigen, Sophus, oneTBB and tsl::robin_map. The Python bindings and the ROS 2 wrapper of the repository are not part of this port. - The libraries are static and position independent, as upstream builds them (its wrappers link them into their own modules). - Upstream has no install step (it is meant to be add_subdirectory()'d); patch 0001 adds one: the three archives, the headers under include/kiss_icp/{core,pipeline,metrics}/ -- the layout the headers themselves include each other by -- and a CMake package kiss_icp (kiss_icp::core, kiss_icp::pipeline, kiss_icp::metrics) that finds the four dependencies again. - The four dependencies come from this workspace; upstream would otherwise download and build its own copies (FetchContent) when one is not found. FETCHCONTENT_FULLY_DISCONNECTED makes that an error instead of a download during the build. - USE_CCACHE=OFF: no compiler launcher from the build machine. - The repository's top-level Makefile is for the Python package ("make" runs pip install); the build starts in cpp/kiss_icp instead.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH kiss-icp`.

### 5. What it installs

15 files under `/usr/local`, 7 headers.

The complete list is `stage/<arch>/.manifest/kiss-icp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa kiss-icp
ls dists/8.0/*/ | grep '^kiss-icp-'
```

This writes `kiss-icp-<ver>-r0.tar.gz` (run time) and `kiss-icp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add kiss-icp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/kiss-icp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/kiss-icp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_kiss_icp`](../../tests/src/) links kiss-icp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-kiss-icp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-kiss-icp
/usr/local/bin/qnx-port-tests/test_kiss_icp
```

```
== kiss-icp ==
  ok   ...
PASS kiss-icp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_kiss_icp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_kiss_icp`](../../tests/src/) -- a program linked against kiss-icp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - PointToVoxel floors towards minus infinity
   - VoxelDownsample keeps one point in each of the 8 voxels
   - a voxel per point keeps all 64; one big voxel keeps 1
   - a new map is empty
   - 729 lattice points stored
   - duplicates and points closer than the map resolution are not added
   - closest neighbour of a displaced lattice point is that point, at the displacement's length
   - no neighbour in the 27 voxels around a far query
   - max_points_per_voxel limits a voxel to 3 points
   - Update drops the voxel beyond max_distance 5 from the origin
   - Update with a pose transforms the points and prunes around the new position
   - range filter keeps the points between 1 m and 10 m
   - deskewing moves points by the motion left until the end of the sweep
   - threshold starts at its initial value
   - a 3 m deviation gives sqrt((2^2 + 3^2)/2)
   - a deviation below min_motion is ignored
   - a rotation counts with its chord length at max range
   - ICP recovers the transform (0.06,-0.04,0.03 m; 0.004,-0.006,0.010 rad) to 1e-6
   - against an empty map the initial guess is returned
   - final position within 5 cm of the truth after more than 2.5 m
   - every frame's pose within 5 cm and 0.005 rad
   - last frame-to-frame motion matches the true step
   - the local map was built
   - absolute trajectory error below 5 cm / 0.01 rad
   - Reset clears the map and the pose
   - ... and 2 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | kiss-icp 1.3.0-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/kiss-icp-1.3.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | kiss-icp 1.3.0-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/kiss-icp-1.3.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add kiss-icp=<version>`), sources in `ported/kiss-icp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates kiss-icp`; port one with `scripts/port-update kiss-icp`.

<details><summary>recipe.sh</summary>

```bash
# kiss-icp: KISS-ICP LiDAR odometry -- point-to-point ICP with an adaptive
# correspondence threshold, motion compensation (deskewing) and a voxel hash
# map as local map. This is the C++ core (cpp/kiss_icp of the repository):
# libkiss_icp_core (voxel map, registration, preprocessing, threshold),
# libkiss_icp_pipeline (kiss_icp::pipeline::KissICP, the odometry itself)
# and libkiss_icp_metrics (KITTI sequence errors), on Eigen, Sophus, oneTBB
# and tsl::robin_map. The Python bindings and the ROS 2 wrapper of the
# repository are not part of this port.
#
# - The libraries are static and position independent, as upstream builds
#   them (its wrappers link them into their own modules).
# - Upstream has no install step (it is meant to be add_subdirectory()'d);
#   patch 0001 adds one: the three archives, the headers under
#   include/kiss_icp/{core,pipeline,metrics}/ -- the layout the headers
#   themselves include each other by -- and a CMake package kiss_icp
#   (kiss_icp::core, kiss_icp::pipeline, kiss_icp::metrics) that finds the
#   four dependencies again.
# - The four dependencies come from this workspace; upstream would otherwise
#   download and build its own copies (FetchContent) when one is not found.
#   FETCHCONTENT_FULLY_DISCONNECTED makes that an error instead of a
#   download during the build.
# - USE_CCACHE=OFF: no compiler launcher from the build machine.
# - The repository's top-level Makefile is for the Python package ("make"
#   runs pip install); the build starts in cpp/kiss_icp instead.
VERSION=1.3.0; GIT=https://github.com/PRBonn/kiss-icp.git; REF=v1.3.0
DEPS="eigen sophus onetbb tsl-robin-map"
BUILDSYS=cmake
SUBDIR=cpp/kiss_icp
CMAKE_ARGS_EXTRA=( -DKISS_ICP_INSTALL=ON -DUSE_CCACHE=OFF
                   -DUSE_SYSTEM_EIGEN3=ON -DUSE_SYSTEM_SOPHUS=ON -DUSE_SYSTEM_TBB=ON
                   -DUSE_SYSTEM_TSL-ROBIN-MAP=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
