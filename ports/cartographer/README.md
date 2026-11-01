# cartographer 2.0.0

> Real-time 2D and 3D SLAM library across multiple platforms and sensors

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/cartographer-project/cartographer.git (tag `2.0.0`) |
| Build system | CMake |
| QNX patches | 3 |
| Build dependencies | [`abseil`](../abseil/) [`boost`](../boost/) [`ceres-solver`](../ceres-solver/) [`eigen`](../eigen/) [`glog`](../glog/) [`gflags`](../gflags/) [`lua5.3`](../lua5.3/) [`protobuf`](../protobuf/) [`cairo`](../cairo/) |
| Run-time dependencies | [`abseil`](../abseil/) [`boost`](../boost/) [`cairo`](../cairo/) [`ceres-solver`](../ceres-solver/) [`gflags`](../gflags/) [`glog`](../glog/) [`lua5.3`](../lua5.3/) [`protobuf`](../protobuf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cartographer`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `cartographer` (programs, shared libraries, run-time data) and `cartographer-dev` (headers, pkg-config/CMake files, static libraries; needs `cartographer`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`cartographer`](https://openqnx.github.io/oss/package/x86_64/cartographer.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cartographer.html)) · [`cartographer-dev`](https://openqnx.github.io/oss/package/x86_64/cartographer-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cartographer-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add cartographer` (dependencies come along); one version: `apk add cartographer=<version>` | `apk del cartographer` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install cartographer` (dependencies first) | `oss-get remove cartographer` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`cartographer-dev` the same way, by its own name (`apk add cartographer-dev`, `oss-get install cartographer-dev`, by hand with its own file and list `cartographer-dev.list`); remove it before `cartographer`, which it needs.

```sh
# on the QNX target, as root
apk add cartographer cartographer-dev
apk add qnx-test-cartographer && /usr/local/bin/qnx-port-tests/test_cartographer    # its on-target test, from tests-8.0-<arch>
apk del cartographer cartographer-dev qnx-test-cartographer
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/cartographer.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/cartographer.list
rm var/lib/manual/cartographer.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cartographer` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (36 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild abseil boost ceres-solver eigen glog gflags lua5.3 protobuf cairo
```

<details><summary>Full build chain (36)</summary>

[`abseil`](../abseil/) [`boost`](../boost/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`ceres-solver`](../ceres-solver/) [`eigen`](../eigen/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`gflags`](../gflags/) [`glib`](../glib/) [`glog`](../glog/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lua5.3`](../lua5.3/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`protobuf`](../protobuf/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.0.0 --recurse-submodules --shallow-submodules \
    https://github.com/cartographer-project/cartographer.git src/cartographer
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cartographer < ports/cartographer/patches/0001-cmake-optional-tests-protobuf-config.patch
patch -p1 -d src/cartographer < ports/cartographer/patches/0002-absl-thread-annotations.patch
patch -p1 -d src/cartographer < ports/cartographer/patches/0003-ceres-2.2-manifolds.patch
```

**[`0001-cmake-optional-tests-protobuf-config.patch`](patches/0001-cmake-optional-tests-protobuf-config.patch)** — Build without the unit tests; find protobuf through its CMake package.

> The unit tests (some 150 programs and a test library) are always built and
> need GMock: CARTOGRAPHER_BUILD_TESTS (default on, as before) turns them off.
> find_package(Ceres COMPONENTS SuiteSparse) fails against a Ceres built
> without SuiteSparse, which Cartographer does not use: ask for Ceres alone.
> protobuf 22 and later are built on abseil: their headers and the generated
> code call abseil inline, so linking the bare libprotobuf found by CMake's
> FindProtobuf module leaves abseil symbols undefined. Use protobuf's own
> package (protobuf::libprotobuf) when there is one; the installed
> cartographer-config.cmake finds protobuf and Boost too, since the exported
> target names their imported targets. The include directories of Ceres and
> Boost come from their imported targets: an empty CERES_INCLUDE_DIRS put the
> source directory into the installed package's include path.

Touches: `CMakeLists.txt`, `cartographer-config.cmake.in` · Upstream status: Pending

**[`0002-absl-thread-annotations.patch`](patches/0002-absl-thread-annotations.patch)** — Use abseil's ABSL_-prefixed thread-safety annotations.

> GUARDED_BY, LOCKS_EXCLUDED and EXCLUSIVE_LOCKS_REQUIRED without the prefix
> came from absl/base/internal/thread_annotations.h, which absl/synchronization
> no longer includes (and newer abseil releases dropped): with abseil
> 20240722 every header using them fails to parse. ABSL_GUARDED_BY etc. are
> the same macros (absl/base/thread_annotations.h). Mechanical rename.

Touches: `cartographer/cloud/internal/map_builder_server.h`, `cartographer/common/internal/blocking_queue.h`, `cartographer/common/internal/testing/thread_pool_for_testing.cc`, `cartographer/common/internal/testing/thread_pool_for_testing.h`, `cartographer/common/task.h`, `cartographer/common/thread_pool.cc` … · Upstream status: Pending

**[`0003-ceres-2.2-manifolds.patch`](patches/0003-ceres-2.2-manifolds.patch)** — Use Ceres 2.2's Manifold API instead of LocalParameterization.

> Ceres 2.2 removed LocalParameterization and its subclasses. The
> replacements are one-for-one: QuaternionManifold, SubsetManifold and
> AutoDiffManifold; Problem::AddParameterBlock and SetManifold take a
> Manifold*. AutoDiffManifold's functor needs Plus() and Minus() instead of
> operator(): rotation_parameterization.h's two functors gain both (Minus()
> is the inverse of the existing update; operator() is kept for the tests).

Touches: `cartographer/mapping/internal/3d/rotation_parameterization.h`, `cartographer/mapping/internal/3d/scan_matching/ceres_scan_matcher_3d.cc`, `cartographer/mapping/internal/imu_based_pose_extrapolator.cc`, `cartographer/mapping/internal/optimization/ceres_pose.cc`, `cartographer/mapping/internal/optimization/ceres_pose.h`, `cartographer/mapping/internal/optimization/optimization_problem_2d.cc` … · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/cartographer -B build/$QNX_ARCH/cartographer -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_GRPC=OFF \
      -DBUILD_PROMETHEUS=OFF \
      -DCARTOGRAPHER_BUILD_TESTS=OFF \
      # \
      The \
      build \
      runs \
      protoc: \
      the \
      build \
      machine's, \
      of \
      the \
      staged \
      # \
      runtime's \
      version \
      (protobuf's \
      host \
      build). \
      -DPROTOBUF_PROTOC_EXECUTABLE="$(_host_protoc)" \
      -Dprotobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf" \
      -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/cartographer -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/cartographer
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Cartographer: Google's real-time 2D and 3D SLAM library (the C++ core: local scan matching, submaps, pose-graph optimisation with Ceres, Lua configuration, protobuf serialisation) -- a static library, as upstream builds it (its internal headers are not installed) -- and its tools. No gRPC (cloud) service, no Prometheus metrics, no unit tests. 2.0.0 is the last release (2021): it predates protobuf 22+ (patches/0001), abseil 2024 (0002) and Ceres 2.2 (0003), which this tree has. Lua 5.3 for the configuration files, as Debian builds it. Boost: iostreams (gzip streams of the .pbstream files); cairo: the map images.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cartographer`.

### 5. What it installs

149 files under `/usr/local`, 4 programs, 129 headers.

```
/usr/local/bin/cartographer_autogenerate_ground_truth
/usr/local/bin/cartographer_compute_relations_metrics
/usr/local/bin/cartographer_pbstream
/usr/local/bin/cartographer_print_configuration
```

The complete list is `stage/<arch>/.manifest/cartographer` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cartographer
ls dists/8.0/*/ | grep '^cartographer-'
```

This writes `cartographer-<ver>-r0.tar.gz` (run time) and `cartographer-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cartographer
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cartographer-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cartographer-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cartographer`](../../tests/src/) links cartographer the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cartographer`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cartographer
/usr/local/bin/qnx-port-tests/test_cartographer
```

```
== cartographer ==
  ok   ...
PASS cartographer
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cartographer` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cartographer`](../../tests/src/) -- a program linked against cartographer the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - Rigid2d composition
   - Embed3D, GetYaw, inverse
   - Lua dictionary: arithmetic, strings, nested tables
   - Lua include of the installed map_builder.lua
   - probability grid stores a cell; unknown cells read the minimum
   - cell centre is within half a cell of the point
   - range data inserter: wall cells hit (p = hit_probability 0.55)
   - range data inserter: cells along the rays free (p = miss_probability 0.49)
   - range data inserter: cells behind the wall stay unknown
   - 2D trajectory builder created
   - a local SLAM result for every scan
   - local SLAM corrects the odometry onto the true trajectory (5 cm, 1 deg)
   - the motion filter inserts a node every 10 cm
   - pose graph holds the nodes and a submap
   - optimised global node poses: within half the odometry's error of the truth

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cartographer 2.0.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/cartographer-2.0.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cartographer 2.0.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/cartographer-2.0.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.0.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cartographer=<version>`), sources in `ported/cartographer/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cartographer`; port one with `scripts/port-update cartographer`.

<details><summary>recipe.sh</summary>

```bash
# Cartographer: Google's real-time 2D and 3D SLAM library (the C++ core:
# local scan matching, submaps, pose-graph optimisation with Ceres, Lua
# configuration, protobuf serialisation) -- a static library, as upstream
# builds it (its internal headers are not installed) -- and its tools. No
# gRPC (cloud) service, no Prometheus metrics, no unit tests.
# 2.0.0 is the last release (2021): it predates protobuf 22+ (patches/0001),
# abseil 2024 (0002) and Ceres 2.2 (0003), which this tree has. Lua 5.3 for
# the configuration files, as Debian builds it. Boost: iostreams (gzip
# streams of the .pbstream files); cairo: the map images.
VERSION=2.0.0; GIT=https://github.com/cartographer-project/cartographer.git; REF=2.0.0
DEPS="abseil boost ceres-solver eigen glog gflags lua5.3 protobuf cairo"
_host_protoc() { find "$BUILD_DIR/host/protobuf" -maxdepth 1 -name 'protoc*' -type f -perm -u+x | head -1; }
CMAKE_ARGS_EXTRA=( -DBUILD_GRPC=OFF -DBUILD_PROMETHEUS=OFF
                   -DCARTOGRAPHER_BUILD_TESTS=OFF
                   # The build runs protoc: the build machine's, of the staged
                   # runtime's version (protobuf's host build).
                   -DPROTOBUF_PROTOC_EXECUTABLE="$(_host_protoc)"
                   -Dprotobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf"
                   -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl"
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
