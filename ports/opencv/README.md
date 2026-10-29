# opencv 4.10.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/opencv/opencv.git (tag `4.10.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`eigen`](../eigen/) |
| Run-time dependencies | [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_opencv`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install opencv
oss-get install opencv-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild opencv` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib eigen
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 4.10.0 --recurse-submodules --shallow-submodules \
    https://github.com/opencv/opencv.git src/opencv
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/opencv -B build/$QNX_ARCH/opencv -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DBUILD_PERF_TESTS=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_DOCS=OFF \
      -DBUILD_JAVA=OFF \
      -DBUILD_opencv_apps=OFF \
      -DBUILD_opencv_python2=OFF \
      -DBUILD_opencv_python3=OFF \
      # \
      find_host_package(PythonInterp \
      OFF) \
      trips \
      OpenCV's \
      own \
      macro \
      under \
      # \
      CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY; \
      no \
      Python \
      is \
      wanted \
      anyway \
      -DOPENCV_PYTHON_SKIP_DETECTION=ON \
      -DBUILD_opencv_python_bindings_generator=OFF \
      # \
      QNX \
      has \
      no \
      V4L/GTK/X11 \
      desktop \
      capture \
      stack \
      -DWITH_V4L=OFF \
      -DWITH_GTK=OFF \
      -DWITH_QT=OFF \
      -DWITH_WIN32UI=OFF \
      -DWITH_FFMPEG=OFF \
      -DWITH_GSTREAMER=OFF \
      -DWITH_1394=OFF \
      -DWITH_OPENCL=OFF \
      -DWITH_CUDA=OFF \
      -DWITH_IPP=OFF \
      -DWITH_LAPACK=OFF \
      -DWITH_PROTOBUF=OFF \
      -DWITH_ITT=OFF \
      -DWITH_TBB=OFF \
      -DWITH_OPENEXR=OFF \
      -DWITH_JASPER=OFF \
      # \
      VA-API \
      is \
      auto-enabled \
      on \
      x86_64; \
      QNX \
      has \
      no \
      libva \
      -DWITH_VA=OFF \
      -DWITH_VA_INTEL=OFF \
      -DWITH_WEBP=OFF \
      -DWITH_TIFF=OFF \
      -DWITH_OPENJPEG=OFF \
      -DBUILD_ZLIB=OFF \
      -DBUILD_PNG=ON \
      -DBUILD_JPEG=ON \
      -DWITH_EIGEN=ON \
      -DOPENCV_GENERATE_PKGCONFIG=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/opencv -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/opencv
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH opencv`.

### 5. What it installs

438 files under `/usr/local`: 45 shared-library entries, 1 programs, 353 headers.

```
/usr/local/lib/libopencv_calib3d.so
/usr/local/lib/libopencv_calib3d.so.4.10.0
/usr/local/lib/libopencv_calib3d.so.410
/usr/local/lib/libopencv_core.so
/usr/local/lib/libopencv_core.so.4.10.0
/usr/local/lib/libopencv_core.so.410
/usr/local/lib/libopencv_dnn.so
/usr/local/lib/libopencv_dnn.so.4.10.0
/usr/local/lib/libopencv_dnn.so.410
/usr/local/lib/libopencv_features2d.so
/usr/local/lib/libopencv_features2d.so.4.10.0
/usr/local/lib/libopencv_features2d.so.410
/usr/local/lib/libopencv_flann.so
/usr/local/lib/libopencv_flann.so.4.10.0
/usr/local/lib/libopencv_flann.so.410
/usr/local/lib/libopencv_gapi.so
...
```

The complete list is `stage/<arch>/.manifest/opencv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH opencv
ls repo/8.0/*/ | grep '^opencv-'
```

This writes `opencv-<ver>-r0.tar.gz` (run time) and `opencv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install opencv
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/opencv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/opencv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_opencv`](../../tests/src/) links opencv the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_opencv root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_opencv'
```

```
== opencv ==
  ok   ...
PASS opencv
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.10.0; GIT=https://github.com/opencv/opencv.git; REF=4.10.0
DEPS="zlib eigen"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF
                   -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_JAVA=OFF
                   -DBUILD_opencv_apps=OFF -DBUILD_opencv_python2=OFF
                   -DBUILD_opencv_python3=OFF
                   # find_host_package(PythonInterp OFF) trips OpenCV's own macro under
                   # CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY; no Python is wanted anyway
                   -DOPENCV_PYTHON_SKIP_DETECTION=ON
                   -DBUILD_opencv_python_bindings_generator=OFF
                   # QNX has no V4L/GTK/X11 desktop capture stack
                   -DWITH_V4L=OFF -DWITH_GTK=OFF -DWITH_QT=OFF -DWITH_WIN32UI=OFF
                   -DWITH_FFMPEG=OFF -DWITH_GSTREAMER=OFF -DWITH_1394=OFF
                   -DWITH_OPENCL=OFF -DWITH_CUDA=OFF -DWITH_IPP=OFF
                   -DWITH_LAPACK=OFF -DWITH_PROTOBUF=OFF -DWITH_ITT=OFF
                   -DWITH_TBB=OFF -DWITH_OPENEXR=OFF -DWITH_JASPER=OFF
                   # VA-API is auto-enabled on x86_64; QNX has no libva
                   -DWITH_VA=OFF -DWITH_VA_INTEL=OFF
                   -DWITH_WEBP=OFF -DWITH_TIFF=OFF -DWITH_OPENJPEG=OFF
                   -DBUILD_ZLIB=OFF -DBUILD_PNG=ON -DBUILD_JPEG=ON
                   -DWITH_EIGEN=ON -DOPENCV_GENERATE_PKGCONFIG=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
