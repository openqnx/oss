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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add protobuf
apk add protobuf-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-protobuf && /usr/local/bin/qnx-port-tests/test_protobuf    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install protobuf`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa protobuf
ls dists/8.0/*/ | grep '^protobuf-'
```

This writes `protobuf-<ver>-r0.tar.gz` (run time) and `protobuf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add protobuf
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/protobuf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/protobuf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_protobuf`](../../tests/src/) links protobuf the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-protobuf`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-protobuf
/usr/local/bin/qnx-port-tests/test_protobuf
```

```
== protobuf ==
  ok   ...
PASS protobuf
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_protobuf` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_protobuf`](../../tests/src/) -- a program linked against protobuf the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - serialize Timestamp
   - wire bytes produced
   - parse Timestamp
   - field round trip
   - Struct map fields
   - map value readback

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **29.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add protobuf=<version>`), sources in `ported/protobuf/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates protobuf`; port one with `scripts/port-update protobuf`.

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
