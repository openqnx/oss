# tensorflow-lite 2.16.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/tensorflow/tensorflow.git (tag `v2.16.1`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_tflite`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install tensorflow-lite
oss-get install tensorflow-lite-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tensorflow-lite` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2.16.1 --recurse-submodules --shallow-submodules \
    https://github.com/tensorflow/tensorflow.git src/tensorflow-lite
```

The project lives in the `tensorflow/lite/` subdirectory of the repository.

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/tensorflow-lite < ports/tensorflow-lite/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add qnx port

> ---
>  .../lite/cmake/DownloadPThreadPool.cmake      |  1 +
>  tensorflow/lite/kernels/CMakeLists.txt        | 23 +++++++++++++++---
>  .../lite/tools/cmake/modules/Findre2.cmake    |  2 +-
>  .../lite/tools/cmake/modules/abseil-cpp.cmake |  1 +
>  .../lite/tools/cmake/modules/eigen.cmake      |  1 +
>  .../cmake/modules/ml_dtypes/CMakeLists.txt    |  4 ++--
>  tensorflow/lite/tools/cmake/modules/ruy.cmake |  1 +
>  .../native_tools/flatbuffers/CMakeLists.txt   |  2 +-
>  tensorflow/lite/tools/evaluation/utils.cc     | 24 +++++++++++++++++++
>  9 files changed, 52 insertions(+), 7 deletions(-)

Touches: `tensorflow/lite/cmake/DownloadPThreadPool.cmake`, `tensorflow/lite/kernels/CMakeLists.txt`, `tensorflow/lite/tools/cmake/modules/Findre2.cmake`, `tensorflow/lite/tools/cmake/modules/abseil-cpp.cmake`, `tensorflow/lite/tools/cmake/modules/eigen.cmake`, `tensorflow/lite/tools/cmake/modules/ml_dtypes/CMakeLists.txt` … · Upstream status: QNX-specific (from qnx-ports)

More QNX changes, outside the main source tree:

