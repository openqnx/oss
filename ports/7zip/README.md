# 7zip 25.01

> File archiver with a high compression ratio (7-Zip command line, 7zz)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ip7z/7zip/releases/download/25.01/7z2501-src.tar.xz |
| SHA-256 | `ed087f83ee789c1ea5f39c464c55a5c9d4008deb0efe900814f2df262b82c36e` |
| Build system | custom (see recipe) |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_7zip`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `7zip`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`7zip`](https://openqnx.github.io/oss/package/x86_64/7zip.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/7zip.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add 7zip` (dependencies come along); one version: `apk add 7zip=<version>` | `apk del 7zip` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install 7zip` (dependencies first) | `oss-get remove 7zip` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-7zip && /usr/local/bin/qnx-port-tests/test_7zip    # its on-target test, from tests-8.0-<arch>
apk del 7zip qnx-test-7zip
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/7zip.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/7zip.list
rm var/lib/manual/7zip.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild 7zip` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/ip7z/7zip/releases/download/25.01/7z2501-src.tar.xz
mkdir -p dist src/7zip
curl -fL -o dist/$(basename $URL) $URL
echo "ed087f83ee789c1ea5f39c464c55a5c9d4008deb0efe900814f2df262b82c36e  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/7zip --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/7zip < ports/7zip/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/7zip < ports/7zip/patches/0002-qnx-no-getauxval.patch
patch -p1 -d src/7zip < ports/7zip/patches/0003-qnx-ram-size-from-syspage.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — QNX support from qnx-ports

> Imported from qnx-ports/7zip (branch qnx-25.01, 67c101a..318fd72); original authorship kept.
>
> Commits:
>   dce1857 Patched 25.01
>   318fd72 Merge pull request #2 from taang-qnx/mybranch
> ---
>  C/Threads.c              |  6 +++---
>  C/Threads.h              |  1 +
>  CPP/7zip/7zip_gcc.mak    | 10 +++++-----
>  CPP/Windows/FileFind.cpp |  4 ++--
>  CPP/Windows/FileFind.h   |  6 +++---
>  CPP/Windows/System.cpp   | 10 ++++++++++
>  6 files changed, 24 insertions(+), 13 deletions(-)

Touches: `C/Threads.c`, `C/Threads.h`, `CPP/7zip/7zip_gcc.mak`, `CPP/Windows/FileFind.cpp`, `CPP/Windows/FileFind.h`, `CPP/Windows/System.cpp` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-no-getauxval.patch`](patches/0002-qnx-no-getauxval.patch)** — CpuArch: no getauxval() hardware-capability probe on QNX

> QNX ships <sys/auxv.h>, so the __has_include test below turned on
> USE_HWCAP, but it has neither getauxval() nor AT_HWCAP/HWCAP_*: the arm64
> build failed. On QNX the CPU feature checks return 0 (the hardware CRC32,
> SHA and AES code paths are not used; NEON is always on for arm64).

Touches: `C/CpuArch.c` · Upstream status: Pending

**[`0003-qnx-ram-size-from-syspage.patch`](patches/0003-qnx-ram-size-from-syspage.patch)** — System: the RAM size on QNX from the system page

> qnx-ports' GetRamSize() for QNX asks sysconf(_SC_PHYS_PAGES), which QNX 8
> does not implement (-1), and then reported a RAM size of 0 -- instead of
> the 4 GB the code otherwise assumes -- to everything that sizes threads,
> dictionaries and buffers by the available memory. Add up the "ram"
> entries of the system page's address-space table (what pidin info
> reports) when sysconf() has no answer.

Touches: `CPP/Windows/System.cpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/7zip; B=build/$QNX_ARCH/7zip; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/"; }
```

```bash
do_build() { _7m -j"$(nproc)"; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/CPP/7zip/Bundles/Alone2/b/g/7zz" "$STAGE$PREFIX/bin/7zz"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- 7-Zip 25.01: the 7zz command-line archiver (7z, xz, zip, tar, gzip, bzip2 ... create, list, test, extract; also reads many more formats), with qnx-ports' QNX support (patches/0001): no d_type in QNX's struct dirent (directory entries are typed through stat, as on AIX and Solaris), the RAM size from sysconf(_SC_PHYS_PAGES), no pthread affinity calls, and makefile changes so the cross build passes its own CFLAGS and is not stripped or built with -Werror. patches/0002: no getauxval() hardware probe on QNX (arm64); 0003: the RAM size from the system page, since QNX does not implement _SC_PHYS_PAGES (0001 reported 0 bytes).
- The source archive has no top-level directory, so the default fetch (--strip-components=1) would drop one level: unpack it as is.
- Bundles/Alone2 is 7zz, the standalone 7-Zip with every codec built in (no 7z.so plugin). cmpl_gcc.mak: the portable C/C++ code, no assembler (the x86 .asm files need asmc/uasm). The patched makefile appends to CFLAGS but sets CXXFLAGS itself, so the optimisation goes in MY_ARCH, which both use. "-z noexecstack" goes through -Wl: qcc does not forward a bare -z. LIB2 (-lpthread -ldl) is empty: both are part of QNX's libc.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH 7zip`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/7zz
```

The complete list is `stage/<arch>/.manifest/7zip` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa 7zip
ls dists/8.0/*/ | grep '^7zip-'
```

This writes `7zip-<ver>-r0.tar.gz` (run time) and `7zip-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add 7zip
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/7zip-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/7zip-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_7zip`](../../tests/src/) links 7zip the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-7zip`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-7zip
/usr/local/bin/qnx-port-tests/test_7zip
```

