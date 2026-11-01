# up-core-api 1.6.0-alpha2

> uProtocol data model and core service APIs (protobuf, compiled for C++)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/eclipse-uprotocol/up-spec.git (tag `v1.6.0-alpha.2`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`protobuf`](../protobuf/) |
| Run-time dependencies | [`abseil`](../abseil/) [`protobuf`](../protobuf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_up_core_api`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `up-core-api` (programs, shared libraries, run-time data) and `up-core-api-dev` (headers, pkg-config/CMake files, static libraries; needs `up-core-api`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`up-core-api`](https://openqnx.github.io/oss/package/x86_64/up-core-api.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/up-core-api.html)) · [`up-core-api-dev`](https://openqnx.github.io/oss/package/x86_64/up-core-api-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/up-core-api-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add up-core-api` (dependencies come along); one version: `apk add up-core-api=<version>` | `apk del up-core-api` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install up-core-api` (dependencies first) | `oss-get remove up-core-api` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`up-core-api-dev` the same way, by its own name (`apk add up-core-api-dev`, `oss-get install up-core-api-dev`, by hand with its own file and list `up-core-api-dev.list`); remove it before `up-core-api`, which it needs.

```sh
# on the QNX target, as root
apk add up-core-api up-core-api-dev
apk add qnx-test-up-core-api && /usr/local/bin/qnx-port-tests/test_up_core_api    # its on-target test, from tests-8.0-<arch>-m-z
apk del up-core-api up-core-api-dev qnx-test-up-core-api
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/up-core-api.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/up-core-api.list
rm var/lib/manual/up-core-api.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild up-core-api` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.6.0-alpha.2 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-uprotocol/up-spec.git src/up-core-api
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/up-core-api < ports/up-core-api/patches/0001-cmake-build.patch
```

**[`0001-cmake-build.patch`](patches/0001-cmake-build.patch)** — Add a CMake build: generate and compile the C++ sources, install a package

> up-spec ships only the .proto files. This is the CMakeLists.txt of
> uProtocol's Conan recipe (up-conan-recipes, up-core-api/CMakeLists.txt; the
> same file in qnx-ports' fork), changed for a build without Conan: an installed
> CMake package (up-core-api::up-core-api, up-core-api_INCLUDE_DIR -- what
> up-cpp and up-transport-zenoh-cpp look for, and what Conan's CMakeDeps would
> generate), a versioned shared library, the well-known protobuf types on
> protoc's import path, the .proto files installed to share/up-core-api, and
> UP_CORE_API_PROTOC to name a protoc that runs on the build machine (cross
> builds: the staged protoc is a QNX program).

Touches: `CMakeLists.txt` · Upstream status: QNX-specific (from up-conan-recipes, github.com/qnx-ports/up-conan-recipes branch qnx-main)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/up-core-api; B=build/$QNX_ARCH/up-core-api; mkdir -p $B
```

```bash
do_configure() {
  local HOST_PROTOC
  HOST_PROTOC=$(find "$BUILD_DIR/host/protobuf" \( -type f -o -type l \) -name protoc | head -1)
  [ -x "$HOST_PROTOC" ] || { echo "no host protoc: build the protobuf component first"; return 1; }
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DBUILD_SHARED_LIBS=ON -DUP_CORE_API_PROTOC="$HOST_PROTOC"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The uProtocol data model (UUri, UUID, UAttributes, UMessage, UStatus) and core service interfaces (uSubscription, uDiscovery, uTwin) as Protocol Buffers, compiled for C++: the "up-core-api" library up-cpp builds on. The up-spec repository has only the .proto files; uProtocol's Conan recipe (github.com/eclipse-uprotocol/up-conan-recipes, which qnx-ports' up-conan-recipes follows) supplies the CMakeLists.txt -- patches/0001, here also a shared library and the CMake package (up-core-api::up-core-api) Conan would otherwise generate. 1.6.0-alpha2 is the version that recipe pairs with up-cpp 1.0.1.
- The .pb.cc files are generated with the protobuf component's build-machine protoc (29.3, the runtime's version: protobuf 29 gencode compiles only against its own runtime); the staged protoc is a QNX program.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH up-core-api`.

### 5. What it installs

29 files under `/usr/local`: 3 shared-library entries, 11 headers.

```
/usr/local/lib/libup-core-api.so
/usr/local/lib/libup-core-api.so.1
/usr/local/lib/libup-core-api.so.1.6.0
```

The complete list is `stage/<arch>/.manifest/up-core-api` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa up-core-api
ls dists/8.0/*/ | grep '^up-core-api-'
```

This writes `up-core-api-<ver>-r0.tar.gz` (run time) and `up-core-api-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add up-core-api
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/up-core-api-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/up-core-api-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_up_core_api`](../../tests/src/) links up-core-api the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-up-core-api`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-up-core-api
/usr/local/bin/qnx-port-tests/test_up_core_api
```

```
== up-core-api ==
  ok   ...
PASS up-core-api
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_up_core_api` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_up_core_api`](../../tests/src/) -- a program linked against up-core-api the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - serialize a UUri
   - UUri wire bytes 0a 01 61 10 01 18 02 20 81 80 02
   - parse them back
   - descriptor full name uprotocol.v1.UUri
   - UCode NOT_FOUND = 5
   - UStatus round trip
   - UMessage round trip (attributes, id, payload)
   - uSubscription option service_name = core.usubscription
   - service_version_major = 3
   - Subscribe method_id = 1

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | up-core-api 1.6.0_alpha2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/up-core-api-1.6.0_alpha2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | up-core-api 1.6.0_alpha2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/up-core-api-1.6.0_alpha2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.6.0-alpha2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add up-core-api=<version>`), sources in `ported/up-core-api/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates up-core-api`; port one with `scripts/port-update up-core-api`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.6.0-alpha2; GIT=https://github.com/eclipse-uprotocol/up-spec.git; REF=v1.6.0-alpha.2
# The uProtocol data model (UUri, UUID, UAttributes, UMessage, UStatus) and
# core service interfaces (uSubscription, uDiscovery, uTwin) as Protocol
# Buffers, compiled for C++: the "up-core-api" library up-cpp builds on. The
# up-spec repository has only the .proto files; uProtocol's Conan recipe
# (github.com/eclipse-uprotocol/up-conan-recipes, which qnx-ports'
# up-conan-recipes follows) supplies the CMakeLists.txt -- patches/0001, here
# also a shared library and the CMake package (up-core-api::up-core-api)
# Conan would otherwise generate. 1.6.0-alpha2 is the version that recipe
# pairs with up-cpp 1.0.1.
DEPS="protobuf"
# The .pb.cc files are generated with the protobuf component's build-machine
# protoc (29.3, the runtime's version: protobuf 29 gencode compiles only
# against its own runtime); the staged protoc is a QNX program.
do_configure() {
  local HOST_PROTOC
  HOST_PROTOC=$(find "$BUILD_DIR/host/protobuf" \( -type f -o -type l \) -name protoc | head -1)
  [ -x "$HOST_PROTOC" ] || { echo "no host protoc: build the protobuf component first"; return 1; }
  cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DBUILD_SHARED_LIBS=ON -DUP_CORE_API_PROTOC="$HOST_PROTOC"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
