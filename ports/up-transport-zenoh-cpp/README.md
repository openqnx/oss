# up-transport-zenoh-cpp 1.0.0-git-20250319

> uProtocol transport over Eclipse Zenoh for up-cpp (zenoh-cpp on zenoh-c)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/eclipse-uprotocol/up-transport-zenoh-cpp.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zenoh-cpp`](../zenoh-cpp/) [`zenoh-c`](../zenoh-c/) [`up-cpp`](../up-cpp/) [`up-core-api`](../up-core-api/) [`protobuf`](../protobuf/) [`spdlog`](../spdlog/) |
| Run-time dependencies | [`abseil`](../abseil/) [`fmt`](../fmt/) [`protobuf`](../protobuf/) [`spdlog`](../spdlog/) [`up-core-api`](../up-core-api/) [`up-cpp`](../up-cpp/) [`zenoh-c`](../zenoh-c/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_up_transport_zenoh_cpp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `up-transport-zenoh-cpp` (programs, shared libraries, run-time data) and `up-transport-zenoh-cpp-dev` (headers, pkg-config/CMake files, static libraries; needs `up-transport-zenoh-cpp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`up-transport-zenoh-cpp`](https://openqnx.github.io/oss/package/x86_64/up-transport-zenoh-cpp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/up-transport-zenoh-cpp.html)) · [`up-transport-zenoh-cpp-dev`](https://openqnx.github.io/oss/package/x86_64/up-transport-zenoh-cpp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/up-transport-zenoh-cpp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add up-transport-zenoh-cpp` (dependencies come along); one version: `apk add up-transport-zenoh-cpp=<version>` | `apk del up-transport-zenoh-cpp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install up-transport-zenoh-cpp` (dependencies first) | `oss-get remove up-transport-zenoh-cpp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`up-transport-zenoh-cpp-dev` the same way, by its own name (`apk add up-transport-zenoh-cpp-dev`, `oss-get install up-transport-zenoh-cpp-dev`, by hand with its own file and list `up-transport-zenoh-cpp-dev.list`); remove it before `up-transport-zenoh-cpp`, which it needs.

```sh
# on the QNX target, as root
apk add up-transport-zenoh-cpp up-transport-zenoh-cpp-dev
apk add qnx-test-up-transport-zenoh-cpp && /usr/local/bin/qnx-port-tests/test_up_transport_zenoh_cpp    # its on-target test, from tests-8.0-<arch>-m-z
apk del up-transport-zenoh-cpp up-transport-zenoh-cpp-dev qnx-test-up-transport-zenoh-cpp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/up-transport-zenoh-cpp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/up-transport-zenoh-cpp.list
rm var/lib/manual/up-transport-zenoh-cpp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild up-transport-zenoh-cpp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zenoh-cpp zenoh-c up-cpp up-core-api protobuf spdlog
```

<details><summary>Full build chain (11)</summary>

[`abseil`](../abseil/) [`fmt`](../fmt/) [`protobuf`](../protobuf/) [`rust`](../rust/) [`spdlog`](../spdlog/) [`up-core-api`](../up-core-api/) [`up-cpp`](../up-cpp/) [`zenoh-c`](../zenoh-c/) [`zenoh-cpp`](../zenoh-cpp/) [`zenoh-pico`](../zenoh-pico/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-uprotocol/up-transport-zenoh-cpp.git src/up-transport-zenoh-cpp
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/up-transport-zenoh-cpp < ports/up-transport-zenoh-cpp/patches/0001-cmake-package.patch
```

**[`0001-cmake-package.patch`](patches/0001-cmake-package.patch)** — CMake: zenoh-cpp's package, install a CMake package, tests optional

> up-transport-zenoh-cpp is built with Conan upstream, which supplies a
> "zenohcpp::lib" target and generates the CMake packages. Without Conan:
> - zenoh-cpp's own package: find_package(zenohc) + find_package(zenohcxx), and
>   the zenoh-c backend target zenohcxx::zenohc;
> - zenoh-cpp, up-cpp, up-core-api and protobuf are PUBLIC link dependencies
>   (ZenohUTransport.h includes zenoh.hxx and up-cpp's headers) instead of
>   include directories that would export the build machine's staging path;
> - install up-transport-zenoh-cpp::up-transport-zenoh-cpp
>   (up-transport-zenoh-cppConfig.cmake) and a versioned shared library;
> - the tests only with BUILD_TESTING (CTest's switch); they need GoogleTest;
> - ZenohUTransport.h defines ZENOHCXX_ZENOHC only when it is not defined yet:
>   zenohcxx::zenohc passes -DZENOHCXX_ZENOHC, and the redefinition is an error
>   under the project's -Werror.

Touches: `CMakeLists.txt`, `include/up-transport-zenoh-cpp/ZenohUTransport.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/up-transport-zenoh-cpp -B build/$QNX_ARCH/up-transport-zenoh-cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON
cmake --build build/$QNX_ARCH/up-transport-zenoh-cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/up-transport-zenoh-cpp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- uProtocol's UTransport over Eclipse Zenoh (zenoh-cpp on zenoh-c), for up-cpp. The last release, 1.0.0-rc3 (what qnx-ports' branch qnx-v1.0.0-rc3 ports), is written against zenoh-cpp 1.0.0-rc5; this commit moved it to zenoh 1.2.1, the version zenoh-c and zenoh-cpp are at here, and is the last one that builds against up-cpp 1.0.1 (the next changes UTransport::cleanupListener's signature, which is in up-cpp after 1.0.1). qnx-ports' QNX changes switch the backend to zenoh-pico; zenoh-c is ported here, so its full backend (configuration files, all transports) is used and they are not needed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH up-transport-zenoh-cpp`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libup-transport-zenoh-cpp.so
/usr/local/lib/libup-transport-zenoh-cpp.so.1
/usr/local/lib/libup-transport-zenoh-cpp.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/up-transport-zenoh-cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa up-transport-zenoh-cpp
ls dists/8.0/*/ | grep '^up-transport-zenoh-cpp-'
```

This writes `up-transport-zenoh-cpp-<ver>-r0.tar.gz` (run time) and `up-transport-zenoh-cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add up-transport-zenoh-cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/up-transport-zenoh-cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/up-transport-zenoh-cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_up_transport_zenoh_cpp`](../../tests/src/) links up-transport-zenoh-cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-up-transport-zenoh-cpp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-up-transport-zenoh-cpp
/usr/local/bin/qnx-port-tests/test_up_transport_zenoh_cpp
```

```
== up-transport-zenoh-cpp ==
  ok   ...
PASS up-transport-zenoh-cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_up_transport_zenoh_cpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_up_transport_zenoh_cpp`](../../tests/src/) -- a program linked against up-transport-zenoh-cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - write two Zenoh configuration files
   - transport A open (listening on tcp/127.0.0.1:17450)
   - transport B open (connected to A)
   - A's entity URI
   - subscribe to //vehicle1/110AB/1/8001 on B
   - publish on A returns OK
   - B's subscriber received the message
   - payload intact
   - type PUBLISH
   - source is the topic
   - priority CS4
   - payload format TEXT
   - id is a UUIDv7
   - no exception

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | up-transport-zenoh-cpp 1.0.0_git20250319-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/up-transport-zenoh-cpp-1.0.0_git20250319-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | up-transport-zenoh-cpp 1.0.0_git20250319-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/up-transport-zenoh-cpp-1.0.0_git20250319-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0.0-git-20250319**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add up-transport-zenoh-cpp=<version>`), sources in `ported/up-transport-zenoh-cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates up-transport-zenoh-cpp`; port one with `scripts/port-update up-transport-zenoh-cpp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.0.0-git-20250319; GIT=https://github.com/eclipse-uprotocol/up-transport-zenoh-cpp.git; COMMIT=04678fc6035e9c88bb088b95960924a8f1987fd8
# uProtocol's UTransport over Eclipse Zenoh (zenoh-cpp on zenoh-c), for
# up-cpp. The last release, 1.0.0-rc3 (what qnx-ports' branch
# qnx-v1.0.0-rc3 ports), is written against zenoh-cpp 1.0.0-rc5; this commit
# moved it to zenoh 1.2.1, the version zenoh-c and zenoh-cpp are at here, and
# is the last one that builds against up-cpp 1.0.1 (the next changes
# UTransport::cleanupListener's signature, which is in up-cpp after 1.0.1).
# qnx-ports' QNX changes switch the backend to zenoh-pico; zenoh-c is ported
# here, so its full backend (configuration files, all transports) is used and
# they are not needed.
DEPS="zenoh-cpp zenoh-c up-cpp up-core-api protobuf spdlog"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