```
== 7zip ==
  ok   ...
PASS 7zip
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_7zip` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_7zip`](../../tests/src/) -- a program linked against 7zip the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | 7zip 25.01-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/7zip-25.01-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | 7zip 25.01-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/7zip-25.01-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **25.01**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add 7zip=<version>`), sources in `ported/7zip/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates 7zip`; port one with `scripts/port-update 7zip`.

<details><summary>recipe.sh</summary>

```bash
# 7-Zip 25.01: the 7zz command-line archiver (7z, xz, zip, tar, gzip,
# bzip2 ... create, list, test, extract; also reads many more formats), with
# qnx-ports' QNX support (patches/0001): no d_type in QNX's struct dirent
# (directory entries are typed through stat, as on AIX and Solaris), the
# RAM size from sysconf(_SC_PHYS_PAGES), no pthread affinity calls, and
# makefile changes so the cross build passes its own CFLAGS and is not
# stripped or built with -Werror. patches/0002: no getauxval() hardware
# probe on QNX (arm64); 0003: the RAM size from the system page, since QNX
# does not implement _SC_PHYS_PAGES (0001 reported 0 bytes).
VERSION=25.01
TARBALL=https://github.com/ip7z/7zip/releases/download/25.01/7z2501-src.tar.xz
SHA256=ed087f83ee789c1ea5f39c464c55a5c9d4008deb0efe900814f2df262b82c36e
BUILDSYS=custom
# The source archive has no top-level directory, so the default fetch
# (--strip-components=1) would drop one level: unpack it as is.
do_fetch() {
  local tb="$WS/dist/7z2501-src.tar.xz"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL -o "$tb" "$TARBALL" || return 1
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "7zip checksum mismatch"; return 1; }
  tar xJf "$tb" -C "$SRCROOT"
}
# Bundles/Alone2 is 7zz, the standalone 7-Zip with every codec built in (no
# 7z.so plugin). cmpl_gcc.mak: the portable C/C++ code, no assembler (the
# x86 .asm files need asmc/uasm). The patched makefile appends to CFLAGS but
# sets CXXFLAGS itself, so the optimisation goes in MY_ARCH, which both use.
# "-z noexecstack" goes through -Wl: qcc does not forward a bare -z.
# LIB2 (-lpthread -ldl) is empty: both are part of QNX's libc.
_7m() { CFLAGS="-D_QNX_SOURCE" make -C "$B/CPP/7zip/Bundles/Alone2" -f ../../cmpl_gcc.mak \
  CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" MY_ARCH="-O2 -D_QNX_SOURCE" \
  LFLAGS_NOEXECSTACK="-Wl,-z,noexecstack" LIB2= "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _7m -j"$(nproc)"; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/CPP/7zip/Bundles/Alone2/b/g/7zz" "$STAGE$PREFIX/bin/7zz"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
