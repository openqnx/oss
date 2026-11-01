# highwayhash 0_git20240418

> Fast strong hash functions: SipHash/HighwayHash

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/google/highwayhash.git |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_highwayhash`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `highwayhash` (programs, shared libraries, run-time data) and `highwayhash-dev` (headers, pkg-config/CMake files, static libraries; needs `highwayhash`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`highwayhash`](https://openqnx.github.io/oss/package/x86_64/highwayhash.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/highwayhash.html)) · [`highwayhash-dev`](https://openqnx.github.io/oss/package/x86_64/highwayhash-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/highwayhash-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add highwayhash` (dependencies come along); one version: `apk add highwayhash=<version>` | `apk del highwayhash` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install highwayhash` (dependencies first) | `oss-get remove highwayhash` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`highwayhash-dev` the same way, by its own name (`apk add highwayhash-dev`, `oss-get install highwayhash-dev`, by hand with its own file and list `highwayhash-dev.list`); remove it before `highwayhash`, which it needs.

```sh
# on the QNX target, as root
apk add highwayhash highwayhash-dev
apk add qnx-test-highwayhash && /usr/local/bin/qnx-port-tests/test_highwayhash    # its on-target test, from tests-8.0-<arch>
apk del highwayhash highwayhash-dev qnx-test-highwayhash
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/highwayhash.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/highwayhash.list
rm var/lib/manual/highwayhash.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild highwayhash` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/google/highwayhash.git src/highwayhash
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/highwayhash < ports/highwayhash/patches/0001-cmake-install.patch
patch -p1 -d src/highwayhash < ports/highwayhash/patches/0002-no-strict-aliasing.patch
```

**[`0001-cmake-install.patch`](patches/0001-cmake-install.patch)** — CMake: a usable shared library (dispatch, soname, install); tests optional.

> The CMake build compiled a library nothing outside the source tree could
> use: it installed nothing, had no soname, and left out instruction_sets.cc
> (InstructionSets::Supported(), which the run-time dispatch to the AVX2,
> SSE4.1 or NEON code, InstructionSets::Run, calls) -- that file was only in
> the benchmarks' object library. This adds it to the library, gives the
> library the soname the Makefile gives it (libhighwayhash.so.0), and
> installs the library and every header under include/highwayhash/, as the
> Makefile's install target does.
>
> The tests, benchmarks and examples are built only with HH_BUILD_TESTS
> (default ON, as before): their os_specific.cc (thread affinity and
> priority for benchmarking) knows Windows, Linux, FreeBSD and macOS and
> stops with #error "port" anywhere else, QNX included.

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

**[`0002-no-strict-aliasing.patch`](patches/0002-no-strict-aliasing.patch)** — Build with -fno-strict-aliasing: the portable code type-puns, and GCC -O3 miscomputes it.

> hh_portable.h reads the input packets and lanes through pointer casts
> (reinterpret_cast of the byte buffer to HHPacket and uint64_t), which the
> C++ aliasing rules do not allow. GCC at -O3 -- the optimization the
> library is built with -- then reorders the loads, and the portable
> HighwayHash target (the one used where neither AVX2/SSE4.1 nor NEON is
> available, and HighwayHash<HH_TARGET_Portable> called directly) returns
> wrong hashes for every input longer than one 32-byte packet: for the test
> vector of 33 bytes 0xe224df579cf94ee5 instead of 0x2c90f73ca03181fc. Seen
> with GCC 12 (QNX SDP 8.0) and GCC 13 (Linux) alike; upstream's own test
> does not notice because it compiles its portable implementation in its
> own translation unit. With -fno-strict-aliasing all 65 reference values
> of highwayhash_test.cc match, for every target.

Touches: `CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/highwayhash -B build/$QNX_ARCH/highwayhash -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DHH_BUILD_TESTS=OFF
cmake --build build/$QNX_ARCH/highwayhash -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/highwayhash
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- highwayhash: Google's fast keyed hash functions -- HighwayHash (SIMD: AVX2/SSE4.1 on x86_64, NEON on aarch64, chosen at run time) and SipHash -- as libhighwayhash. Built for riegeli, which hashes its chunks with it. Upstream has no releases: the newest commit (2024-04-18).
- patches/0001: the CMake build makes the library usable from outside -- with the run-time target dispatch (instruction_sets.cc), a soname and install rules -- and builds its tests and benchmarks only on request (their timer code has no QNX port). patches/0002: -fno-strict-aliasing, without which GCC -O3 miscomputes the portable target (not QNX-specific).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH highwayhash`.

### 5. What it installs

35 files under `/usr/local`: 2 shared-library entries, 33 headers.

```
/usr/local/lib/libhighwayhash.so
/usr/local/lib/libhighwayhash.so.0
```

The complete list is `stage/<arch>/.manifest/highwayhash` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa highwayhash
ls dists/8.0/*/ | grep '^highwayhash-'
```

This writes `highwayhash-<ver>-r0.tar.gz` (run time) and `highwayhash-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add highwayhash
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/highwayhash-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/highwayhash-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_highwayhash`](../../tests/src/) links highwayhash the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-highwayhash`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-highwayhash
/usr/local/bin/qnx-port-tests/test_highwayhash
```

```
== highwayhash ==
  ok   ...
PASS highwayhash
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_highwayhash` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_highwayhash`](../../tests/src/) -- a program linked against highwayhash the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - HighwayHash-64, best target: 65 of 65 known values
   - HighwayHash-64, portable target: 65 of 65 known values
   - C binding HighwayHash64(): 65 of 65 known values
   - SipHash-2-4 reference vector

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | highwayhash 0_git20240418-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/highwayhash-0_git20240418-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | highwayhash 0_git20240418-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/highwayhash-0_git20240418-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0_git20240418**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add highwayhash=<version>`), sources in `ported/highwayhash/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates highwayhash`; port one with `scripts/port-update highwayhash`.

<details><summary>recipe.sh</summary>

```bash
# highwayhash: Google's fast keyed hash functions -- HighwayHash (SIMD:
# AVX2/SSE4.1 on x86_64, NEON on aarch64, chosen at run time) and SipHash --
# as libhighwayhash. Built for riegeli, which hashes its chunks with it.
# Upstream has no releases: the newest commit (2024-04-18).
VERSION=0_git20240418; GIT=https://github.com/google/highwayhash.git; COMMIT=f8381f3331d9c56a9792f9b4a35f61c41108c39e
DEPS=""
BUILDSYS=cmake
# patches/0001: the CMake build makes the library usable from outside --
# with the run-time target dispatch (instruction_sets.cc), a soname and
# install rules -- and builds its tests and benchmarks only on request
# (their timer code has no QNX port). patches/0002: -fno-strict-aliasing, without
# which GCC -O3 miscomputes the portable target (not QNX-specific).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DHH_BUILD_TESTS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
