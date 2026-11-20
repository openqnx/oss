# tsl-robin-map 1.4.1

> tsl::robin_map and tsl::robin_set: fast C++ hash map and set with robin hood hashing, header-only

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/Tessil/robin-map.git (tag `v1.4.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_tsl_robin_map`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `tsl-robin-map-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`tsl-robin-map-dev`](https://openqnx.github.io/oss/package/x86_64/tsl-robin-map-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tsl-robin-map-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add tsl-robin-map-dev` (dependencies come along); one version: `apk add tsl-robin-map-dev=<version>` | `apk del tsl-robin-map-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install tsl-robin-map-dev` (dependencies first) | `oss-get remove tsl-robin-map-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-tsl-robin-map && /usr/local/bin/qnx-port-tests/test_tsl_robin_map    # its on-target test, from tests-8.0-<arch>-m-z
apk del tsl-robin-map-dev qnx-test-tsl-robin-map
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/tsl-robin-map-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/tsl-robin-map-dev.list
rm var/lib/manual/tsl-robin-map-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tsl-robin-map` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.4.1 --recurse-submodules --shallow-submodules \
    https://github.com/Tessil/robin-map.git src/tsl-robin-map
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/tsl-robin-map -B build/$QNX_ARCH/tsl-robin-map -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DTSL_ROBIN_MAP_ENABLE_INSTALL=ON
cmake --build build/$QNX_ARCH/tsl-robin-map -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/tsl-robin-map
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tsl-robin-map: tsl::robin_map and tsl::robin_set, C++ hash map and hash set with open addressing and robin hood (backward shift deletion) hashing -- drop-in alternatives to std::unordered_map/set that keep their entries in one contiguous array. Header-only: include/tsl/ and the CMake package tsl-robin-map (target tsl::robin_map) in share/cmake/tsl-robin-map. kiss-icp's voxel hash map is built on it. Nothing had to change for QNX; upstream's tests (Boost.Test) are a separate CMake project and are not built. tests/src/test_tsl_robin_map.cpp checks the containers on the target.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tsl-robin-map`.

### 5. What it installs

7 files under `/usr/local`, 4 headers.

The complete list is `stage/<arch>/.manifest/tsl-robin-map` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tsl-robin-map
ls dists/8.0/*/ | grep '^tsl-robin-map-'
```

This writes `tsl-robin-map-<ver>-r0.tar.gz` (run time) and `tsl-robin-map-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tsl-robin-map
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tsl-robin-map-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tsl-robin-map-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_tsl_robin_map`](../../tests/src/) links tsl-robin-map the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-tsl-robin-map`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-tsl-robin-map
/usr/local/bin/qnx-port-tests/test_tsl_robin_map
```

```
== tsl-robin-map ==
  ok   ...
PASS tsl-robin-map
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tsl_robin_map` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_tsl_robin_map`](../../tests/src/) -- a program linked against tsl-robin-map the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - a new map is empty
   - 10000 squares inserted; iteration visits each once
   - lookups hit and miss correctly
   - at() on a missing key throws std::out_of_range
   - load factor within its maximum after growth
   - insert of an existing key keeps the old value
   - insert_or_assign overwrites
   - try_emplace adds a new key
   - 5000 odd keys erased, all even keys still found
   - erase through iterators removes the 2501 multiples of 4
   - rehash keeps the contents
   - clear empties the map
   - 50000 mixed operations leave the same contents as std::unordered_map
   - a hash with only 4 distinct values still stores and erases correctly
   - lookup by string_view without building a std::string
   - it.value() modifies in place
   - copy equals the moved-to map; the moved-from map is empty
   - robin_set keeps 100 distinct values of 1000 inserts
   - default growth policy: bucket count is a power of two
   - prime growth policy: odd bucket count, contents intact
   - serialize / deserialize restores 100 entries

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tsl-robin-map 1.4.1-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/tsl-robin-map-1.4.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tsl-robin-map 1.4.1-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/tsl-robin-map-1.4.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.4.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tsl-robin-map=<version>`), sources in `ported/tsl-robin-map/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tsl-robin-map`; port one with `scripts/port-update tsl-robin-map`.

<details><summary>recipe.sh</summary>

```bash
# tsl-robin-map: tsl::robin_map and tsl::robin_set, C++ hash map and hash set
# with open addressing and robin hood (backward shift deletion) hashing --
# drop-in alternatives to std::unordered_map/set that keep their entries in
# one contiguous array. Header-only: include/tsl/ and the CMake package
# tsl-robin-map (target tsl::robin_map) in share/cmake/tsl-robin-map.
# kiss-icp's voxel hash map is built on it.
#
# Nothing had to change for QNX; upstream's tests (Boost.Test) are a separate
# CMake project and are not built. tests/src/test_tsl_robin_map.cpp checks
# the containers on the target.
VERSION=1.4.1; GIT=https://github.com/Tessil/robin-map.git; REF=v1.4.1
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DTSL_ROBIN_MAP_ENABLE_INSTALL=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
