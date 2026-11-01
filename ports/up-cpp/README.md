# up-cpp 1.0.1

> Eclipse uProtocol C++ library: data model and communication APIs

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/eclipse-uprotocol/up-cpp.git (tag `v1.0.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`protobuf`](../protobuf/) [`up-core-api`](../up-core-api/) |
| Run-time dependencies | [`abseil`](../abseil/) [`protobuf`](../protobuf/) [`up-core-api`](../up-core-api/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_up_cpp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `up-cpp` (programs, shared libraries, run-time data) and `up-cpp-dev` (headers, pkg-config/CMake files, static libraries; needs `up-cpp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`up-cpp`](https://openqnx.github.io/oss/package/x86_64/up-cpp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/up-cpp.html)) · [`up-cpp-dev`](https://openqnx.github.io/oss/package/x86_64/up-cpp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/up-cpp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add up-cpp` (dependencies come along); one version: `apk add up-cpp=<version>` | `apk del up-cpp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install up-cpp` (dependencies first) | `oss-get remove up-cpp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`up-cpp-dev` the same way, by its own name (`apk add up-cpp-dev`, `oss-get install up-cpp-dev`, by hand with its own file and list `up-cpp-dev.list`); remove it before `up-cpp`, which it needs.

```sh
# on the QNX target, as root
apk add up-cpp up-cpp-dev
apk add qnx-test-up-cpp && /usr/local/bin/qnx-port-tests/test_up_cpp    # its on-target test, from tests-8.0-<arch>-m-z
apk del up-cpp up-cpp-dev qnx-test-up-cpp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/up-cpp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/up-cpp.list
rm var/lib/manual/up-cpp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild up-cpp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild protobuf up-core-api
```

<details><summary>Full build chain (4)</summary>

[`abseil`](../abseil/) [`protobuf`](../protobuf/) [`up-core-api`](../up-core-api/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.0.1 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-uprotocol/up-cpp.git src/up-cpp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/up-cpp < ports/up-cpp/patches/0001-cmake-package.patch
```

**[`0001-cmake-package.patch`](patches/0001-cmake-package.patch)** — CMake: install the up-cpp package, tests optional, no gcov in release builds

> up-cpp is built with Conan upstream: its CMakeLists installs the library and
> headers but no CMake package, links libgcov into every build, and always
> configures its unit tests (which need GoogleTest). Without Conan:
> - install up-cpp::up-cpp (up-cppConfig.cmake, finding protobuf and
>   up-core-api), which up-transport-zenoh-cpp's find_package(up-cpp) needs;
> - up-core-api and protobuf are PUBLIC link dependencies (up-cpp's headers
>   include the generated .pb.h files) instead of include directories that
>   would export the build machine's staging path;
> - the -Werror warning set is PRIVATE: as PUBLIC it was imposed on every
>   program built against up-cpp;
> - a versioned shared library (libup-cpp.so.1);
> - -lgcov only in the Coverage build, the tests only with BUILD_TESTING
>   (CTest's switch).

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/up-cpp -B build/$QNX_ARCH/up-cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON
cmake --build build/$QNX_ARCH/up-cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/up-cpp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Eclipse uProtocol's C++ library: builders, serializers and validators for the uProtocol data model (UUri, UUID v8, UMessage) and the communication layer (Publisher/Subscriber, RPC client/server, notifications) over any UTransport. 1.0.1 is the release qnx-ports' branch qnx-v1.0.1 and the up-conan-recipes pair with up-core-api 1.6.0-alpha2. qnx-ports' QNX changes are to the unit tests (no -lpthread, gtest_add_tests when cross compiling), which are not built here; the library needs none. patches/0001: the CMake package (up-cpp::up-cpp) Conan would generate, a versioned shared library, tests only with BUILD_TESTING, and -lgcov only for the Coverage build.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH up-cpp`.

### 5. What it installs

26 files under `/usr/local`: 3 shared-library entries, 19 headers.

```
/usr/local/lib/libup-cpp.so
/usr/local/lib/libup-cpp.so.1
/usr/local/lib/libup-cpp.so.1.0.1
```

The complete list is `stage/<arch>/.manifest/up-cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa up-cpp
ls dists/8.0/*/ | grep '^up-cpp-'
```

This writes `up-cpp-<ver>-r0.tar.gz` (run time) and `up-cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add up-cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/up-cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/up-cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_up_cpp`](../../tests/src/) links up-cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-up-cpp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-up-cpp
/usr/local/bin/qnx-port-tests/test_up_cpp
```

```
== up-cpp ==
  ok   ...
PASS up-cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_up_cpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_up_cpp`](../../tests/src/) -- a program linked against up-cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - UUID msb = time<<16 | version 7 | rand_a
   - UUID lsb = variant 10b | rand_b
   - UUID string 018bcfe5-687b-7def-8123-456789abcdef
   - UUID string parses back
   - UUID as 16 bytes
   - validator: version 7, variant 2
   - validator: the time it was built with
   - isUuid
   - parse //vehicle1/110AB/3/80CD into authority, ue_id, version, resource
   - and serialize it back
   - resource 0x80CD is a valid publish topic
   - instance id 0 (ue_id 0x10AB) is a wildcard, not a topic
   - resource 0x0001 is not a topic
   - but a valid RPC method
   - a non-hex entity id is rejected
   - built a PUBLISH message
   - source is the topic
   - priority CS2
   - text payload
   - message id is a UUIDv7
   - isValidPublish
   - not a valid RPC request
   - protobuf-serialize the UMessage
   - parse it back
   - with id, source and payload intact
   - ... and 1 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | up-cpp 1.0.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/up-cpp-1.0.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | up-cpp 1.0.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/up-cpp-1.0.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add up-cpp=<version>`), sources in `ported/up-cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates up-cpp`; port one with `scripts/port-update up-cpp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.1; GIT=https://github.com/eclipse-uprotocol/up-cpp.git; REF=v1.0.1
# Eclipse uProtocol's C++ library: builders, serializers and validators for
# the uProtocol data model (UUri, UUID v8, UMessage) and the communication
# layer (Publisher/Subscriber, RPC client/server, notifications) over any
# UTransport. 1.0.1 is the release qnx-ports' branch qnx-v1.0.1 and the
# up-conan-recipes pair with up-core-api 1.6.0-alpha2. qnx-ports' QNX changes
# are to the unit tests (no -lpthread, gtest_add_tests when cross compiling),
# which are not built here; the library needs none.
# patches/0001: the CMake package (up-cpp::up-cpp) Conan would generate, a
# versioned shared library, tests only with BUILD_TESTING, and -lgcov only for
# the Coverage build.
DEPS="protobuf up-core-api"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
