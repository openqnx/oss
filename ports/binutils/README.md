# binutils 2.43

> GNU binary tools: assembler, linker, objdump, readelf and others

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.xz |
| SHA-256 | `b53606f443ac8f01d1d5fc9c39497f2af322d99e14cea5c0b4b124d630379365` |
| Build system | GNU Autotools |
| QNX patches | 10 |
| Build dependencies | [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Run-time dependencies | [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_binutils`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `binutils` (programs, shared libraries, run-time data) and `binutils-dev` (headers, pkg-config/CMake files, static libraries; needs `binutils`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`binutils`](https://openqnx.github.io/oss/package/x86_64/binutils.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/binutils.html)) · [`binutils-dev`](https://openqnx.github.io/oss/package/x86_64/binutils-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/binutils-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add binutils` (dependencies come along); one version: `apk add binutils=<version>` | `apk del binutils` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install binutils` (dependencies first) | `oss-get remove binutils` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`binutils-dev` the same way, by its own name (`apk add binutils-dev`, `oss-get install binutils-dev`, by hand with its own file and list `binutils-dev.list`); remove it before `binutils`, which it needs.

```sh
# on the QNX target, as root
apk add binutils binutils-dev
apk add qnx-test-binutils && /usr/local/bin/qnx-port-tests/test_binutils    # its on-target test, from tests-8.0-<arch>
apk del binutils binutils-dev qnx-test-binutils
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/binutils.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/binutils.list
rm var/lib/manual/binutils.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild binutils` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib zstd
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.xz
mkdir -p dist src/binutils
curl -fL -o dist/$(basename $URL) $URL
echo "b53606f443ac8f01d1d5fc9c39497f2af322d99e14cea5c0b4b124d630379365  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/binutils --strip-components=1
```

### 3. Apply the QNX patches

10 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/binutils < ports/binutils/patches/0001-aports-0001.patch
patch -p1 -d src/binutils < ports/binutils/patches/0002-aports-0002.patch
patch -p1 -d src/binutils < ports/binutils/patches/0003-aports-0003.patch
patch -p1 -d src/binutils < ports/binutils/patches/0004-aports-0004.patch
patch -p1 -d src/binutils < ports/binutils/patches/0005-aports-0005.patch
patch -p1 -d src/binutils < ports/binutils/patches/0006-aports-0006.patch
patch -p1 -d src/binutils < ports/binutils/patches/0007-aports-0007.patch
patch -p1 -d src/binutils < ports/binutils/patches/0008-aports-0008.patch
patch -p1 -d src/binutils < ports/binutils/patches/0009-aports-0009.patch
patch -p1 -d src/binutils < ports/binutils/patches/0010-aports-0010.patch
```

**[`0001-aports-0001.patch`](patches/0001-aports-0001.patch)** — QNX change from QNX's aports (core/binutils/0001.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> bfd: adding qnx bfd patches for binutils
>
> adding proper target and bfd changes for qnx as-is from internal
> svn.
>
> With edits to add qnx target support
>
> CLT-9242
> ---
>  bfd/Makefile.am     |  2 +-
>  bfd/Makefile.in     |  2 +-
>  bfd/coffcode.h      |  4 ++++
>  bfd/compress.c      |  4 ++++
>  bfd/config.bfd      | 29 ++++++++++++++++-------------
>  bfd/development.sh  |  2 +-
>  bfd/elf.c           | 20 +++++++++++++++++++-
>  bfd/elf32-arm.c     |  9 +++++++--
>  bfd/elfnn-aarch64.c |  6 ++++++
>  bfd/elfxx-x86.h     |  8 ++++++++
>  10 files changed, 67 insertions(+), 19 deletions(-)

Touches: `bfd/Makefile.am`, `bfd/Makefile.in`, `bfd/coffcode.h`, `bfd/config.bfd`, `bfd/development.sh`, `bfd/elf.c` … · Upstream status: QNX-specific

**[`0002-aports-0002.patch`](patches/0002-aports-0002.patch)** — QNX change from QNX's aports (core/binutils/0002.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> binutils: adding qnx patches
>
> added qnx patches from internal svn to git as-is.
>
> CLT-9242
> ---

Touches: `binutils/testsuite/binutils-all/aarch64/pei-aarch64-little.d`, `binutils/testsuite/binutils-all/elfedit-2.d`, `binutils/testsuite/binutils-all/i386/i386.exp`, `binutils/testsuite/binutils-all/strip-3.d`, `binutils/testsuite/lib/binutils-common.exp` · Upstream status: QNX-specific

**[`0003-aports-0003.patch`](patches/0003-aports-0003.patch)** — QNX change from QNX's aports (core/binutils/0003.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
> Changed here: the hunk adding gas/itbl-lex.c is dropped -- it is a generated
> file the release tarball already contains (QNX's git tree did not).
>
>
> gas: add qnx patches for assembler
>
> adding qnx patches for gas as-is from internal svn.
>
> CLT-9242
> ---

Touches: `gas/configure.tgt`, `gas/testsuite/gas/arm/mapshort-elf.d` · Upstream status: QNX-specific

**[`0004-aports-0004.patch`](patches/0004-aports-0004.patch)** — QNX change from QNX's aports (core/binutils/0004.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> ld: adding qnx patches
>
> adding qnx patches to ld code as-is.
>
> CLT-9242
> ---

Touches: `ld/Makefile.am`, `ld/Makefile.in`, `ld/configure.tgt`, `ld/emulparams/aarch64nto.sh`, `ld/emulparams/armnto.sh`, `ld/emulparams/elf32lppcnto.sh` … · Upstream status: QNX-specific

**[`0005-aports-0005.patch`](patches/0005-aports-0005.patch)** — QNX change from QNX's aports (core/binutils/0005.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> ld: adding qnx testsuite
>
> adding testsuite changes for qnx as-is.
>
> CLT-9242
> ---

Touches: `ld/testsuite/ld-arm/arm-app-abs32.s`, `ld/testsuite/ld-arm/arm-app.s`, `ld/testsuite/ld-arm/arm-static-app.s`, `ld/testsuite/ld-arm/cortex-a8-far-1.s`, `ld/testsuite/ld-arm/cortex-a8-far-2.s`, `ld/testsuite/ld-arm/cortex-a8-fix-blx.s` … · Upstream status: QNX-specific

**[`0006-aports-0006.patch`](patches/0006-aports-0006.patch)** — QNX change from QNX's aports (core/binutils/0006.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> libiberty: adding qnx patches
>
> adding qnx patches to libiberty
>
> CLT-9242
> ---

Touches: `libiberty/pex-unix.c` · Upstream status: QNX-specific

**[`0007-aports-0007.patch`](patches/0007-aports-0007.patch)** — QNX change from QNX's aports (core/binutils/0007.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> include: update elf and gdb headers for qnx
>
> adding qnx patches to elf and gdb headers
>
> CLT-9242
> ---

Touches: `include/elf/common.h`, `include/gdb/signals.def` · Upstream status: QNX-specific

**[`0008-aports-0008.patch`](patches/0008-aports-0008.patch)** — QNX change from QNX's aports (core/binutils/0008.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> qnx: updating config files
>
> updating config files to support qnx properly
>
> CLT-9242
> ---

Touches: `config.sub` · Upstream status: QNX-specific

**[`0009-aports-0009.patch`](patches/0009-aports-0009.patch)** — QNX change from QNX's aports (core/binutils/0009.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> qnx: re-sync with svn
>
> re-syncing the qnx/main branch to the latest in svn.
>
> CLT-9242
> ---

Touches: `ld/emultempl/nto.em` · Upstream status: QNX-specific

**[`0010-aports-0010.patch`](patches/0010-aports-0010.patch)** — QNX change from QNX's aports (core/binutils/0010.patch).

> Taken from qnx-ports/aports core/binutils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> bdf-ld : custom linker script null derefence
>
> check the bed before using it.
>
> CLT-9353
> ---

Touches: `bfd/elflink.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/binutils && cd build/$QNX_ARCH/binutils
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/binutils/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls \
    --disable-werror \
    --enable-shared=no \
    --with-system-zlib \
    --disable-gprofng
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU binutils for the target (as, ld, objdump, readelf, ...). Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): zstd

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH binutils`.

### 5. What it installs

287 files under `/usr/local`, 16 programs, 11 headers.

```
/usr/local/bin/addr2line
/usr/local/bin/ar
/usr/local/bin/as
/usr/local/bin/c++filt
/usr/local/bin/elfedit
/usr/local/bin/gprof
/usr/local/bin/ld
/usr/local/bin/ld.bfd
/usr/local/bin/nm
/usr/local/bin/objcopy
/usr/local/bin/objdump
/usr/local/bin/ranlib
/usr/local/bin/readelf
/usr/local/bin/size
/usr/local/bin/strings
/usr/local/bin/strip
```

The complete list is `stage/<arch>/.manifest/binutils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa binutils
ls dists/8.0/*/ | grep '^binutils-'
```

This writes `binutils-<ver>-r0.tar.gz` (run time) and `binutils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add binutils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/binutils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/binutils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_binutils`](../../tests/src/) links binutils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-binutils`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-binutils
/usr/local/bin/qnx-port-tests/test_binutils
```

```
== binutils ==
  ok   ...
PASS binutils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_binutils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_binutils`](../../tests/src/) -- a program linked against binutils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | binutils 2.43-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/binutils-2.43-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | binutils 2.43-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/binutils-2.43-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | binutils 2.43-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/binutils-2.43-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | binutils 2.43-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/binutils-2.43-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.43**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add binutils=<version>`), sources in `ported/binutils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates binutils`; port one with `scripts/port-update binutils`.

<details><summary>recipe.sh</summary>

```bash
# GNU binutils for the target (as, ld, objdump, readelf, ...). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.43
TARBALL=https://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.xz
SHA256=b53606f443ac8f01d1d5fc9c39497f2af322d99e14cea5c0b4b124d630379365
# Also linked at run time (found by scripts/gen-dependency-index): zstd
DEPS="zlib zstd"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --disable-werror --enable-shared=no --with-system-zlib --disable-gprofng )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
