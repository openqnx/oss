# openexr 3.4.1

> High dynamic-range (HDR) image file format library and tools

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/AcademySoftwareFoundation/openexr.git (tag `v3.4.1`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`imath`](../imath/) [`libdeflate`](../libdeflate/) [`openjph`](../openjph/) |
| Run-time dependencies | [`imath`](../imath/) [`libdeflate`](../libdeflate/) [`openjph`](../openjph/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openexr`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `openexr` (programs, shared libraries, run-time data) and `openexr-dev` (headers, pkg-config/CMake files, static libraries; needs `openexr`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`openexr`](https://openqnx.github.io/oss/package/x86_64/openexr.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openexr.html)) · [`openexr-dev`](https://openqnx.github.io/oss/package/x86_64/openexr-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/openexr-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add openexr` (dependencies come along); one version: `apk add openexr=<version>` | `apk del openexr` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install openexr` (dependencies first) | `oss-get remove openexr` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`openexr-dev` the same way, by its own name (`apk add openexr-dev`, `oss-get install openexr-dev`, by hand with its own file and list `openexr-dev.list`); remove it before `openexr`, which it needs.

```sh
# on the QNX target, as root
apk add openexr openexr-dev
apk add qnx-test-openexr && /usr/local/bin/qnx-port-tests/test_openexr    # its on-target test, from tests-8.0-<arch>-m-z
apk del openexr openexr-dev qnx-test-openexr
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/openexr.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/openexr.list
rm var/lib/manual/openexr.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openexr` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (15 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild imath libdeflate openjph
```

<details><summary>Full build chain (15)</summary>

[`gettext`](../gettext/) [`giflib`](../giflib/) [`imath`](../imath/) [`libdeflate`](../libdeflate/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`ncurses`](../ncurses/) [`openjph`](../openjph/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v3.4.1 --recurse-submodules --shallow-submodules \
    https://github.com/AcademySoftwareFoundation/openexr.git src/openexr
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/openexr < ports/openexr/patches/0001-qnx-endian.patch
```

**[`0001-qnx-endian.patch`](patches/0001-qnx-endian.patch)** — OpenEXRCore: byte-order functions from <net/netbyte.h> on QNX

> QNX has no <endian.h> and no glibc __BYTE_ORDER/__LITTLE_ENDIAN, so the
> generic branch fails ("endian.h: No such file or directory"). QNX declares
> htole16() ... le64toh() in <net/netbyte.h> and the BSD BYTE_ORDER /
> LITTLE_ENDIAN in <sys/param.h>: a QNX branch takes them from there.

Touches: `src/lib/OpenEXRCore/internal_xdr.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/openexr -B build/$QNX_ARCH/openexr -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DOPENEXR_FORCE_INTERNAL_IMATH=OFF \
      -DOPENEXR_FORCE_INTERNAL_DEFLATE=OFF \
      -DOPENEXR_FORCE_INTERNAL_OPENJPH=OFF \
      -DOPENEXR_BUILD_TOOLS=ON \
      -DOPENEXR_INSTALL_TOOLS=ON \
      -DOPENEXR_BUILD_EXAMPLES=OFF \
      -DOPENEXR_BUILD_PYTHON=OFF \
      -DOPENEXR_INSTALL_DOCS=OFF \
      -DBUILD_WEBSITE=OFF \
      -DOPENEXR_ENABLE_THREADING=ON
cmake --build build/$QNX_ARCH/openexr -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/openexr
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- openexr: the OpenEXR high-dynamic-range image file format (ILM/ASWF) -- the C core library, the C++ IlmImf/OpenEXR library, IlmThread and Iex, and the exr* tools (exrinfo, exrheader, exrmaketiled, exrenvmap, exrstdattr ...).
- Built against the staged Imath, libdeflate and OpenJPH (HTJ2K compression): without them OpenEXR's configure downloads its own copies. Threading on; no examples, tests, Python bindings, docs or website.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openexr`.

### 5. What it installs

184 files under `/usr/local`: 20 shared-library entries, 11 programs, 148 headers.

```
/usr/local/lib/libIex-3_4.so
/usr/local/lib/libIex-3_4.so.33
/usr/local/lib/libIex-3_4.so.33.3.4.1
/usr/local/lib/libIex.so
/usr/local/lib/libIlmThread-3_4.so
/usr/local/lib/libIlmThread-3_4.so.33
/usr/local/lib/libIlmThread-3_4.so.33.3.4.1
/usr/local/lib/libIlmThread.so
/usr/local/lib/libOpenEXR-3_4.so
/usr/local/lib/libOpenEXR-3_4.so.33
/usr/local/lib/libOpenEXR-3_4.so.33.3.4.1
/usr/local/lib/libOpenEXR.so
/usr/local/lib/libOpenEXRCore-3_4.so
/usr/local/lib/libOpenEXRCore-3_4.so.33
/usr/local/lib/libOpenEXRCore-3_4.so.33.3.4.1
/usr/local/lib/libOpenEXRCore.so
...
```

The complete list is `stage/<arch>/.manifest/openexr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openexr
ls dists/8.0/*/ | grep '^openexr-'
```

This writes `openexr-<ver>-r0.tar.gz` (run time) and `openexr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openexr
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openexr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openexr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openexr`](../../tests/src/) links openexr the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openexr`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openexr
/usr/local/bin/qnx-port-tests/test_openexr
```

```
== openexr ==
  ok   ...
PASS openexr
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openexr` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openexr`](../../tests/src/) -- a program linked against openexr the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

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

This recipe builds **3.4.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openexr=<version>`), sources in `ported/openexr/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openexr`; port one with `scripts/port-update openexr`.

<details><summary>recipe.sh</summary>

```bash
# openexr: the OpenEXR high-dynamic-range image file format (ILM/ASWF) -- the
# C core library, the C++ IlmImf/OpenEXR library, IlmThread and Iex, and the
# exr* tools (exrinfo, exrheader, exrmaketiled, exrenvmap, exrstdattr ...).
VERSION=3.4.1; GIT=https://github.com/AcademySoftwareFoundation/openexr.git; REF=v3.4.1
DEPS="imath libdeflate openjph"
BUILDSYS=cmake
# Built against the staged Imath, libdeflate and OpenJPH (HTJ2K compression):
# without them OpenEXR's configure downloads its own copies. Threading on;
# no examples, tests, Python bindings, docs or website.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DOPENEXR_FORCE_INTERNAL_IMATH=OFF -DOPENEXR_FORCE_INTERNAL_DEFLATE=OFF
                   -DOPENEXR_FORCE_INTERNAL_OPENJPH=OFF
                   -DOPENEXR_BUILD_TOOLS=ON -DOPENEXR_INSTALL_TOOLS=ON
                   -DOPENEXR_BUILD_EXAMPLES=OFF -DOPENEXR_BUILD_PYTHON=OFF
                   -DOPENEXR_INSTALL_DOCS=OFF -DBUILD_WEBSITE=OFF -DOPENEXR_ENABLE_THREADING=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
