# onnx 1.17.0

> Open Neural Network Exchange (ONNX) format library for AI models

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/onnx/onnx.git (tag `v1.17.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`protobuf`](../protobuf/) |
| Run-time dependencies | [`abseil`](../abseil/) [`protobuf`](../protobuf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_onnx`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `onnx` (programs, shared libraries, run-time data) and `onnx-dev` (headers, pkg-config/CMake files, static libraries; needs `onnx`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`onnx`](https://openqnx.github.io/oss/package/x86_64/onnx.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/onnx.html)) · [`onnx-dev`](https://openqnx.github.io/oss/package/x86_64/onnx-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/onnx-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add onnx` (dependencies come along); one version: `apk add onnx=<version>` | `apk del onnx` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install onnx` (dependencies first) | `oss-get remove onnx` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`onnx-dev` the same way, by its own name (`apk add onnx-dev`, `oss-get install onnx-dev`, by hand with its own file and list `onnx-dev.list`); remove it before `onnx`, which it needs.

```sh
# on the QNX target, as root
apk add onnx onnx-dev
apk add qnx-test-onnx && /usr/local/bin/qnx-port-tests/test_onnx    # its on-target test
apk del onnx onnx-dev qnx-test-onnx
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/onnx.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/onnx.list
rm var/lib/manual/onnx.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild onnx` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.17.0 --recurse-submodules --shallow-submodules \
    https://github.com/onnx/onnx.git src/onnx
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/onnx; B=build/$QNX_ARCH/onnx; mkdir -p $B
```

```bash
do_configure() {
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DONNX_BUILD_TESTS=OFF -DONNX_USE_PROTOBUF_SHARED_LIBS=ON -DBUILD_ONNX_PYTHON=OFF -DONNX_USE_LITE_PROTO=OFF \
    -DONNX_WERROR=OFF -DBUILD_SHARED_LIBS=ON -DProtobuf_PROTOC_EXECUTABLE="$(_host_protoc)" \
    -DPython3_EXECUTABLE=/usr/bin/python3 -DPYTHON_EXECUTABLE=/usr/bin/python3
}
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)"; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ONNX generates its schema sources with protoc while it builds, so it needs a protoc that runs on the build host -- and protobuf 29 gencode compiles only against the exact same runtime version, so it must be the host protoc the protobuf recipe builds (29.3), not a distribution one. The code generator scripts are Python and need the host interpreter, which the QNX toolchain file would otherwise look for in the target sysroot.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH onnx`.

### 5. What it installs

100 files under `/usr/local`: 2 shared-library entries, 94 headers.

```
/usr/local/lib/libonnx.so
/usr/local/lib/libonnx_proto.so
```

The complete list is `stage/<arch>/.manifest/onnx` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa onnx
ls dists/8.0/*/ | grep '^onnx-'
```

This writes `onnx-<ver>-r0.tar.gz` (run time) and `onnx-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add onnx
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/onnx-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/onnx-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_onnx`](../../tests/src/) links onnx the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-onnx`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-onnx
/usr/local/bin/qnx-port-tests/test_onnx
```

```
== onnx ==
  ok   ...
PASS onnx
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_onnx` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_onnx`](../../tests/src/) -- a program linked against onnx the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - operator schema registry knows Add-17
   - Add has 2 inputs, 1 output
   - Conv-17 schema registered
   - ModelProto serializes
   - ModelProto parses back
   - parsed model has the same nodes
   - checker accepts the valid model
   - shape inference runs
   - Y inferred as [2,4]
   - intermediate S inferred as [2,3] (broadcast)
   - checker (full_check) accepts the model from a file
   - checker rejects Add with one input

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | onnx 1.17.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/onnx-1.17.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | onnx 1.17.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/onnx-1.17.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | onnx 1.17.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/onnx-1.17.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | onnx 1.17.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/onnx-1.17.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.17.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add onnx=<version>`), sources in `ported/onnx/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates onnx`; port one with `scripts/port-update onnx`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.17.0; GIT=https://github.com/onnx/onnx.git; REF=v1.17.0
DEPS="protobuf"
# ONNX generates its schema sources with protoc while it builds, so it needs a
# protoc that runs on the build host -- and protobuf 29 gencode compiles only
# against the exact same runtime version, so it must be the host protoc the
# protobuf recipe builds (29.3), not a distribution one. The code generator
# scripts are Python and need the host interpreter, which the QNX toolchain
# file would otherwise look for in the target sysroot.
_host_protoc() {
  local HB="$BUILD_DIR/host/protobuf" p
  p=$(find "$HB" -maxdepth 1 -name 'protoc*' -type f -perm -u+x 2>/dev/null | head -1)
  if [ -z "$p" ]; then
    local PS="$SRC_DIR/protobuf"
    [ -d "$PS" ] || git clone -q --depth 1 -b v29.3 --recurse-submodules https://github.com/protocolbuffers/protobuf.git "$PS"
    cmake -S "$PS" -B "$HB" -G Ninja -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF \
      -Dprotobuf_ABSL_PROVIDER=module -DBUILD_SHARED_LIBS=OFF >&2 && cmake --build "$HB" --target protoc >&2
    p=$(find "$HB" -maxdepth 1 -name 'protoc*' -type f -perm -u+x | head -1)
  fi
  echo "$p"
}
do_configure() {
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DONNX_BUILD_TESTS=OFF -DONNX_USE_PROTOBUF_SHARED_LIBS=ON -DBUILD_ONNX_PYTHON=OFF -DONNX_USE_LITE_PROTO=OFF \
    -DONNX_WERROR=OFF -DBUILD_SHARED_LIBS=ON -DProtobuf_PROTOC_EXECUTABLE="$(_host_protoc)" \
    -DPython3_EXECUTABLE=/usr/bin/python3 -DPYTHON_EXECUTABLE=/usr/bin/python3
}
do_build() { cmake --build "$B" -j"$(nproc)"; }
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
