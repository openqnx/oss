# pcl 1.14.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/PointCloudLibrary/pcl.git (tag `pcl-1.14.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`boost`](../boost/) [`eigen`](../eigen/) [`flann`](../flann/) |
| Run-time dependencies | [`boost`](../boost/) [`flann`](../flann/) [`lz4`](../lz4/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pcl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pcl
apk add pcl-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-pcl && /usr/local/bin/qnx-port-tests/test_pcl    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pcl`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pcl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild boost eigen flann
```

<details><summary>Full build chain (11)</summary>

[`boost`](../boost/) [`bzip2`](../bzip2/) [`eigen`](../eigen/) [`flann`](../flann/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b pcl-1.14.1 --recurse-submodules --shallow-submodules \
    https://github.com/PointCloudLibrary/pcl.git src/pcl
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/pcl -B build/$QNX_ARCH/pcl -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      # \
      no \
      VTK/OpenGL/X11 \
      on \
      a \
      QNX \
      target \
      -- \
      visualization \
      is \
      out \
      -DWITH_VTK=OFF \
      -DBUILD_visualization=OFF \
      -DWITH_OPENGL=OFF \
      -DWITH_QT=OFF \
      -DWITH_PCAP=OFF \
      -DWITH_PNG=OFF \
      -DWITH_QHULL=OFF \
      -DBUILD_surface_on_nurbs=OFF \
      -DWITH_OPENNI=OFF \
      -DWITH_OPENNI2=OFF \
      -DWITH_ENSENSO=OFF \
      -DWITH_DAVIDSDK=OFF \
      -DWITH_DSSDK=OFF \
      -DWITH_RSSDK=OFF \
      -DWITH_LIBUSB=OFF \
      -DWITH_OPENMP=OFF \
      -DWITH_CUDA=OFF \
      -DBUILD_apps=OFF \
      -DBUILD_examples=OFF \
      -DBUILD_tools=OFF \
      -DBUILD_global_tests=OFF \
      -DBOOST_ROOT="$STAGE$PREFIX" \
      -DEigen3_DIR="$STAGE$PREFIX/share/eigen3/cmake" \
      -DFLANN_ROOT="$STAGE$PREFIX" \
      # \
      PCL \
      probes \
      aligned \
      allocation \
      with \
      check_cxx_source_runs, \
      # \
      which \
      can \
      never \
      succeed \
      when \
      cross-compiling \
      -- \
      CMake \
      marks \
      # \
      it \
      FAILED_TO_RUN \
      and \
      pcl_macros.h \
      then \
      #errors \
      with \
      # \
      aligned_malloc \
      not \
      supported \
      on \
      your \
      platform \
      . \
      QNX \
      does \
      # \
      provide \
      posix_memalign \
      (verified \
      against \
      the \
      SDP), \
      so \
      seed \
      # \
      the \
      run \
      result \
      instead \
      of \
      letting \
      the \
      probe \
      decide. \
      -DHAVE_POSIX_MEMALIGN=1 \
      -DHAVE_POSIX_MEMALIGN_EXITCODE=0 \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/pcl -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/pcl
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pcl`.

### 5. What it installs

926 files under `/usr/local`: 51 shared-library entries, 838 headers.

```
/usr/local/lib/libpcl_common.so
/usr/local/lib/libpcl_common.so.1.14
/usr/local/lib/libpcl_common.so.1.14.1
/usr/local/lib/libpcl_features.so
/usr/local/lib/libpcl_features.so.1.14
/usr/local/lib/libpcl_features.so.1.14.1
/usr/local/lib/libpcl_filters.so
/usr/local/lib/libpcl_filters.so.1.14
/usr/local/lib/libpcl_filters.so.1.14.1
/usr/local/lib/libpcl_io.so
/usr/local/lib/libpcl_io.so.1.14
/usr/local/lib/libpcl_io.so.1.14.1
/usr/local/lib/libpcl_io_ply.so
/usr/local/lib/libpcl_io_ply.so.1.14
/usr/local/lib/libpcl_io_ply.so.1.14.1
/usr/local/lib/libpcl_kdtree.so
...
```

The complete list is `stage/<arch>/.manifest/pcl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pcl
ls dists/8.0/*/ | grep '^pcl-'
```

This writes `pcl-<ver>-r0.tar.gz` (run time) and `pcl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pcl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pcl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pcl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pcl`](../../tests/src/) links pcl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pcl`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pcl
/usr/local/bin/qnx-port-tests/test_pcl
```

```
== pcl ==
  ok   ...
PASS pcl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pcl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pcl`](../../tests/src/) -- a program linked against pcl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - cloud populated
   - getMinMax3D
   - voxel grid downsampled
   - kd-tree knn search returned 5 neighbours
   - neighbours sorted by distance

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.14.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pcl=<version>`), sources in `ported/pcl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pcl`; port one with `scripts/port-update pcl`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.14.1; GIT=https://github.com/PointCloudLibrary/pcl.git; REF=pcl-1.14.1
DEPS="boost eigen flann"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON
                   # no VTK/OpenGL/X11 on a QNX target -- visualization is out
                   -DWITH_VTK=OFF -DBUILD_visualization=OFF -DWITH_OPENGL=OFF
                   -DWITH_QT=OFF -DWITH_PCAP=OFF -DWITH_PNG=OFF
                   -DWITH_QHULL=OFF -DBUILD_surface_on_nurbs=OFF
                   -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_ENSENSO=OFF
                   -DWITH_DAVIDSDK=OFF -DWITH_DSSDK=OFF -DWITH_RSSDK=OFF
                   -DWITH_LIBUSB=OFF -DWITH_OPENMP=OFF -DWITH_CUDA=OFF
                   -DBUILD_apps=OFF -DBUILD_examples=OFF -DBUILD_tools=OFF
                   -DBUILD_global_tests=OFF
                   -DBOOST_ROOT="$STAGE$PREFIX"
                   -DEigen3_DIR="$STAGE$PREFIX/share/eigen3/cmake"
                   -DFLANN_ROOT="$STAGE$PREFIX"
                   # PCL probes aligned allocation with check_cxx_source_runs,
                   # which can never succeed when cross-compiling -- CMake marks
                   # it FAILED_TO_RUN and pcl_macros.h then #errors with
                   # "aligned_malloc not supported on your platform". QNX does
                   # provide posix_memalign (verified against the SDP), so seed
                   # the run result instead of letting the probe decide.
                   -DHAVE_POSIX_MEMALIGN=1
                   -DHAVE_POSIX_MEMALIGN_EXITCODE=0
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
