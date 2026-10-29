# protobuf 29.3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/protocolbuffers/protobuf.git (tag `v29.3`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`zlib`](../zlib/) [`abseil`](../abseil/) |
| Run-time dependencies | [`abseil`](../abseil/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_protobuf`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install protobuf
oss-get install protobuf-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild protobuf` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib abseil
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v29.3 --recurse-submodules --shallow-submodules \
    https://github.com/protocolbuffers/protobuf.git src/protobuf
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/protobuf; B=build/$QNX_ARCH/protobuf; mkdir -p $B
```

```bash
do_configure() {
  local HB="$BUILD_DIR/host/protobuf"
  if [ ! -x "$HB/protoc" ]; then
    mkdir -p "$HB"
    cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_EXAMPLES=OFF \
      -Dprotobuf_ABSL_PROVIDER=module -DBUILD_SHARED_LIBS=OFF
    cmake --build "$HB" -j"$(nproc)" --target protoc
  fi
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -Dprotobuf_PROTOC_EXE="$(find "$HB" -name protoc -type f | head -1)" \
    "${_common[@]}"
}
```

```bash
do_build()   { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- protoc must run on the *host* during the build, so a native protoc is built first and handed to the cross build via protobuf_PROTOC_EXE.
- ABSL_PROVIDER=package: link the staged abseil rather than installing the submodule's copy (20240116) next to it -- see ports/abseil.  The host protoc below keeps the module: it is a static build machine tool and installs nothing.  gRPC's cross build uses that same host protoc, because protobuf 29 gencode compiles only against the exact runtime version ("#if PROTOBUF_VERSION != ..." in compiler/cpp/file.cc).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH protobuf`.

### 5. What it installs

410 files under `/usr/local`: 8 shared-library entries, 8 programs, 379 headers.

```
/usr/local/lib/libprotobuf-lite.so
/usr/local/lib/libprotobuf-lite.so.29.3.0
/usr/local/lib/libprotobuf.so
/usr/local/lib/libprotobuf.so.29.3.0
/usr/local/lib/libprotoc.so
/usr/local/lib/libprotoc.so.29.3.0
/usr/local/lib/libutf8_range.so
/usr/local/lib/libutf8_validity.so
/usr/local/bin/protoc
/usr/local/bin/protoc-29.3.0
/usr/local/bin/protoc-gen-upb
/usr/local/bin/protoc-gen-upb-29.3.0
/usr/local/bin/protoc-gen-upb_minitable
/usr/local/bin/protoc-gen-upb_minitable-29.3.0
/usr/local/bin/protoc-gen-upbdefs
/usr/local/bin/protoc-gen-upbdefs-29.3.0
```

The complete list is `stage/<arch>/.manifest/protobuf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH protobuf
ls repo/8.0/*/ | grep '^protobuf-'
```

This writes `protobuf-<ver>-r0.tar.gz` (run time) and `protobuf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install protobuf
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/protobuf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/protobuf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_protobuf`](../../tests/src/) links protobuf the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_protobuf root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_protobuf'
```

```
== protobuf ==
  ok   ...
PASS protobuf
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=29.3; GIT=https://github.com/protocolbuffers/protobuf.git; REF=v29.3
DEPS="zlib abseil"
# protoc must run on the *host* during the build, so a native protoc is built
# first and handed to the cross build via protobuf_PROTOC_EXE.
BUILDSYS=custom
# ABSL_PROVIDER=package: link the staged abseil rather than installing the
# submodule's copy (20240116) next to it -- see ports/abseil.  The host protoc
# below keeps the module: it is a static build machine tool and installs
# nothing.  gRPC's cross build uses that same host protoc, because protobuf 29
# gencode compiles only against the exact runtime version
# ("#if PROTOBUF_VERSION != ..." in compiler/cpp/file.cc).
_common=( -DBUILD_SHARED_LIBS=ON -Dprotobuf_BUILD_TESTS=OFF
          -Dprotobuf_BUILD_EXAMPLES=OFF -Dprotobuf_ABSL_PROVIDER=package
          -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl"
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
do_configure() {
  local HB="$BUILD_DIR/host/protobuf"
  if [ ! -x "$HB/protoc" ]; then
    mkdir -p "$HB"
    cmake -S "$S" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_EXAMPLES=OFF \
      -Dprotobuf_ABSL_PROVIDER=module -DBUILD_SHARED_LIBS=OFF
    cmake --build "$HB" -j"$(nproc)" --target protoc
  fi
  cmake -S "$S" -B "$B" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -Dprotobuf_PROTOC_EXE="$(find "$HB" -name protoc -type f | head -1)" \
    "${_common[@]}"
}
do_build()   { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
