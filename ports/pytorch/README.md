# pytorch 2.3.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/pytorch/pytorch.git (tag `v2.3.1`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`python3`](../python3/) [`protobuf`](../protobuf/) [`openblas`](../openblas/) |
| Run-time dependencies | [`abseil`](../abseil/) [`openblas`](../openblas/) [`protobuf`](../protobuf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_torch`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pytorch
apk add pytorch-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-torch && /usr/local/bin/qnx-port-tests/test_torch    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pytorch`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pytorch` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (18 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 protobuf openblas
```

<details><summary>Full build chain (18)</summary>

[`abseil`](../abseil/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openblas`](../openblas/) [`openssl`](../openssl/) [`protobuf`](../protobuf/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.3.1 --recurse-submodules --shallow-submodules \
    https://github.com/pytorch/pytorch.git src/pytorch
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pytorch < ports/pytorch/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Readded obvious QNX SDP 7.1 commits.

> ---
>  CMakeLists.txt                                |   2 +-
>  .../quantized/cpu/qnnpack/CMakeLists.txt      |   2 +-
>  .../quantized/cpu/qnnpack/src/qnnpack/math.h  |   2 +-
>  c10/util/BFloat16-inl.h                       |   7 +
>  c10/util/Half-inl.h                           |   7 +
>  cmake/Dependencies.cmake                      |   2 +-
>  cmake/External/nnpack.cmake                   |   2 +-
>  scripts/build_mobile_qnx_arm.sh               | 132 ++++++++++++++++++
>  tools/setup_helpers/env.py                    |   1 +
>  torch/csrc/utils/byte_order.h                 |   4 +
>  10 files changed, 156 insertions(+), 5 deletions(-)
>  create mode 100755 scripts/build_mobile_qnx_arm.sh

Touches: `CMakeLists.txt`, `aten/src/ATen/native/quantized/cpu/qnnpack/CMakeLists.txt`, `aten/src/ATen/native/quantized/cpu/qnnpack/src/qnnpack/math.h`, `c10/util/BFloat16-inl.h`, `c10/util/Half-inl.h`, `cmake/Dependencies.cmake` … · Upstream status: QNX-specific (from qnx-ports)

More QNX changes, outside the main source tree:

- [`deps/`](deps/) — 10 patches for dependencies the build fetches itself (applied by the recipe)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pytorch; B=build/$QNX_ARCH/pytorch; mkdir -p $B
```

```bash
do_patch() {
  local d p
  for d in "$RECIPE_DIR/pytorch/deps"/*/; do
    d=$(basename "$d")
    for p in "$RECIPE_DIR/pytorch/deps/$d"/*.patch; do
      echo "-- patch third_party/$d: $(basename "$p")"
      git -C "$SRCROOT/third_party/$d" apply --whitespace=nowarn "$p" || return 1
    done
  done
}
```

```bash
do_configure() {
  local H="$BUILD_DIR/host/pytorch-$VERSION"
  if [ ! -x "$H/sleef/bin/mkrename" ]; then
    env -u CC -u CXX cmake -S "$S/third_party/sleef" -B "$H/sleef" -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTS=OFF -DBUILD_DFT=OFF -DBUILD_QUAD=OFF -DBUILD_GNUABI_LIBS=OFF && ninja -C "$H/sleef" || return 1
  fi
  [ -x "$BUILD_DIR/host/protobuf/protoc" ] || { echo "no host protoc: build the protobuf component first"; return 1; }
  rm -rf "$B/xnnpack-include" && cp -a "$S/third_party/XNNPACK/include" "$B/xnnpack-include" || return 1
  CFLAGS="-I$B/xnnpack-include" CXXFLAGS="-I$B/xnnpack-include" \
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PT_PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=ON \
    -DBUILD_PYTHON=OFF -DBUILD_CAFFE2=OFF -DBUILD_TEST=OFF -DBUILD_BINARY=OFF -DBUILD_LITE_INTERPRETER=OFF \
    -DUSE_ROCM=OFF -DUSE_CUDA=OFF -DUSE_ITT=OFF -DUSE_GFLAGS=OFF -DUSE_GLOG=OFF -DUSE_OPENCV=OFF \
    -DUSE_LMDB=OFF -DUSE_LEVELDB=OFF -DUSE_MPI=OFF -DUSE_OPENMP=OFF -DUSE_MKLDNN=OFF -DUSE_NNPACK=OFF \
    -DUSE_NUMPY=OFF -DUSE_BLAS=OFF -DUSE_DISTRIBUTED=OFF -DUSE_KINETO=OFF -DUSE_FBGEMM=OFF \
    -DNATIVE_BUILD_DIR="$H/sleef" \
    -DBUILD_CUSTOM_PROTOBUF=OFF -DCAFFE2_CUSTOM_PROTOC_EXECUTABLE="$BUILD_DIR/host/protobuf/protoc" \
    -DProtobuf_PROTOC_EXECUTABLE="$BUILD_DIR/host/protobuf/protoc" \
    -DPYTHON_EXECUTABLE=/usr/bin/python3 -DPython_EXECUTABLE=/usr/bin/python3 $(_pt_tryrun) \
    -DCMAKE_DISABLE_FIND_PACKAGE_Backtrace=ON
}
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Fetched like any git component, except the Eigen submodule: gitlab.com kept refusing git clones ("GitLab is currently unable to handle this request"), so Eigen is the archive of the commit the submodule pins, checksum-verified.
- PyTorch 2.3.1 as a C++ library (libtorch), the release qnx-ports ports to QNX 8.0. No Python bindings, no CUDA/ROCm, CPU kernels only. patches/: PyTorch's own QNX changes (qnx-ports/pytorch qnx_v2.3.1). deps/<submodule>/*.patch: QNX changes to the third_party submodules, applied inside each submodule (qnx-ports/build-files ports/pytorch/patches). Two generators must run on the build host: SLEEF's helper programs, built natively from PyTorch's own third_party copy, and protoc, the protobuf component's build-host protoc. libtorch_cpu also links the openblas component, which PyTorch's CMake finds on its own.
- Like TFLite, PyTorch installs the third_party libraries it builds (cpuinfo, fmt, sleef, XNNPACK, pthreadpool, fp16, FXdiv headers) next to itself, over the components of those names; it gets its own prefix, /usr/local/opt/pytorch (its libraries already carry $ORIGIN as run path).
- Protobuf: the protobuf component (29.3) with its build-host protoc, not the bundled third_party/protobuf -- generating the ONNX sources with one protoc and compiling them against the other's headers failed ("generated by a newer version of protoc"). XNNPACK: a copy of the bundled third_party/XNNPACK headers goes in CFLAGS/CXXFLAGS as -I, which is searched ahead of the toolchain's -isystem staging directory; otherwise the xnnpack component's newer staged xnnpack.h won and its API lacks what PyTorch calls ("'xnn_create_add_nd_qs8' was not declared"). A copy, because PyTorch's targets also name third_party/XNNPACK/ include with -isystem, and GCC ignores a -I for a directory it already has as a system one. deps/sleef: SLEEF only knows GNU/Clang flags and CMake calls qcc "QCC". Backtrace: CMake's FindBacktrace takes QNX's unrelated <backtrace.h> (bt_* API) for glibc's and c10 then had no include directory for it; without it c10 builds with SUPPORTS_BACKTRACE=0. CPU-feature checks (cmake/Modules/FindAVX.cmake) are try_run()s, which a cross build cannot run; give their answer instead: no AVX/AVX2/AVX-512 assumed at build time, so the generic code paths are compiled in.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pytorch`.

### 5. What it installs

9364 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/pytorch` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pytorch
ls dists/8.0/*/ | grep '^pytorch-'
```

This writes `pytorch-<ver>-r0.tar.gz` (run time) and `pytorch-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pytorch
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pytorch-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pytorch-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_torch`](../../tests/src/) links pytorch the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-torch`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-torch
/usr/local/bin/qnx-port-tests/test_torch
```

```
== pytorch ==
  ok   ...
PASS pytorch
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_torch` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_torch`](../../tests/src/) -- a program linked against pytorch the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - matmul shape 2x2
   - matmul values
   - sum reduction
   - autograd gradient 2x

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64 virt, vanilla mkqnximage image | apk:local-dists | pytorch 2.3.1-r1 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pytorch 2.3.1-r1 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pytorch 2.3.1-r1 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pytorch=<version>`), sources in `ported/pytorch/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pytorch`; port one with `scripts/port-update pytorch`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.1; GIT=https://github.com/pytorch/pytorch.git; REF=v2.3.1
# Fetched like any git component, except the Eigen submodule: gitlab.com kept
# refusing git clones ("GitLab is currently unable to handle this request"),
# so Eigen is the archive of the commit the submodule pins, checksum-verified.
PT_EIGEN_COMMIT=3147391d946bb4b6c68edd901f2add6ac1f31f8c
PT_EIGEN_SHA256=0c8c490764f9c2a793133491adca0cd073b73e0bde965c68cbe58d91b5ed4261
do_fetch() {
  git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT" || return 1
  [ "$(git -C "$SRCROOT" ls-tree HEAD third_party/eigen | awk '{print $3}')" = "$PT_EIGEN_COMMIT" ] || \
    { echo "third_party/eigen is not at $PT_EIGEN_COMMIT"; return 1; }
  git -C "$SRCROOT" -c submodule.third_party/eigen.update=none submodule update -q --init --recursive --depth 1 || return 1
  local t="$WS/dist/eigen-$PT_EIGEN_COMMIT.tar.gz"
  [ -f "$t" ] || curl -fsSL -o "$t" "https://gitlab.com/libeigen/eigen/-/archive/$PT_EIGEN_COMMIT/eigen-$PT_EIGEN_COMMIT.tar.gz" || return 1
  echo "$PT_EIGEN_SHA256  $t" | sha256sum -c --status || { echo "checksum mismatch: $t"; rm -f "$t"; return 1; }
  mkdir -p "$SRCROOT/third_party/eigen" && tar xzf "$t" -C "$SRCROOT/third_party/eigen" --strip-components=1
}
# PyTorch 2.3.1 as a C++ library (libtorch), the release qnx-ports ports to
# QNX 8.0. No Python bindings, no CUDA/ROCm, CPU kernels only.
#
# patches/: PyTorch's own QNX changes (qnx-ports/pytorch qnx_v2.3.1).
# deps/<submodule>/*.patch: QNX changes to the third_party submodules, applied
# inside each submodule (qnx-ports/build-files ports/pytorch/patches).
#
# Two generators must run on the build host: SLEEF's helper programs, built
# natively from PyTorch's own third_party copy, and protoc, the protobuf
# component's build-host protoc. libtorch_cpu also links the openblas
# component, which PyTorch's CMake finds on its own.
DEPS="python3 protobuf openblas"
BUILDSYS=custom
do_patch() {
  local d p
  for d in "$RECIPE_DIR/pytorch/deps"/*/; do
    d=$(basename "$d")
    for p in "$RECIPE_DIR/pytorch/deps/$d"/*.patch; do
      echo "-- patch third_party/$d: $(basename "$p")"
      git -C "$SRCROOT/third_party/$d" apply --whitespace=nowarn "$p" || return 1
    done
  done
}
# Like TFLite, PyTorch installs the third_party libraries it builds (cpuinfo,
# fmt, sleef, XNNPACK, pthreadpool, fp16, FXdiv headers) next to itself, over
# the components of those names; it gets its own prefix,
# /usr/local/opt/pytorch (its libraries already carry $ORIGIN as run path).
PT_PREFIX="$PREFIX/opt/pytorch"
do_configure() {
  local H="$BUILD_DIR/host/pytorch-$VERSION"
  if [ ! -x "$H/sleef/bin/mkrename" ]; then
    env -u CC -u CXX cmake -S "$S/third_party/sleef" -B "$H/sleef" -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTS=OFF -DBUILD_DFT=OFF -DBUILD_QUAD=OFF -DBUILD_GNUABI_LIBS=OFF && ninja -C "$H/sleef" || return 1
  fi
  [ -x "$BUILD_DIR/host/protobuf/protoc" ] || { echo "no host protoc: build the protobuf component first"; return 1; }
  rm -rf "$B/xnnpack-include" && cp -a "$S/third_party/XNNPACK/include" "$B/xnnpack-include" || return 1
  CFLAGS="-I$B/xnnpack-include" CXXFLAGS="-I$B/xnnpack-include" \
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PT_PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=ON \
    -DBUILD_PYTHON=OFF -DBUILD_CAFFE2=OFF -DBUILD_TEST=OFF -DBUILD_BINARY=OFF -DBUILD_LITE_INTERPRETER=OFF \
    -DUSE_ROCM=OFF -DUSE_CUDA=OFF -DUSE_ITT=OFF -DUSE_GFLAGS=OFF -DUSE_GLOG=OFF -DUSE_OPENCV=OFF \
    -DUSE_LMDB=OFF -DUSE_LEVELDB=OFF -DUSE_MPI=OFF -DUSE_OPENMP=OFF -DUSE_MKLDNN=OFF -DUSE_NNPACK=OFF \
    -DUSE_NUMPY=OFF -DUSE_BLAS=OFF -DUSE_DISTRIBUTED=OFF -DUSE_KINETO=OFF -DUSE_FBGEMM=OFF \
    -DNATIVE_BUILD_DIR="$H/sleef" \
    -DBUILD_CUSTOM_PROTOBUF=OFF -DCAFFE2_CUSTOM_PROTOC_EXECUTABLE="$BUILD_DIR/host/protobuf/protoc" \
    -DProtobuf_PROTOC_EXECUTABLE="$BUILD_DIR/host/protobuf/protoc" \
    -DPYTHON_EXECUTABLE=/usr/bin/python3 -DPython_EXECUTABLE=/usr/bin/python3 $(_pt_tryrun) \
    -DCMAKE_DISABLE_FIND_PACKAGE_Backtrace=ON
}
# Protobuf: the protobuf component (29.3) with its build-host protoc, not the
# bundled third_party/protobuf -- generating the ONNX sources with one protoc
# and compiling them against the other's headers failed ("generated by a
# newer version of protoc").
# XNNPACK: a copy of the bundled third_party/XNNPACK headers goes in
# CFLAGS/CXXFLAGS as -I, which is searched ahead of the toolchain's -isystem
# staging directory; otherwise the xnnpack component's newer staged xnnpack.h
# won and its API lacks what PyTorch calls ("'xnn_create_add_nd_qs8' was not
# declared"). A copy, because PyTorch's targets also name third_party/XNNPACK/
# include with -isystem, and GCC ignores a -I for a directory it already has
# as a system one.
# deps/sleef: SLEEF only knows GNU/Clang flags and CMake calls qcc "QCC".
# Backtrace: CMake's FindBacktrace takes QNX's unrelated <backtrace.h> (bt_*
# API) for glibc's and c10 then had no include directory for it; without it
# c10 builds with SUPPORTS_BACKTRACE=0.
# CPU-feature checks (cmake/Modules/FindAVX.cmake) are try_run()s, which a cross
# build cannot run; give their answer instead: no AVX/AVX2/AVX-512 assumed at
# build time, so the generic code paths are compiled in.
_pt_tryrun() {
  local l v n
  for l in C CXX; do for v in AVX AVX2 AVX512; do for n in 1 2 3; do
    printf -- '-D%s_HAS_%s_%s_EXITCODE=1 -D%s_HAS_%s_%s_EXITCODE__TRYRUN_OUTPUT= ' $l $v $n $l $v $n
  done; done; done
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
