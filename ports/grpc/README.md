# grpc 1.69.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/grpc/grpc.git (tag `v1.69.0`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`zlib`](../zlib/) [`openssl`](../openssl/) [`c-ares`](../c-ares/) [`abseil`](../abseil/) [`protobuf`](../protobuf/) [`re2`](../re2/) |
| Run-time dependencies | [`abseil`](../abseil/) [`c-ares`](../c-ares/) [`openssl`](../openssl/) [`protobuf`](../protobuf/) [`re2`](../re2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_grpc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install grpc
oss-get install grpc-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild grpc` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib openssl c-ares abseil protobuf re2
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.69.0 --recurse-submodules --shallow-submodules \
    https://github.com/grpc/grpc.git src/grpc
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/grpc < ports/grpc/patches/0001-absl-maskedpointer-default-ctor-libcxx.patch
patch -p1 -d src/grpc < ports/grpc/patches/0002-qnx-platform-socket-support.patch
```

**[`0001-absl-maskedpointer-default-ctor-libcxx.patch`](patches/0001-absl-maskedpointer-default-ctor-libcxx.patch)** — gRPC's bundled abseil: default constructor for MaskedPointer (libc++).

> The same change as ports/abseil/patches/0001, for the abseil copy gRPC
> builds: under libc++, the C++ library QNX ships, std::atomic<T> requires a
> default-constructible T.

Touches: `third_party/abseil-cpp/absl/flags/internal/flag.h` · Upstream status: Pending (affects any libc++ build)

**[`0002-qnx-platform-socket-support.patch`](patches/0002-qnx-platform-socket-support.patch)** — gRPC: QNX platform definition for the POSIX socket code.

> Without a GPR_<platform> symbol the sockaddr and iomgr headers do not treat
> QNX as POSIX and every socket type is undeclared. Define GPR_QNX and select the
> features SDP 8.0 has (getifaddrs, AF_UNIX, SO_NOSIGPIPE, IPV6_RECVPKTINFO);
> with no native epoll, the poll engine and the pipe wakeup fd are used.

Touches: `include/grpc/support/port_platform.h`, `include/grpc/event_engine/port.h`, `src/core/lib/iomgr/port.h`, `src/core/util/posix/directory_reader.cc` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/grpc; B=build/$QNX_ARCH/grpc; mkdir -p $B
```

```bash
do_configure() {
  local HB; HB=$(_HB)
  if [ ! -x "$HB/grpc_cpp_plugin" ]; then
    mkdir -p "$HB"
    ( unset CC CXX
      cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DgRPC_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF \
        -DgRPC_BUILD_CODEGEN=ON \
        -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
        -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
        -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF )
    cmake --build "$HB" -j"$(nproc)" --target protoc grpc_cpp_plugin
  fi
  local HOST_PROTOC HOST_PLUGIN
  # The protoc that matches the staged runtime -- the protobuf recipe's host
  # build (29.3), not the one bundled with gRPC (29.0).  protobuf 29 gencode
  # #errors unless PROTOBUF_VERSION matches exactly.
  HOST_PROTOC=$(find "$BUILD_DIR/host/protobuf" \( -type f -o -type l \) -name protoc | head -1)
  HOST_PLUGIN=$(find "$HB" \( -type f -o -type l \) -name grpc_cpp_plugin | head -1)
  [ -x "$HOST_PROTOC" ] && [ -x "$HOST_PLUGIN" ] || { echo "host codegen tools missing"; return 1; }

  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DBUILD_SHARED_LIBS=ON -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_INSTALL=ON -DgRPC_BUILD_CODEGEN=OFF \
    -DgRPC_ZLIB_PROVIDER=package -DZLIB_ROOT="$STAGE$PREFIX" \
    -DgRPC_SSL_PROVIDER=package -DOPENSSL_ROOT_DIR="$STAGE$PREFIX" \
    -DgRPC_PROTOBUF_PROVIDER=package -DgRPC_ABSL_PROVIDER=package \
    -DgRPC_CARES_PROVIDER=package -DgRPC_RE2_PROVIDER=package \
    -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" \
    -DProtobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf" \
    -Dc-ares_DIR="$STAGE$PREFIX/lib/cmake/c-ares" \
    -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
    -D_gRPC_PROTOBUF_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DProtobuf_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DgRPC_CPP_PLUGIN_EXECUTABLE="$HOST_PLUGIN" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
}
```

```bash
do_build()   { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- gRPC's build runs protoc and grpc_cpp_plugin to generate its own sources, so both must be *host* binaries. Build a native pass first, then point the cross configure at the tools it produced. protobuf, abseil and c-ares come from their own recipes (PROVIDER=package). With PROVIDER=module gRPC installed its bundled copies over the tree: a second protobuf (29.0 beside 29.3), a second abseil, and c-ares's libcares.pc and libcares.a overwritten by the bundled c-ares.  re2 likewise comes from its own recipe: as a module, gRPC installed its bundled re2's headers over it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH grpc`.

### 5. What it installs

357 files under `/usr/local`: 54 shared-library entries, 289 headers.

```
/usr/local/lib/libaddress_sorting.so
/usr/local/lib/libaddress_sorting.so.44
/usr/local/lib/libaddress_sorting.so.44.2.0
/usr/local/lib/libgpr.so
/usr/local/lib/libgpr.so.44
/usr/local/lib/libgpr.so.44.2.0
/usr/local/lib/libgrpc++.so
/usr/local/lib/libgrpc++.so.1.69
/usr/local/lib/libgrpc++.so.1.69.0
/usr/local/lib/libgrpc++_alts.so
/usr/local/lib/libgrpc++_alts.so.1.69
/usr/local/lib/libgrpc++_alts.so.1.69.0
/usr/local/lib/libgrpc++_error_details.so
/usr/local/lib/libgrpc++_error_details.so.1.69
/usr/local/lib/libgrpc++_error_details.so.1.69.0
/usr/local/lib/libgrpc++_unsecure.so
...
```

The complete list is `stage/<arch>/.manifest/grpc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH grpc
ls repo/8.0/*/ | grep '^grpc-'
```

This writes `grpc-<ver>-r0.tar.gz` (run time) and `grpc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install grpc
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/grpc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/grpc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_grpc`](../../tests/src/) links grpc the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_grpc root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_grpc'
```

```
== grpc ==
  ok   ...
PASS grpc
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.69.0; GIT=https://github.com/grpc/grpc.git; REF=v1.69.0
DEPS="zlib openssl c-ares abseil protobuf re2"
BUILDSYS=custom
# gRPC's build runs protoc and grpc_cpp_plugin to generate its own sources, so
# both must be *host* binaries. Build a native pass first, then point the cross
# configure at the tools it produced.
#
# protobuf, abseil and c-ares come from their own recipes (PROVIDER=package).
# With PROVIDER=module gRPC installed its bundled copies over the tree: a second
# protobuf (29.0 beside 29.3), a second abseil, and c-ares's libcares.pc and
# libcares.a overwritten by the bundled c-ares.  re2 likewise comes from its
# own recipe: as a module, gRPC installed its bundled re2's headers over it.
_HB() { echo "$BUILD_DIR/host/grpc"; }

do_configure() {
  local HB; HB=$(_HB)
  if [ ! -x "$HB/grpc_cpp_plugin" ]; then
    mkdir -p "$HB"
    ( unset CC CXX
      cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DgRPC_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=OFF \
        -DgRPC_BUILD_CODEGEN=ON \
        -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
        -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
        -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF )
    cmake --build "$HB" -j"$(nproc)" --target protoc grpc_cpp_plugin
  fi
  local HOST_PROTOC HOST_PLUGIN
  # The protoc that matches the staged runtime -- the protobuf recipe's host
  # build (29.3), not the one bundled with gRPC (29.0).  protobuf 29 gencode
  # #errors unless PROTOBUF_VERSION matches exactly.
  HOST_PROTOC=$(find "$BUILD_DIR/host/protobuf" \( -type f -o -type l \) -name protoc | head -1)
  HOST_PLUGIN=$(find "$HB" \( -type f -o -type l \) -name grpc_cpp_plugin | head -1)
  [ -x "$HOST_PROTOC" ] && [ -x "$HOST_PLUGIN" ] || { echo "host codegen tools missing"; return 1; }

  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DBUILD_SHARED_LIBS=ON -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_INSTALL=ON -DgRPC_BUILD_CODEGEN=OFF \
    -DgRPC_ZLIB_PROVIDER=package -DZLIB_ROOT="$STAGE$PREFIX" \
    -DgRPC_SSL_PROVIDER=package -DOPENSSL_ROOT_DIR="$STAGE$PREFIX" \
    -DgRPC_PROTOBUF_PROVIDER=package -DgRPC_ABSL_PROVIDER=package \
    -DgRPC_CARES_PROVIDER=package -DgRPC_RE2_PROVIDER=package \
    -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" \
    -DProtobuf_DIR="$STAGE$PREFIX/lib/cmake/protobuf" \
    -Dc-ares_DIR="$STAGE$PREFIX/lib/cmake/c-ares" \
    -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
    -D_gRPC_PROTOBUF_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DProtobuf_PROTOC_EXECUTABLE="$HOST_PROTOC" \
    -DgRPC_CPP_PLUGIN_EXECUTABLE="$HOST_PLUGIN" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
}
do_build()   { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
