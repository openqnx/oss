# nasm 2.16.03

> Netwide Assembler: x86 and x86-64 assembler and disassembler (ndisasm)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/nasm-2.16.03.tar.xz |
| SHA-256 | `1412a1c760bbd05db026b6c0d1657affd6631cd0a63cddb6f73cc6d4aa616148` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_nasm`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `nasm` (programs, shared libraries, run-time data) and `nasm-dev` (headers, pkg-config/CMake files, static libraries; needs `nasm`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`nasm`](https://openqnx.github.io/oss/package/x86_64/nasm.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nasm.html)) · [`nasm-dev`](https://openqnx.github.io/oss/package/x86_64/nasm-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nasm-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add nasm` (dependencies come along); one version: `apk add nasm=<version>` | `apk del nasm` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install nasm` (dependencies first) | `oss-get remove nasm` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`nasm-dev` the same way, by its own name (`apk add nasm-dev`, `oss-get install nasm-dev`, by hand with its own file and list `nasm-dev.list`); remove it before `nasm`, which it needs.

```sh
# on the QNX target, as root
apk add nasm nasm-dev
apk add qnx-test-nasm && /usr/local/bin/qnx-port-tests/test_nasm    # its on-target test, from tests-8.0-<arch>-m-z
apk del nasm nasm-dev qnx-test-nasm
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/nasm.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/nasm.list
rm var/lib/manual/nasm.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nasm` performs exactly the same steps (2–5) in one command.

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
URL=https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/nasm-2.16.03.tar.xz
mkdir -p dist src/nasm
curl -fL -o dist/$(basename $URL) $URL
echo "1412a1c760bbd05db026b6c0d1657affd6631cd0a63cddb6f73cc6d4aa616148  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/nasm --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/nasm; B=build/$QNX_ARCH/nasm; mkdir -p $B
```

```bash
do_configure() {
  ( cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
      CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" CFLAGS="-O2" \
      "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" )
}
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- NASM, the Netwide Assembler, and ndisasm: an x86/x86-64 assembler (Intel syntax; ELF, COFF, Mach-O, binary and other output formats). Version as in QNX's own package list. It runs on both architectures (on aarch64 it is a cross assembler for x86 code).
- Autotools, configured by hand: the default configure step puts the staging include directory first (qnxbuild), where a staged md5.h (libmd's) shadows NASM's own include/md5.h (a different MD5Transform), hazard 22. NASM needs nothing from staging, so its compiler sees only the SDP's headers.
- The manuals are prebuilt in the release tarball (nasm.1, ndisasm.1).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nasm`.

### 5. What it installs

4 files under `/usr/local`, 2 programs.

```
/usr/local/bin/nasm
/usr/local/bin/ndisasm
```

The complete list is `stage/<arch>/.manifest/nasm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nasm
ls dists/8.0/*/ | grep '^nasm-'
```

This writes `nasm-<ver>-r0.tar.gz` (run time) and `nasm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nasm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nasm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nasm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_nasm`](../../tests/src/) links nasm the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-nasm`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-nasm
/usr/local/bin/qnx-port-tests/test_nasm
```

```
== nasm ==
  ok   ...
PASS nasm
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_nasm` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_nasm`](../../tests/src/) -- a program linked against nasm the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> nasm`.

## Versions

This recipe builds **2.16.03**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nasm=<version>`), sources in `ported/nasm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nasm`; port one with `scripts/port-update nasm`.

<details><summary>recipe.sh</summary>

```bash
# NASM, the Netwide Assembler, and ndisasm: an x86/x86-64 assembler (Intel
# syntax; ELF, COFF, Mach-O, binary and other output formats). Version as in
# QNX's own package list. It runs on both architectures (on aarch64 it is
# a cross assembler for x86 code).
VERSION=2.16.03
TARBALL=https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/nasm-2.16.03.tar.xz
SHA256=1412a1c760bbd05db026b6c0d1657affd6631cd0a63cddb6f73cc6d4aa616148
DEPS=""
# Autotools, configured by hand: the default configure step puts the
# staging include directory first (qnxbuild), where a staged md5.h (libmd's) shadows
# NASM's own include/md5.h (a different MD5Transform), hazard 22. NASM needs
# nothing from staging, so its compiler sees only the SDP's headers.
BUILDSYS=custom
do_configure() {
  ( cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
      CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" CFLAGS="-O2" \
      "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" )
}
do_build() { make -C "$B" -j"$(nproc)"; }
# The manuals are prebuilt in the release tarball (nasm.1, ndisasm.1).
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
