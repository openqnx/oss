# grpc 1.69.0

> Remote procedure call (RPC) framework from Google

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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `grpc` (programs, shared libraries, run-time data) and `grpc-dev` (headers, pkg-config/CMake files, static libraries; needs `grpc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`grpc`](https://openqnx.github.io/oss/package/x86_64/grpc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/grpc.html)) · [`grpc-dev`](https://openqnx.github.io/oss/package/x86_64/grpc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/grpc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add grpc` (dependencies come along); one version: `apk add grpc=<version>` | `apk del grpc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install grpc` (dependencies first) | `oss-get remove grpc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`grpc-dev` the same way, by its own name (`apk add grpc-dev`, `oss-get install grpc-dev`, by hand with its own file and list `grpc-dev.list`); remove it before `grpc`, which it needs.

```sh
# on the QNX target, as root
apk add grpc grpc-dev
apk add qnx-test-grpc && /usr/local/bin/qnx-port-tests/test_grpc    # its on-target test
apk del grpc grpc-dev qnx-test-grpc
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/grpc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/grpc.list
rm var/lib/manual/grpc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa grpc
ls dists/8.0/*/ | grep '^grpc-'
```

This writes `grpc-<ver>-r0.tar.gz` (run time) and `grpc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add grpc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/grpc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/grpc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_grpc`](../../tests/src/) links grpc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-grpc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-grpc
/usr/local/bin/qnx-port-tests/test_grpc
```

```
== grpc ==
  ok   ...
PASS grpc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_grpc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_grpc`](../../tests/src/) -- a program linked against grpc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - InsecureChannelCredentials
   - CreateChannel
   - channel starts IDLE
   - CreateCustomChannel with args
   - SslCredentials (TLS stack linked)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | grpc 1.69.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/grpc-1.69.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | grpc 1.69.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/grpc-1.69.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.69.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add grpc=<version>`), sources in `ported/grpc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates grpc`; port one with `scripts/port-update grpc`.

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
