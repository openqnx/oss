# tensorflow-lite 2.16.1

> TensorFlow Lite C/C++ runtime for machine learning inference

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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add tensorflow-lite
apk add tensorflow-lite-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-tflite && /usr/local/bin/qnx-port-tests/test_tflite    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install tensorflow-lite`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
  _tfl_private_names > "$B/qnx-private-names.cmake" || return 1
  CFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" CXXFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" \
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_PROJECT_tensorflow-lite_INCLUDE="$B/qnx-private-names.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TFL_PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_RPATH='$ORIGIN' -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
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
  find "$B/_deps" -name 'libtflite-XNNPACK.so*' -exec cp -a {} "$STAGE$TFL_PREFIX/lib/" \;
  [ -e "$STAGE$TFL_PREFIX/lib/libtflite-XNNPACK.so" ]
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- TensorFlow Lite (the C/C++ inference runtime, tensorflow/lite), the release qnx-ports ports. patches/ holds TensorFlow's own QNX changes. TFLite's CMake downloads its dependencies at configure time (FetchContent), pinned to exact commits. Their QNX changes come in two kinds: - deps/fetchcontent/{pthreadpool,XNNPACK}.patch: applied by TensorFlow's own download step (the QNX patch adds PATCH_COMMANDs that read them from QNX_PATCH_DIR) -- from qnx-ports/build-files. - deps/<name>/*.patch for abseil-cpp, cpuinfo and farmhash, which qnx-ports replaces with its forks: here the dependency is fetched at the commit TensorFlow pins, these patches (the forks' QNX commits) are applied, and FetchContent is pointed at that tree. cpuinfo is the exception: its fork's QNX commits are based on 3c8b1533 (April 2024), newer than the commit TensorFlow pins, and do not apply to that one -- so it is fetched at the fork's base, as ports/cpuinfo is. ruy builds unchanged (the fork's only change links its tests with libregex).
- name|repository|commit TensorFlow 2.16.1 pins (tensorflow/lite/tools/cmake/modules), except cpuinfo (see above). ml_dtypes is fetched here too, without its git submodules: its third_party/eigen is a gitlab.com submodule (see the Eigen note below), and TFLite only uses ml_dtypes' headers, with its own Eigen.
- The QNX patch makes the XNNPACK and pthreadpool downloads run ${XNNPACK_PATCH_CMD} / ${PTHREADPOOL_PATCH_CMD} but defines neither, so the fetched XNNPACK went unpatched ("Unrecognized CMAKE_SYSTEM_NAME value QNX"). They are given here, as CMake lists. TFLite builds the abseil, Eigen, flatbuffers, ruy, cpuinfo, ... it pins and installs them with itself (its CMake package needs them exported); in the common prefix they overwrote the abseil, eigen, flatbuffers, ... components (abseil 2308 headers and dev links over 20240722). It gets a prefix of its own instead, as ROS does, with $ORIGIN as the run path so its libraries find the copies beside them: /usr/local/opt/tensorflow-lite (link with -L.../opt/tensorflow-lite/lib, or point CMake at its lib/cmake).
- TFLite's XNNPACK, pthreadpool and farmhash are shared libraries with the file names -- and sonames -- of the xnnpack, pthreadpool and farmhash components, which are other versions of the same code. QNX's loader puts LD_LIBRARY_PATH ahead of a library's own run path (DT_RPATH or DT_RUNPATH), so with /usr/local/lib on it TFLite would load those instead. TFLite's are renamed libtflite-<name>.so, after its CMakeLists has created them (a deferred call, from a project include). Linking them statically instead is refused by TFLite's exported CMake package.
- TFLite's install rules also leave its XNNPACK library out; in the common prefix the xnnpack component's libXNNPACK.so stood in for it ("undefined reference to xnn_experiment_enable_adaptive_avx_optimization"). Install its own, which has the install run path already (CMAKE_BUILD_WITH_INSTALL_RPATH).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tensorflow-lite`.

### 5. What it installs

1838 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/tensorflow-lite` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tensorflow-lite
ls dists/8.0/*/ | grep '^tensorflow-lite-'
```

This writes `tensorflow-lite-<ver>-r0.tar.gz` (run time) and `tensorflow-lite-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tensorflow-lite
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tensorflow-lite-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tensorflow-lite-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_tflite`](../../tests/src/) links tensorflow-lite the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-tflite`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-tflite
/usr/local/bin/qnx-port-tests/test_tflite
```

```
== tensorflow-lite ==
  ok   ...
PASS tensorflow-lite
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tflite` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_tflite`](../../tests/src/) -- a program linked against tensorflow-lite the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - builtin ADD kernel registered
   - add 3 tensors
   - set inputs
   - set outputs
   - tensor parameters
   - add ADD node
   - allocate tensors
   - invoke
   - a + b computed

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64 virt, vanilla mkqnximage image | apk:local-dists | tensorflow-lite 2.16.1-r1 | ✅ PASS | 2026-10-30 | [log](../../tests/logs/aarch64le/tensorflow-lite-2.16.1-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tensorflow-lite 2.16.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/tensorflow-lite-2.16.1-r1.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tensorflow-lite 2.16.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/tensorflow-lite-2.16.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.16.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tensorflow-lite=<version>`), sources in `ported/tensorflow-lite/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tensorflow-lite`; port one with `scripts/port-update tensorflow-lite`.

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
  _tfl_private_names > "$B/qnx-private-names.cmake" || return 1
  CFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" CXXFLAGS="-DFARMHASH_LITTLE_ENDIAN -D__LITTLE_ENDIAN__" \
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_PROJECT_tensorflow-lite_INCLUDE="$B/qnx-private-names.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TFL_PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_RPATH='$ORIGIN' -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
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
# TFLite's XNNPACK, pthreadpool and farmhash are shared libraries with the
# file names -- and sonames -- of the xnnpack, pthreadpool and farmhash
# components, which are other versions of the same code. QNX's loader puts
# LD_LIBRARY_PATH ahead of a library's own run path (DT_RPATH or DT_RUNPATH),
# so with /usr/local/lib on it TFLite would load those instead. TFLite's are
# renamed libtflite-<name>.so, after its CMakeLists has created them (a
# deferred call, from a project include). Linking them statically instead is
# refused by TFLite's exported CMake package.
_tfl_private_names() {
  cat <<'EOF'
function(_qnx_private_names)
  foreach(t XNNPACK pthreadpool farmhash)
    if(TARGET ${t})
      set_target_properties(${t} PROPERTIES OUTPUT_NAME "tflite-${t}")
    endif()
  endforeach()
endfunction()
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _qnx_private_names)
EOF
}
# TFLite's install rules also leave its XNNPACK library out; in the common
# prefix the xnnpack component's libXNNPACK.so stood in for it ("undefined
# reference to xnn_experiment_enable_adaptive_avx_optimization"). Install its
# own, which has the install run path already (CMAKE_BUILD_WITH_INSTALL_RPATH).
do_install() {
  DESTDIR="$STAGE" cmake --install "$B"
  find "$B/_deps" -name 'libtflite-XNNPACK.so*' -exec cp -a {} "$STAGE$TFL_PREFIX/lib/" \;
  [ -e "$STAGE$TFL_PREFIX/lib/libtflite-XNNPACK.so" ]
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