- [`deps/`](deps/) — 11 patches for dependencies the build fetches itself (applied by the recipe)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tensorflow-lite/tensorflow/lite; B=build/$QNX_ARCH/tensorflow-lite; mkdir -p $B
```

```bash
do_patch() {
  local line name url commit dir p
  for line in $TFL_DEPS; do
    IFS='|' read -r name url commit <<<"$line"
    dir="$SRCROOT/_qnx_deps/$name"
    git init -q "$dir" && git -C "$dir" remote add origin "$url" && \
      git -C "$dir" fetch -q --depth 1 origin "$commit" && \
      git -C "$dir" -c advice.detachedHead=false checkout -q FETCH_HEAD || return 1
    for p in "$RECIPE_DIR/tensorflow-lite/deps/$name"/*.patch; do
      [ -e "$p" ] || continue
      echo "-- patch $name: $(basename "$p")"
      patch -p1 -d "$dir" < "$p" || return 1
    done
  done
  # Eigen as the archive of the commit TensorFlow pins (tools/cmake/modules/
  # eigen.cmake): cloning
  # gitlab.com over git kept failing ("GitLab is currently unable to handle
  # this request"), and a FETCHCONTENT_SOURCE_DIR also skips FetchContent's
  # own patch step.
  local t="$WS/dist/eigen-$TFL_EIGEN_COMMIT.tar.gz" e="$SRCROOT/_qnx_deps/eigen"
  [ -f "$t" ] || curl -fsSL -o "$t" "https://gitlab.com/libeigen/eigen/-/archive/$TFL_EIGEN_COMMIT/eigen-$TFL_EIGEN_COMMIT.tar.gz" || return 1
  echo "$TFL_EIGEN_SHA256  $t" | sha256sum -c --status || { echo "checksum mismatch: $t"; rm -f "$t"; return 1; }
  # deps/fetchcontent/eigen.patch is not applied: it adds the
  # numext::get_integer_by_size helpers, which Eigen has itself at the commit
  # TensorFlow 2.16.1 pins -- applied (with fuzz) it defined them twice.
  mkdir -p "$e" && tar xzf "$t" -C "$e" --strip-components=1
}
```

```bash
do_configure() {
  local d="$SRCROOT/_qnx_deps"
  # The defines qnx-ports' build passes too: farmhash otherwise looks for
  # <endian.h>, which QNX does not have. CFLAGS/CXXFLAGS add to the
  # toolchain's flags (CMAKE_*_FLAGS_INIT) rather than replacing them.
  CFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" CXXFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" \
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TFL_PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_RPATH='$ORIGIN' \
    -DTFLITE_ENABLE_INSTALL=ON -DABSL_ENABLE_INSTALL=ON -DBUILD_SHARED_LIBS=ON \
    -DTFLITE_ENABLE_XNNPACK=ON -DXNNPACK_ENABLE_ASSEMBLY=OFF -DTFLITE_KERNEL_TEST=OFF \
    -DTFLITE_ENABLE_GPU=OFF -DTFLITE_ENABLE_NNAPI=OFF \
    -DQNX_PATCH_DIR="$RECIPE_DIR/tensorflow-lite/deps/fetchcontent" \
    -DXNNPACK_PATCH_CMD="patch;-p1;-i;$RECIPE_DIR/tensorflow-lite/deps/fetchcontent/XNNPACK.patch" \
    -DPTHREADPOOL_PATCH_CMD="patch;-p1;-i;$RECIPE_DIR/tensorflow-lite/deps/fetchcontent/pthreadpool.patch" \
    -DFETCHCONTENT_SOURCE_DIR_ABSEIL-CPP="$d/abseil-cpp" \
    -DFETCHCONTENT_SOURCE_DIR_CPUINFO="$d/cpuinfo" \
    -DFETCHCONTENT_SOURCE_DIR_RUY="$d/ruy" \
    -DFETCHCONTENT_SOURCE_DIR_FARMHASH="$d/farmhash" \
    -DFETCHCONTENT_SOURCE_DIR_EIGEN="$d/eigen" \
    -DFETCHCONTENT_SOURCE_DIR_ML_DTYPES="$d/ml_dtypes" \
    -DPython3_EXECUTABLE=/usr/bin/python3
}
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  find "$B/_deps" -name 'libXNNPACK.so*' -exec cp -a {} "$STAGE$TFL_PREFIX/lib/" \;
  [ -e "$STAGE$TFL_PREFIX/lib/libXNNPACK.so" ]
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- TensorFlow Lite (the C/C++ inference runtime, tensorflow/lite), the release qnx-ports ports. patches/ holds TensorFlow's own QNX changes. TFLite's CMake downloads its dependencies at configure time (FetchContent), pinned to exact commits. Their QNX changes come in two kinds: - deps/fetchcontent/{pthreadpool,XNNPACK}.patch: applied by TensorFlow's own download step (the QNX patch adds PATCH_COMMANDs that read them from QNX_PATCH_DIR) -- from qnx-ports/build-files. - deps/<name>/*.patch for abseil-cpp, cpuinfo and farmhash, which qnx-ports replaces with its forks: here the dependency is fetched at the commit TensorFlow pins, these patches (the forks' QNX commits) are applied, and FetchContent is pointed at that tree. cpuinfo is the exception: its fork's QNX commits are based on 3c8b1533 (April 2024), newer than the commit TensorFlow pins, and do not apply to that one -- so it is fetched at the fork's base, as ports/cpuinfo is. ruy builds unchanged (the fork's only change links its tests with libregex).
- name|repository|commit TensorFlow 2.16.1 pins (tensorflow/lite/tools/cmake/modules), except cpuinfo (see above). ml_dtypes is fetched here too, without its git submodules: its third_party/eigen is a gitlab.com submodule (see the Eigen note below), and TFLite only uses ml_dtypes' headers, with its own Eigen.
- The QNX patch makes the XNNPACK and pthreadpool downloads run ${XNNPACK_PATCH_CMD} / ${PTHREADPOOL_PATCH_CMD} but defines neither, so the fetched XNNPACK went unpatched ("Unrecognized CMAKE_SYSTEM_NAME value QNX"). They are given here, as CMake lists. TFLite builds the abseil, Eigen, flatbuffers, ruy, cpuinfo, ... it pins and installs them with itself (its CMake package needs them exported); in the common prefix they overwrote the abseil, eigen, flatbuffers, ... components (abseil 2308 headers and dev links over 20240722). It gets a prefix of its own instead, as ROS does, with $ORIGIN as the run path so its libraries find the copies beside them: /usr/local/opt/tensorflow-lite (link with -L.../opt/tensorflow-lite/lib, or point CMake at its lib/cmake).
- TFLite links the XNNPACK it builds (libXNNPACK.so) but its install rules leave that library out. In the common prefix the xnnpack component's libXNNPACK.so stood in for it -- another XNNPACK version, short of what TFLite calls ("undefined reference to xnn_experiment_enable_adaptive_avx_optimization"). Install its own.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tensorflow-lite`.

### 5. What it installs

1838 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/tensorflow-lite` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH tensorflow-lite
ls repo/8.0/*/ | grep '^tensorflow-lite-'
```

This writes `tensorflow-lite-<ver>-r0.tar.gz` (run time) and `tensorflow-lite-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install tensorflow-lite
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/tensorflow-lite-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/tensorflow-lite-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_tflite`](../../tests/src/) links tensorflow-lite the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tflite root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_tflite'
```

```
== tensorflow-lite ==
  ok   ...
PASS tensorflow-lite
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.16.1; GIT=https://github.com/tensorflow/tensorflow.git; REF=v2.16.1
# TensorFlow Lite (the C/C++ inference runtime, tensorflow/lite), the release
# qnx-ports ports. patches/ holds TensorFlow's own QNX changes.
#
# TFLite's CMake downloads its dependencies at configure time (FetchContent),
# pinned to exact commits. Their QNX changes come in two kinds:
#  - deps/fetchcontent/{pthreadpool,XNNPACK}.patch: applied by
#    TensorFlow's own download step (the QNX patch adds PATCH_COMMANDs that read
#    them from QNX_PATCH_DIR) -- from qnx-ports/build-files.
#  - deps/<name>/*.patch for abseil-cpp, cpuinfo and farmhash, which
#    qnx-ports replaces with its forks: here the dependency is fetched at the
#    commit TensorFlow pins, these patches (the forks' QNX commits) are applied,
#    and FetchContent is pointed at that tree. cpuinfo is the exception: its
#    fork's QNX commits are based on 3c8b1533 (April 2024), newer than the
#    commit TensorFlow pins, and do not apply to that one -- so it is fetched
#    at the fork's base, as ports/cpuinfo is. ruy builds unchanged (the fork's
#    only change links its tests with libregex).
BUILDSYS=custom
SUBDIR=tensorflow/lite
# name|repository|commit TensorFlow 2.16.1 pins (tensorflow/lite/tools/cmake/modules),
# except cpuinfo (see above). ml_dtypes is fetched here too, without its git
# submodules: its third_party/eigen is a gitlab.com submodule (see the Eigen
# note below), and TFLite only uses ml_dtypes' headers, with its own Eigen.
TFL_DEPS="abseil-cpp|https://github.com/abseil/abseil-cpp|fb3621f4f897824c0dbe0615fa94543df6192f30
cpuinfo|https://github.com/pytorch/cpuinfo|3c8b1533ac03dd6531ab6e7b9245d488f13a82a5
ruy|https://github.com/google/ruy|3286a34cc8de6149ac6844107dfdffac91531e72
farmhash|https://github.com/google/farmhash|0d859a811870d10f53a594927d0d0b97573ad06d
ml_dtypes|https://github.com/jax-ml/ml_dtypes|780b6d0ee01ffbfac45f7ec5418bc08f2b166483"

do_patch() {
  local line name url commit dir p
  for line in $TFL_DEPS; do
    IFS='|' read -r name url commit <<<"$line"
    dir="$SRCROOT/_qnx_deps/$name"
    git init -q "$dir" && git -C "$dir" remote add origin "$url" && \
      git -C "$dir" fetch -q --depth 1 origin "$commit" && \
      git -C "$dir" -c advice.detachedHead=false checkout -q FETCH_HEAD || return 1
    for p in "$RECIPE_DIR/tensorflow-lite/deps/$name"/*.patch; do
      [ -e "$p" ] || continue
      echo "-- patch $name: $(basename "$p")"
      patch -p1 -d "$dir" < "$p" || return 1
    done
  done
  # Eigen as the archive of the commit TensorFlow pins (tools/cmake/modules/
  # eigen.cmake): cloning
  # gitlab.com over git kept failing ("GitLab is currently unable to handle
  # this request"), and a FETCHCONTENT_SOURCE_DIR also skips FetchContent's
  # own patch step.
  local t="$WS/dist/eigen-$TFL_EIGEN_COMMIT.tar.gz" e="$SRCROOT/_qnx_deps/eigen"
  [ -f "$t" ] || curl -fsSL -o "$t" "https://gitlab.com/libeigen/eigen/-/archive/$TFL_EIGEN_COMMIT/eigen-$TFL_EIGEN_COMMIT.tar.gz" || return 1
  echo "$TFL_EIGEN_SHA256  $t" | sha256sum -c --status || { echo "checksum mismatch: $t"; rm -f "$t"; return 1; }
  # deps/fetchcontent/eigen.patch is not applied: it adds the
  # numext::get_integer_by_size helpers, which Eigen has itself at the commit
  # TensorFlow 2.16.1 pins -- applied (with fuzz) it defined them twice.
  mkdir -p "$e" && tar xzf "$t" -C "$e" --strip-components=1
}
TFL_EIGEN_COMMIT=aa6964bf3a34fd607837dd8123bc42465185c4f8
TFL_EIGEN_SHA256=35ba771e30c735a4215ed784d7e032086cf89fe6622dce4d793c45dd74373362

# The QNX patch makes the XNNPACK and pthreadpool downloads run
# ${XNNPACK_PATCH_CMD} / ${PTHREADPOOL_PATCH_CMD} but defines neither, so the
# fetched XNNPACK went unpatched ("Unrecognized CMAKE_SYSTEM_NAME value QNX").
# They are given here, as CMake lists.
# TFLite builds the abseil, Eigen, flatbuffers, ruy, cpuinfo, ... it pins and
# installs them with itself (its CMake package needs them exported); in the
# common prefix they overwrote the abseil, eigen, flatbuffers, ... components
# (abseil 2308 headers and dev links over 20240722). It gets a prefix of its
# own instead, as ROS does, with $ORIGIN as the run path so its libraries find
# the copies beside them: /usr/local/opt/tensorflow-lite (link with
# -L.../opt/tensorflow-lite/lib, or point CMake at its lib/cmake).
TFL_PREFIX="$PREFIX/opt/tensorflow-lite"
do_configure() {
  local d="$SRCROOT/_qnx_deps"
  # The defines qnx-ports' build passes too: farmhash otherwise looks for
  # <endian.h>, which QNX does not have. CFLAGS/CXXFLAGS add to the
  # toolchain's flags (CMAKE_*_FLAGS_INIT) rather than replacing them.
  CFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" CXXFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" \
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TFL_PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_RPATH='$ORIGIN' \
    -DTFLITE_ENABLE_INSTALL=ON -DABSL_ENABLE_INSTALL=ON -DBUILD_SHARED_LIBS=ON \
    -DTFLITE_ENABLE_XNNPACK=ON -DXNNPACK_ENABLE_ASSEMBLY=OFF -DTFLITE_KERNEL_TEST=OFF \
    -DTFLITE_ENABLE_GPU=OFF -DTFLITE_ENABLE_NNAPI=OFF \
    -DQNX_PATCH_DIR="$RECIPE_DIR/tensorflow-lite/deps/fetchcontent" \
    -DXNNPACK_PATCH_CMD="patch;-p1;-i;$RECIPE_DIR/tensorflow-lite/deps/fetchcontent/XNNPACK.patch" \
    -DPTHREADPOOL_PATCH_CMD="patch;-p1;-i;$RECIPE_DIR/tensorflow-lite/deps/fetchcontent/pthreadpool.patch" \
    -DFETCHCONTENT_SOURCE_DIR_ABSEIL-CPP="$d/abseil-cpp" \
    -DFETCHCONTENT_SOURCE_DIR_CPUINFO="$d/cpuinfo" \
    -DFETCHCONTENT_SOURCE_DIR_RUY="$d/ruy" \
    -DFETCHCONTENT_SOURCE_DIR_FARMHASH="$d/farmhash" \
    -DFETCHCONTENT_SOURCE_DIR_EIGEN="$d/eigen" \
    -DFETCHCONTENT_SOURCE_DIR_ML_DTYPES="$d/ml_dtypes" \
    -DPython3_EXECUTABLE=/usr/bin/python3
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
# TFLite links the XNNPACK it builds (libXNNPACK.so) but its install rules
# leave that library out. In the common prefix the xnnpack component's
# libXNNPACK.so stood in for it -- another XNNPACK version, short of what
# TFLite calls ("undefined reference to
# xnn_experiment_enable_adaptive_avx_optimization"). Install its own.
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  find "$B/_deps" -name 'libXNNPACK.so*' -exec cp -a {} "$STAGE$TFL_PREFIX/lib/" \;
  [ -e "$STAGE$TFL_PREFIX/lib/libXNNPACK.so" ]
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
