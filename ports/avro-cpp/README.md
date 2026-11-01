# avro-cpp 1.12.2

> C++ library for Apache Avro, a data serialization system

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/apache/avro.git (tag `release-1.12.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`fmt`](../fmt/) [`zlib`](../zlib/) [`snappy`](../snappy/) [`zstd`](../zstd/) |
| Run-time dependencies | [`snappy`](../snappy/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_avro_cpp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `avro-cpp` (programs, shared libraries, run-time data) and `avro-cpp-dev` (headers, pkg-config/CMake files, static libraries; needs `avro-cpp`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`avro-cpp`](https://openqnx.github.io/oss/package/x86_64/avro-cpp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/avro-cpp.html)) · [`avro-cpp-dev`](https://openqnx.github.io/oss/package/x86_64/avro-cpp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/avro-cpp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add avro-cpp` (dependencies come along); one version: `apk add avro-cpp=<version>` | `apk del avro-cpp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install avro-cpp` (dependencies first) | `oss-get remove avro-cpp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`avro-cpp-dev` the same way, by its own name (`apk add avro-cpp-dev`, `oss-get install avro-cpp-dev`, by hand with its own file and list `avro-cpp-dev.list`); remove it before `avro-cpp`, which it needs.

```sh
# on the QNX target, as root
apk add avro-cpp avro-cpp-dev
apk add qnx-test-avro-cpp && /usr/local/bin/qnx-port-tests/test_avro_cpp    # its on-target test, from tests-8.0-<arch>
apk del avro-cpp avro-cpp-dev qnx-test-avro-cpp
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/avro-cpp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/avro-cpp.list
rm var/lib/manual/avro-cpp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild avro-cpp` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild fmt zlib snappy zstd
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-1.12.2 --recurse-submodules --shallow-submodules \
    https://github.com/apache/avro.git src/avro-cpp
```

The project lives in the `lang/c++/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/avro-cpp/lang/c++ -B build/$QNX_ARCH/avro-cpp -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DAVRO_BUILD_TESTS=OFF \
      -DAVRO_BUILD_STATIC=OFF \
      -DAVRO_BUILD_SHARED=ON \
      -DAVRO_BUILD_EXECUTABLES=ON \
      -DAVRO_USE_BOOST=OFF
cmake --build build/$QNX_ARCH/avro-cpp -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/avro-cpp
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Apache Avro C++: Avro schemas, binary and JSON encoders/decoders, generic datums, object container files (null, deflate, snappy and zstd codecs) and avrogencpp, the schema-to-C++ code generator. The release tag of the Avro tree, C++ in lang/c++. Not the avro-cpp release tarball: 1.12.2's lacks cmake/avro-cpp-config.cmake.in, which CMakeLists.txt configures, so it does not build as released.
- fmt is used header-only at build time (no run-time dependency); without a staged fmt, CMakeLists.txt would FetchContent fmt 10.2.1 from GitHub.
- The tests need Boost; the library itself does not (AVRO_USE_BOOST=OFF).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH avro-cpp`.

### 5. What it installs

46 files under `/usr/local`: 2 shared-library entries, 1 programs, 39 headers.

```
/usr/local/lib/libavrocpp.so
/usr/local/lib/libavrocpp.so.1.12.2
/usr/local/bin/avrogencpp
```

The complete list is `stage/<arch>/.manifest/avro-cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa avro-cpp
ls dists/8.0/*/ | grep '^avro-cpp-'
```

This writes `avro-cpp-<ver>-r0.tar.gz` (run time) and `avro-cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add avro-cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/avro-cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/avro-cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_avro_cpp`](../../tests/src/) links avro-cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-avro-cpp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-avro-cpp
/usr/local/bin/qnx-port-tests/test_avro_cpp
```

```
== avro-cpp ==
  ok   ...
PASS avro-cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_avro_cpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_avro_cpp`](../../tests/src/) -- a program linked against avro-cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - record schema with 10 fields compiled
   - full name qnx.test.Reading
   - binary encoding matches the spec (02 7f 04 61 62)
   - binary round trip of a generic record
   - union branch string kept
   - JSON encoding (union as {\"string\": ...})
   - JSON decode gives the same record
   - resolving decoder: int promoted to long, default filled in
   - avrogencpp generates a C++ struct and codec_traits
   - unknown type in a schema: avro::Exception
   - no exception

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | avro-cpp 1.12.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/avro-cpp-1.12.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | avro-cpp 1.12.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/avro-cpp-1.12.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.12.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add avro-cpp=<version>`), sources in `ported/avro-cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates avro-cpp`; port one with `scripts/port-update avro-cpp`.

<details><summary>recipe.sh</summary>

```bash
# Apache Avro C++: Avro schemas, binary and JSON encoders/decoders, generic
# datums, object container files (null, deflate, snappy and zstd codecs) and
# avrogencpp, the schema-to-C++ code generator.
# The release tag of the Avro tree, C++ in lang/c++. Not the avro-cpp release
# tarball: 1.12.2's lacks cmake/avro-cpp-config.cmake.in, which
# CMakeLists.txt configures, so it does not build as released.
VERSION=1.12.2; GIT=https://github.com/apache/avro.git; REF=release-1.12.2
SUBDIR=lang/c++
# fmt is used header-only at build time (no run-time dependency); without a
# staged fmt, CMakeLists.txt would FetchContent fmt 10.2.1 from GitHub.
DEPS="fmt zlib snappy zstd"
# The tests need Boost; the library itself does not (AVRO_USE_BOOST=OFF).
CMAKE_ARGS_EXTRA=( -DAVRO_BUILD_TESTS=OFF -DAVRO_BUILD_STATIC=OFF -DAVRO_BUILD_SHARED=ON
                   -DAVRO_BUILD_EXECUTABLES=ON -DAVRO_USE_BOOST=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
