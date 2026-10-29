# onnxruntime 1.20.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/microsoft/onnxruntime.git (tag `v1.20.1`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`protobuf`](../protobuf/) |
| Run-time dependencies | [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_onnxruntime`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install onnxruntime
oss-get install onnxruntime-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild onnxruntime` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild protobuf
```

<details><summary>Full build chain (3)</summary>

[`abseil`](../abseil/) [`protobuf`](../protobuf/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.20.1 --recurse-submodules --shallow-submodules \
    https://github.com/microsoft/onnxruntime.git src/onnxruntime
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/onnxruntime < ports/onnxruntime/patches/0001-qnx-platform-fixes.patch
```

**[`0001-qnx-platform-fixes.patch`](patches/0001-qnx-platform-fixes.patch)** — ONNX Runtime: QNX platform fixes.

> - path_lib.h: QNX's dirent has d_type but no DT_* values; classify entries with
>   stat(), as the AIX path does.
> - logging.cc, env.cc: no <sys/syscall.h> or SYS_gettid on QNX.
> - threadpool.cc: cpuinfo has no linux_id on QNX; use the logical CPU index,
>   which is also the ThreadCtl() run-mask bit. There is no "current CPU" call;
>   report it as unknown (it only labels profiler output).
> - stacktrace.cc: no <execinfo.h> backtrace() on QNX.
> - cmake: qcc does not accept -Xlinker; use -Wl, for the version script.

Touches: `onnxruntime/core/platform/path_lib.h`, `onnxruntime/core/common/logging/logging.cc`, `onnxruntime/core/platform/posix/env.cc`, `onnxruntime/core/common/threadpool.cc`, `onnxruntime/core/platform/posix/stacktrace.cc`, `cmake/onnxruntime.cmake` … · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/onnxruntime; B=build/$QNX_ARCH/onnxruntime; mkdir -p $B
```

```bash
do_configure() {
  local PBSRC="$SRC_DIR/protobuf-host-$ORT_PROTOBUF_REF"
  local PBB="$BUILD_DIR/host/protobuf-$ORT_PROTOBUF_REF"
  [ -d "$PBSRC" ] || git clone -q --depth 1 -b "$ORT_PROTOBUF_REF" \
      https://github.com/protocolbuffers/protobuf.git "$PBSRC"
  local PBROOT="$PBSRC"; [ -f "$PBSRC/CMakeLists.txt" ] || PBROOT="$PBSRC/cmake"
  if [ ! -e "$PBB/protoc" ]; then
    ( unset CC CXX
      cmake -S "$PBROOT" -B "$PBB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF \
        -Dprotobuf_WITH_ZLIB=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 )
    cmake --build "$PBB" -j"$(nproc)" --target protoc
  fi
  # ORT vendors abseil 20240722.0, which needs the same libc++ fix gRPC did:
  # MaskedPointer has no default constructor, and libc++'s std::atomic<T>
  # requires one. FetchContent pulls abseil during configure, too late for the
  # driver's patch step -- so stage a pre-patched checkout and point ORT at it.
  local ABSL="$SRC_DIR/abseil-ort-20240722.0"
  if [ ! -d "$ABSL" ]; then
    git clone -q --depth 1 -b 20240722.0 https://github.com/abseil/abseil-cpp.git "$ABSL"
    patch -p1 -d "$ABSL" < "$RECIPE_DIR/onnxruntime/files/absl-maskedpointer.patch"
  fi

  # ORT pins Eigen to a post-3.4.0 master commit that has the
  # min<PropagateNaN>() overloads its CPU kernels call. Left to find_package, it
  # picks up the workspace's Eigen 3.4.0 from staging instead and those overloads
  # do not exist. Pin it explicitly to the commit cmake/deps.txt names.
  local EIGEN_SHA=e7248b26a1ed53fa030c5c459f7ea095dfd276ac
  local EIGENDIR="$SRC_DIR/eigen-ort-$EIGEN_SHA"
  if [ ! -d "$EIGENDIR" ]; then
    mkdir -p "$EIGENDIR"
    local tb="$DIST_DIR/eigen-$EIGEN_SHA.tar.gz"
    [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" \
      "https://gitlab.com/libeigen/eigen/-/archive/$EIGEN_SHA/eigen-$EIGEN_SHA.tar.gz"
    tar xf "$tb" -C "$EIGENDIR" --strip-components=1
  fi

  local HOST_PROTOC
  HOST_PROTOC=$(find "$PBB" \( -type f -o -type l \) -name "protoc*" -perm -u+x 2>/dev/null | head -1)
  [ -x "$HOST_PROTOC" ] || { echo "host protoc $ORT_PROTOBUF_REF build failed"; return 1; }
  echo "using host protoc: $HOST_PROTOC ($("$HOST_PROTOC" --version 2>&1))"
  # -Wno-error=maybe-uninitialized: GCC raises a known false positive inside
  # Abseil's SwissTable (raw_hash_set.h, ABSL_SWISSTABLE_IGNORE_UNINITIALIZED_
  # RETURN); ORT builds with -Werror, so the warning has to be demoted.
  # onnxruntime_ENABLE_CPUINFO=OFF: ORT marks cpuinfo supported for any aarch64
  # or x86_64 target without checking the OS, but pytorch_cpuinfo has no QNX
  # backend -- it builds with CPUINFO_SUPPORTED_PLATFORM=0, i.e. an empty
  # library, and the link fails on cpuinfo_initialize / cpuinfo_get_cores_count.
  # Turning it off puts ORT on its hardware_concurrency()/sysconf fallback.
  # CMAKE_DISABLE_FIND_PACKAGE_Eigen3: onnxruntime_external_deps.cmake calls
  # find_package(Eigen3 CONFIG) and, when it succeeds, takes that target's
  # include dirs -- which is how the staging Eigen 3.4.0 kept winning over the
  # commit ORT pins. Disabling the search sends ORT down its own eigen.cmake.
  cmake -S "$S/cmake" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -Donnxruntime_BUILD_SHARED_LIB=ON \
    -Donnxruntime_BUILD_UNIT_TESTS=OFF \
    -Donnxruntime_BUILD_BENCHMARKS=OFF \
    -Donnxruntime_ENABLE_PYTHON=OFF \
    -Donnxruntime_USE_FULL_PROTOBUF=ON \
    -DONNX_CUSTOM_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DProtobuf_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -Donnxruntime_DISABLE_ABSEIL=OFF \
    -Donnxruntime_USE_NNAPI_BUILTIN=OFF -Donnxruntime_USE_CUDA=OFF \
    -Donnxruntime_USE_OPENVINO=OFF -Donnxruntime_USE_DNNL=OFF \
    -Donnxruntime_USE_XNNPACK=OFF \
    -Donnxruntime_ENABLE_CPUINFO=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_C_FLAGS="" \
    -DCMAKE_CXX_FLAGS="-std=gnu++17 -Wno-error=maybe-uninitialized" \
    -DFETCHCONTENT_TRY_FIND_PACKAGE_MODE=NEVER \
    -DFETCHCONTENT_SOURCE_DIR_ABSEIL_CPP="$ABSL" \
    -DCMAKE_DISABLE_FIND_PACKAGE_Eigen3=ON \
    -Donnxruntime_USE_PREINSTALLED_EIGEN=ON \
    -Deigen_SOURCE_PATH="$EIGENDIR"
    # ^ FetchContent otherwise satisfies protobuf via find_package, which hits
    # the workspace's protobuf 29.3 in staging -- while ORT still generates its
    # ONNX sources with the protoc it pins at 21.12. The two disagree and the
    # generated headers compile against the wrong runtime. NEVER forces ORT to
    # vendor every dependency at the versions in cmake/deps.txt, the combination
    # upstream actually tests.
    # ^ deliberately drops the toolchain's global -I<stage>/include for this
    # package. ORT vendors its own protobuf 21.12 via FetchContent, but the
    # workspace staging area holds protobuf 29.3 headers, and CMAKE_CXX_FLAGS is
    # placed ahead of target include directories on the command line -- so the
    # 29.3 headers shadow ORT's, leaving its 21.12 gencode compiling against the
    # wrong runtime (incomplete onnx::TensorProto and friends).
}
```

```bash
do_build()   { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ORT's CMake lives in cmake/, not the repo root, and it runs protoc during the build to generate the ONNX schema sources -- so a host protoc is required. ORT pins protobuf v21.12 (cmake/deps.txt) and generates its ONNX sources with it. Handing it the workspace's protoc (29.3) produces gencode the fetched 21.12 runtime rejects: "Protobuf C++ gencode is built with an incompatible version of Protobuf C++ headers/runtime". So build a *host* protoc at exactly 21.12.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH onnxruntime`.

### 5. What it installs

20 files under `/usr/local`: 4 shared-library entries, 11 headers.

```
/usr/local/lib/libonnxruntime.so
/usr/local/lib/libonnxruntime.so.1
/usr/local/lib/libonnxruntime.so.1.20.1
/usr/local/lib/libonnxruntime_providers_shared.so
```

The complete list is `stage/<arch>/.manifest/onnxruntime` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH onnxruntime
ls repo/8.0/*/ | grep '^onnxruntime-'
```

This writes `onnxruntime-<ver>-r0.tar.gz` (run time) and `onnxruntime-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install onnxruntime
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/onnxruntime-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/onnxruntime-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_onnxruntime`](../../tests/src/) links onnxruntime the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_onnxruntime root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_onnxruntime'
```

```
== onnxruntime ==
  ok   ...
PASS onnxruntime
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.20.1; GIT=https://github.com/microsoft/onnxruntime.git; REF=v1.20.1
DEPS="protobuf"
BUILDSYS=custom
# ORT's CMake lives in cmake/, not the repo root, and it runs protoc during the
# build to generate the ONNX schema sources -- so a host protoc is required.
# ORT pins protobuf v21.12 (cmake/deps.txt) and generates its ONNX sources with
# it. Handing it the workspace's protoc (29.3) produces gencode the fetched 21.12
# runtime rejects: "Protobuf C++ gencode is built with an incompatible version of
# Protobuf C++ headers/runtime". So build a *host* protoc at exactly 21.12.
ORT_PROTOBUF_REF=v21.12

do_configure() {
  local PBSRC="$SRC_DIR/protobuf-host-$ORT_PROTOBUF_REF"
  local PBB="$BUILD_DIR/host/protobuf-$ORT_PROTOBUF_REF"
  [ -d "$PBSRC" ] || git clone -q --depth 1 -b "$ORT_PROTOBUF_REF" \
      https://github.com/protocolbuffers/protobuf.git "$PBSRC"
  local PBROOT="$PBSRC"; [ -f "$PBSRC/CMakeLists.txt" ] || PBROOT="$PBSRC/cmake"
  if [ ! -e "$PBB/protoc" ]; then
    ( unset CC CXX
      cmake -S "$PBROOT" -B "$PBB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF \
        -Dprotobuf_WITH_ZLIB=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 )
    cmake --build "$PBB" -j"$(nproc)" --target protoc
  fi
  # ORT vendors abseil 20240722.0, which needs the same libc++ fix gRPC did:
  # MaskedPointer has no default constructor, and libc++'s std::atomic<T>
  # requires one. FetchContent pulls abseil during configure, too late for the
  # driver's patch step -- so stage a pre-patched checkout and point ORT at it.
  local ABSL="$SRC_DIR/abseil-ort-20240722.0"
  if [ ! -d "$ABSL" ]; then
    git clone -q --depth 1 -b 20240722.0 https://github.com/abseil/abseil-cpp.git "$ABSL"
    patch -p1 -d "$ABSL" < "$RECIPE_DIR/onnxruntime/files/absl-maskedpointer.patch"
  fi

  # ORT pins Eigen to a post-3.4.0 master commit that has the
  # min<PropagateNaN>() overloads its CPU kernels call. Left to find_package, it
  # picks up the workspace's Eigen 3.4.0 from staging instead and those overloads
  # do not exist. Pin it explicitly to the commit cmake/deps.txt names.
  local EIGEN_SHA=e7248b26a1ed53fa030c5c459f7ea095dfd276ac
  local EIGENDIR="$SRC_DIR/eigen-ort-$EIGEN_SHA"
  if [ ! -d "$EIGENDIR" ]; then
    mkdir -p "$EIGENDIR"
    local tb="$DIST_DIR/eigen-$EIGEN_SHA.tar.gz"
    [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" \
      "https://gitlab.com/libeigen/eigen/-/archive/$EIGEN_SHA/eigen-$EIGEN_SHA.tar.gz"
    tar xf "$tb" -C "$EIGENDIR" --strip-components=1
  fi

  local HOST_PROTOC
  HOST_PROTOC=$(find "$PBB" \( -type f -o -type l \) -name "protoc*" -perm -u+x 2>/dev/null | head -1)
  [ -x "$HOST_PROTOC" ] || { echo "host protoc $ORT_PROTOBUF_REF build failed"; return 1; }
  echo "using host protoc: $HOST_PROTOC ($("$HOST_PROTOC" --version 2>&1))"
  # -Wno-error=maybe-uninitialized: GCC raises a known false positive inside
  # Abseil's SwissTable (raw_hash_set.h, ABSL_SWISSTABLE_IGNORE_UNINITIALIZED_
  # RETURN); ORT builds with -Werror, so the warning has to be demoted.
  # onnxruntime_ENABLE_CPUINFO=OFF: ORT marks cpuinfo supported for any aarch64
  # or x86_64 target without checking the OS, but pytorch_cpuinfo has no QNX
  # backend -- it builds with CPUINFO_SUPPORTED_PLATFORM=0, i.e. an empty
  # library, and the link fails on cpuinfo_initialize / cpuinfo_get_cores_count.
  # Turning it off puts ORT on its hardware_concurrency()/sysconf fallback.
  # CMAKE_DISABLE_FIND_PACKAGE_Eigen3: onnxruntime_external_deps.cmake calls
  # find_package(Eigen3 CONFIG) and, when it succeeds, takes that target's
  # include dirs -- which is how the staging Eigen 3.4.0 kept winning over the
  # commit ORT pins. Disabling the search sends ORT down its own eigen.cmake.
  cmake -S "$S/cmake" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -Donnxruntime_BUILD_SHARED_LIB=ON \
    -Donnxruntime_BUILD_UNIT_TESTS=OFF \
    -Donnxruntime_BUILD_BENCHMARKS=OFF \
    -Donnxruntime_ENABLE_PYTHON=OFF \
    -Donnxruntime_USE_FULL_PROTOBUF=ON \
    -DONNX_CUSTOM_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DProtobuf_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -Donnxruntime_DISABLE_ABSEIL=OFF \
    -Donnxruntime_USE_NNAPI_BUILTIN=OFF -Donnxruntime_USE_CUDA=OFF \
    -Donnxruntime_USE_OPENVINO=OFF -Donnxruntime_USE_DNNL=OFF \
    -Donnxruntime_USE_XNNPACK=OFF \
    -Donnxruntime_ENABLE_CPUINFO=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_C_FLAGS="" \
    -DCMAKE_CXX_FLAGS="-std=gnu++17 -Wno-error=maybe-uninitialized" \
    -DFETCHCONTENT_TRY_FIND_PACKAGE_MODE=NEVER \
    -DFETCHCONTENT_SOURCE_DIR_ABSEIL_CPP="$ABSL" \
    -DCMAKE_DISABLE_FIND_PACKAGE_Eigen3=ON \
    -Donnxruntime_USE_PREINSTALLED_EIGEN=ON \
    -Deigen_SOURCE_PATH="$EIGENDIR"
    # ^ FetchContent otherwise satisfies protobuf via find_package, which hits
    # the workspace's protobuf 29.3 in staging -- while ORT still generates its
    # ONNX sources with the protoc it pins at 21.12. The two disagree and the
    # generated headers compile against the wrong runtime. NEVER forces ORT to
    # vendor every dependency at the versions in cmake/deps.txt, the combination
    # upstream actually tests.
    # ^ deliberately drops the toolchain's global -I<stage>/include for this
    # package. ORT vendors its own protobuf 21.12 via FetchContent, but the
    # workspace staging area holds protobuf 29.3 headers, and CMAKE_CXX_FLAGS is
    # placed ahead of target include directories on the command line -- so the
    # 29.3 headers shadow ORT's, leaving its 21.12 gencode compiling against the
    # wrong runtime (incomplete onnx::TensorProto and friends).
}
do_build()   { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
