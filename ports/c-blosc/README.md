# c-blosc 1.21.6

> Blocking, shuffling and lossless compression library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/Blosc/c-blosc.git (tag `v1.21.6`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`lz4`](../lz4/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Run-time dependencies | [`lz4`](../lz4/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_c_blosc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `c-blosc` (programs, shared libraries, run-time data) and `c-blosc-dev` (headers, pkg-config/CMake files, static libraries; needs `c-blosc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`c-blosc`](https://openqnx.github.io/oss/package/x86_64/c-blosc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/c-blosc.html)) · [`c-blosc-dev`](https://openqnx.github.io/oss/package/x86_64/c-blosc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/c-blosc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add c-blosc` (dependencies come along); one version: `apk add c-blosc=<version>` | `apk del c-blosc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install c-blosc` (dependencies first) | `oss-get remove c-blosc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`c-blosc-dev` the same way, by its own name (`apk add c-blosc-dev`, `oss-get install c-blosc-dev`, by hand with its own file and list `c-blosc-dev.list`); remove it before `c-blosc`, which it needs.

```sh
# on the QNX target, as root
apk add c-blosc c-blosc-dev
apk add qnx-test-c-blosc && /usr/local/bin/qnx-port-tests/test_c_blosc    # its on-target test, from tests-8.0-<arch>
apk del c-blosc c-blosc-dev qnx-test-c-blosc
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/c-blosc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/c-blosc.list
rm var/lib/manual/c-blosc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild c-blosc` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild lz4 zlib zstd
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.21.6 --recurse-submodules --shallow-submodules \
    https://github.com/Blosc/c-blosc.git src/c-blosc
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/c-blosc < ports/c-blosc/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — tests: recognize QNX in aligned-alloc feature-test guard

> Imported from qnx-ports/c-blosc (branch qnx-v1.21.6-r0, 616f4b7..b6abcce); original authorship kept.
>
> blosc_test_malloc() picks its aligned-allocator implementation via an
> #if/#elif chain ending in a hard #error if nothing matches. QNX does
> implement posix_memalign (it's POSIX-compliant), the existing
> _POSIX_C_SOURCE/_XOPEN_SOURCE guard just isn't satisfied in this
> translation unit on QNX/clang -- a feature-test-macro gap, not a missing
> platform capability. Only affects the test suite; blosc.c/blosc.h
> (the shipped library) already build clean with zero changes.
> ---
>  tests/test_common.h | 2 +-
>  1 file changed, 1 insertion(+), 1 deletion(-)

Touches: `tests/test_common.h` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/c-blosc -B build/$QNX_ARCH/c-blosc -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=ON \
      -DBUILD_TESTS=OFF \
      -DBUILD_FUZZERS=OFF \
      -DBUILD_BENCHMARKS=OFF \
      -DPREFER_EXTERNAL_LZ4=ON \
      -DPREFER_EXTERNAL_ZLIB=ON \
      -DPREFER_EXTERNAL_ZSTD=ON \
      -DDEACTIVATE_SNAPPY=ON \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/$QNX_ARCH/c-blosc -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/c-blosc
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- c-blosc 1.21.6: the Blosc blocking, shuffling meta-compressor (blosclz, lz4, zlib, zstd codecs behind one API), with qnx-ports' QNX change (patches/0001: its test suite's posix_memalign guard, which tests _POSIX_C_SOURCE, recognises QNX -- hazard 16; only the tests use it).
- The codecs come from this tree's lz4, zlib and zstd rather than the copies bundled in internal-complibs/. Snappy stays off (upstream default: it would need a C++ snappy). SSE2/AVX2 shuffle kernels are chosen at run time on x86_64; aarch64 uses the generic ones.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH c-blosc`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libblosc.so
/usr/local/lib/libblosc.so.1
/usr/local/lib/libblosc.so.1.21.6
```

The complete list is `stage/<arch>/.manifest/c-blosc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa c-blosc
ls dists/8.0/*/ | grep '^c-blosc-'
```

This writes `c-blosc-<ver>-r0.tar.gz` (run time) and `c-blosc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add c-blosc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/c-blosc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/c-blosc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_c_blosc`](../../tests/src/) links c-blosc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-c-blosc`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-c-blosc
/usr/local/bin/qnx-port-tests/test_c_blosc
```

```
== c-blosc ==
  ok   ...
PASS c-blosc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_c_blosc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_c_blosc`](../../tests/src/) -- a program linked against c-blosc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version 1.21.6
   - zstd codec is the Zstd library
   - 4 threads
   - blosc_getitem: items 500000..500002 out of the middle
   - context API (zstd, 2 threads) round trip
   - cbuffer_validate accepts the real buffer
   - cbuffer_validate rejects a truncated buffer

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | c-blosc 1.21.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/c-blosc-1.21.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | c-blosc 1.21.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/c-blosc-1.21.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.21.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add c-blosc=<version>`), sources in `ported/c-blosc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates c-blosc`; port one with `scripts/port-update c-blosc`.

<details><summary>recipe.sh</summary>

```bash
# c-blosc 1.21.6: the Blosc blocking, shuffling meta-compressor (blosclz,
# lz4, zlib, zstd codecs behind one API), with qnx-ports' QNX change
# (patches/0001: its test suite's posix_memalign guard, which tests
# _POSIX_C_SOURCE, recognises QNX -- hazard 16; only the tests use it).
VERSION=1.21.6; GIT=https://github.com/Blosc/c-blosc.git; REF=v1.21.6
DEPS="lz4 zlib zstd"
# The codecs come from this tree's lz4, zlib and zstd rather than the copies
# bundled in internal-complibs/. Snappy stays off (upstream default: it would
# need a C++ snappy). SSE2/AVX2 shuffle kernels are chosen at run time on
# x86_64; aarch64 uses the generic ones.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED=ON -DBUILD_STATIC=ON -DBUILD_TESTS=OFF -DBUILD_FUZZERS=OFF
                   -DBUILD_BENCHMARKS=OFF -DPREFER_EXTERNAL_LZ4=ON -DPREFER_EXTERNAL_ZLIB=ON
                   -DPREFER_EXTERNAL_ZSTD=ON -DDEACTIVATE_SNAPPY=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
