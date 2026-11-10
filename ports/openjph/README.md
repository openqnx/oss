# openjph 0.24.1

> HTJ2K (High-Throughput JPEG 2000) encoder and decoder library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/aous72/OpenJPH.git (tag `0.24.1`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libtiff`](../libtiff/) |
| Run-time dependencies | [`libtiff`](../libtiff/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openexr`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `openjph` (programs, shared libraries, run-time data) and `openjph-dev` (headers, pkg-config/CMake files, static libraries; needs `openjph`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`openjph`](https://openqnx.github.io/oss/package/x86_64/openjph.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openjph.html)) · [`openjph-dev`](https://openqnx.github.io/oss/package/x86_64/openjph-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openjph-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add openjph` (dependencies come along); one version: `apk add openjph=<version>` | `apk del openjph` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install openjph` (dependencies first) | `oss-get remove openjph` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`openjph-dev` the same way, by its own name (`apk add openjph-dev`, `oss-get install openjph-dev`, by hand with its own file and list `openjph-dev.list`); remove it before `openjph`, which it needs.

```sh
# on the QNX target, as root
apk add openjph openjph-dev
apk add qnx-test-openexr && /usr/local/bin/qnx-port-tests/test_openexr    # its on-target test, from tests-8.0-<arch>-m-z
apk del openjph openjph-dev qnx-test-openexr
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/openjph.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/openjph.list
rm var/lib/manual/openjph.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openjph` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (12 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libtiff
```

<details><summary>Full build chain (12)</summary>

[`gettext`](../gettext/) [`giflib`](../giflib/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.24.1 --recurse-submodules --shallow-submodules \
    https://github.com/aous72/OpenJPH.git src/openjph
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/openjph -B build/$QNX_ARCH/openjph -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DCMAKE_CXX_STANDARD=17 \
      -DBUILD_SHARED_LIBS=ON \
      -DOJPH_BUILD_TESTS=OFF \
      -DOJPH_BUILD_EXECUTABLES=ON \
      -DOJPH_ENABLE_TIFF_SUPPORT=ON
cmake --build build/$QNX_ARCH/openjph -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/openjph
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- openjph: HTJ2K (High-Throughput JPEG 2000, ITU-T T.814) encoder and decoder library, with ojph_compress/ojph_expand. Added as a dependency of OpenEXR 3.4, whose HTJ2K compression is built on it (otherwise OpenEXR downloads a copy at configure time). 0.24.1 is the release OpenEXR 3.4.1 pins.
- Shared library and the two tools, with TIFF input/output (libtiff is here). The SIMD paths are chosen at run time from the CPU's features. C++17: QNX declares aligned_alloc() only for C11/C++17 and later (the project defaults to C++14, where it is "not declared in this scope").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openjph`.

### 5. What it installs

20 files under `/usr/local`: 3 shared-library entries, 2 programs, 10 headers.

```
/usr/local/lib/libopenjph.so
/usr/local/lib/libopenjph.so.0.24
/usr/local/lib/libopenjph.so.0.24.1
/usr/local/bin/ojph_compress
/usr/local/bin/ojph_expand
```

The complete list is `stage/<arch>/.manifest/openjph` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openjph
ls dists/8.0/*/ | grep '^openjph-'
```

This writes `openjph-<ver>-r0.tar.gz` (run time) and `openjph-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openjph
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openjph-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openjph-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openexr`](../../tests/src/) links openjph the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openexr`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openexr
/usr/local/bin/qnx-port-tests/test_openexr
```

```
== openjph ==
  ok   ...
PASS openjph
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openexr` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openexr`](../../tests/src/) -- a program linked against openjph the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temp dir
   - lossy B44 round trip within tolerance
   - tiled file has 4x2 tiles of 16x16
   - tiled pixels read back
   - tiled file
   - exr_start_read opens the file (C core)
   - core: one part, 64x32, ZIP
   - core: channels A,B,G,R

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openexr 3.4.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/openexr-3.4.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | openjph 0.24.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/openjph-0.24.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openexr 3.4.1-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/openexr-3.4.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openjph 0.24.1-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/openjph-0.24.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.24.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openjph=<version>`), sources in `ported/openjph/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openjph`; port one with `scripts/port-update openjph`.

<details><summary>recipe.sh</summary>

```bash
# openjph: HTJ2K (High-Throughput JPEG 2000, ITU-T T.814) encoder and decoder
# library, with ojph_compress/ojph_expand. Added as a dependency of OpenEXR 3.4,
# whose HTJ2K compression is built on it (otherwise OpenEXR downloads a copy at
# configure time). 0.24.1 is the release OpenEXR 3.4.1 pins.
VERSION=0.24.1; GIT=https://github.com/aous72/OpenJPH.git; REF=0.24.1
DEPS="libtiff"
BUILDSYS=cmake
# Shared library and the two tools, with TIFF input/output (libtiff is here).
# The SIMD paths are chosen at run time from the CPU's features.
# C++17: QNX declares aligned_alloc() only for C11/C++17 and later (the
# project defaults to C++14, where it is "not declared in this scope").
CMAKE_ARGS_EXTRA=( -DCMAKE_CXX_STANDARD=17 -DBUILD_SHARED_LIBS=ON -DOJPH_BUILD_TESTS=OFF -DOJPH_BUILD_EXECUTABLES=ON
                   -DOJPH_ENABLE_TIFF_SUPPORT=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
