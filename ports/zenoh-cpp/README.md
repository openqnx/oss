# zenoh-cpp 1.2.1

> C++ bindings for Eclipse Zenoh (header-only, over zenoh-c or zenoh-pico)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/eclipse-zenoh/zenoh-cpp.git (tag `1.2.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`zenoh-c`](../zenoh-c/) [`zenoh-pico`](../zenoh-pico/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zenoh_cpp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `zenoh-cpp-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zenoh-cpp-dev`](https://openqnx.github.io/oss/package/x86_64/zenoh-cpp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zenoh-cpp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zenoh-cpp-dev` (dependencies come along); one version: `apk add zenoh-cpp-dev=<version>` | `apk del zenoh-cpp-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zenoh-cpp-dev` (dependencies first) | `oss-get remove zenoh-cpp-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-zenoh-cpp && /usr/local/bin/qnx-port-tests/test_zenoh_cpp    # its on-target test, from tests-8.0-<arch>-m-z
apk del zenoh-cpp-dev qnx-test-zenoh-cpp
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zenoh-cpp-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zenoh-cpp-dev.list
rm var/lib/manual/zenoh-cpp-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zenoh-cpp` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zenoh-c zenoh-pico
```

<details><summary>Full build chain (3)</summary>

[`rust`](../rust/) [`zenoh-c`](../zenoh-c/) [`zenoh-pico`](../zenoh-pico/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.2.1 --recurse-submodules --shallow-submodules \
    https://github.com/eclipse-zenoh/zenoh-cpp.git src/zenoh-cpp
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/zenoh-cpp -B build/$QNX_ARCH/zenoh-cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DZENOHCXX_ZENOHC=ON \
      -DZENOHCXX_ZENOHPICO=ON \
      -DZENOHCXX_ENABLE_TESTS=OFF \
      -DZENOHCXX_ENABLE_EXAMPLES=OFF \
      -DZENOHCXX_EXAMPLES_PROTOBUF=OFF
cmake --build build/$QNX_ARCH/zenoh-cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/zenoh-cpp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- C++ bindings for Eclipse Zenoh: headers only, over either zenoh-c (the full Rust implementation, ports/zenoh-c) or zenoh-pico. 1.2.1 is the release both of those are at here. qnx-ports' branch qnx-1.2.1 changes only the tests' CMake (no zenohd router when cross compiling); the headers need nothing for QNX. Configured with both backends, so the installed zenohcxxConfig.cmake gives zenohcxx::zenohc and zenohcxx::zenohpico to whichever of zenohc / zenohpico a consumer found first. Tests, examples and docs are not built.
- Headers only: what a program built with them loads is the backend.
- The zenoh-c and zenoh-pico submodules are not needed: the staged ones are used.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zenoh-cpp`.

### 5. What it installs

64 files under `/usr/local`, 61 headers.

The complete list is `stage/<arch>/.manifest/zenoh-cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zenoh-cpp
ls dists/8.0/*/ | grep '^zenoh-cpp-'
```

This writes `zenoh-cpp-<ver>-r0.tar.gz` (run time) and `zenoh-cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zenoh-cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zenoh-cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zenoh-cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zenoh_cpp`](../../tests/src/) links zenoh-cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zenoh-cpp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zenoh-cpp
/usr/local/bin/qnx-port-tests/test_zenoh_cpp
```

```
== zenoh-cpp ==
  ok   ...
PASS zenoh-cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zenoh_cpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zenoh_cpp`](../../tests/src/) -- a program linked against zenoh-cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - ext::serialize/deserialize pair<string,int32>
   - Bytes from a string and back
   - key expression qnx/a/b intersects qnx/*/b
   - qnx/a/b does not intersect qnx/c/*
   - open a session listening on TCP 127.0.0.1:17449
   - open a second session connecting to it
   - the sessions have different ids
   - the subscriber received a sample over TCP
   - with the payload intact
   - get qnx/port-test/calc?a=19;b=23 replies 42
   - no zenoh exception

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> zenoh-cpp`.

## Versions

This recipe builds **1.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zenoh-cpp=<version>`), sources in `ported/zenoh-cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zenoh-cpp`; port one with `scripts/port-update zenoh-cpp`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.2.1; GIT=https://github.com/eclipse-zenoh/zenoh-cpp.git; REF=1.2.1
# C++ bindings for Eclipse Zenoh: headers only, over either zenoh-c (the
# full Rust implementation, ports/zenoh-c) or zenoh-pico. 1.2.1 is the
# release both of those are at here. qnx-ports' branch qnx-1.2.1 changes only
# the tests' CMake (no zenohd router when cross compiling); the headers need
# nothing for QNX.
# Configured with both backends, so the installed zenohcxxConfig.cmake gives
# zenohcxx::zenohc and zenohcxx::zenohpico to whichever of zenohc /
# zenohpico a consumer found first. Tests, examples and docs are not built.
DEPS="zenoh-c zenoh-pico"
# Headers only: what a program built with them loads is the backend.
RUNTIME_DEPS="zenoh-c"
CMAKE_ARGS_EXTRA=( -DZENOHCXX_ZENOHC=ON -DZENOHCXX_ZENOHPICO=ON -DZENOHCXX_ENABLE_TESTS=OFF
                   -DZENOHCXX_ENABLE_EXAMPLES=OFF -DZENOHCXX_EXAMPLES_PROTOBUF=OFF )
# The zenoh-c and zenoh-pico submodules are not needed: the staged ones are used.
do_fetch() { git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
