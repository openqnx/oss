# zip 3.0

> Info-ZIP zip: creates and updates .zip archives (zip, zipnote, zipsplit, zipcloak)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/infozip/zip30.tar.gz |
| SHA-256 | `f0e8bb1f9b7eb0b01285495a2699df3a4b766784c1765a8f1aeedf63c0806369` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zip`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `zip` (programs, shared libraries, run-time data) and `zip-dev` (headers, pkg-config/CMake files, static libraries; needs `zip`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zip`](https://openqnx.github.io/oss/package/x86_64/zip.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zip.html)) · [`zip-dev`](https://openqnx.github.io/oss/package/x86_64/zip-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zip-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zip` (dependencies come along); one version: `apk add zip=<version>` | `apk del zip` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zip` (dependencies first) | `oss-get remove zip` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`zip-dev` the same way, by its own name (`apk add zip-dev`, `oss-get install zip-dev`, by hand with its own file and list `zip-dev.list`); remove it before `zip`, which it needs.

```sh
# on the QNX target, as root
apk add zip zip-dev
apk add qnx-test-zip && /usr/local/bin/qnx-port-tests/test_zip    # its on-target test, from tests-8.0-<arch>-m-z
apk del zip zip-dev qnx-test-zip
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zip.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zip.list
rm var/lib/manual/zip.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zip` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/infozip/zip30.tar.gz
mkdir -p dist src/zip
curl -fL -o dist/$(basename $URL) $URL
echo "f0e8bb1f9b7eb0b01285495a2699df3a4b766784c1765a8f1aeedf63c0806369  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/zip --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/zip; B=build/$QNX_ARCH/zip; mkdir -p $B
```

```bash
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" -f unix/Makefile generic CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    IZ_BZIP2=/nonexistent LOCAL_ZIP="-O2 -DLARGE_FILE_SUPPORT"
}
```

```bash
do_install() {
  make -C "$B" -f unix/Makefile install prefix="$STAGE$PREFIX" MANDIR="$STAGE$PREFIX/share/man/man1"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Info-ZIP Zip 3.0: creates and updates .zip archives (zip, zipnote, zipsplit, zipcloak). Version as in QNX's own package list (the last Info-ZIP release). unzip is a separate project.
- unix/Makefile "generic": unix/configure probes the compiler (compile and link tests only) and writes the flags; it builds in the source tree, so in a copy (src/ stays clean for scripts/mkpatch). No bzip2 support (zip's bzip2 method is rarely used, and it would look for a bzip2 source tree). Two of configure's probes run a test program, which a cross build cannot: LARGE_FILE_SUPPORT (the answer on QNX 8: 64-bit off_t and st_size, so files over 2 GiB and Zip64 archives) is given here; the 16-bit UID/GID probe already falls back to the right answer (QNX's are 32-bit).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zip`.

### 5. What it installs

8 files under `/usr/local`, 4 programs.

```
/usr/local/bin/zip
/usr/local/bin/zipcloak
/usr/local/bin/zipnote
/usr/local/bin/zipsplit
```

The complete list is `stage/<arch>/.manifest/zip` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zip
ls dists/8.0/*/ | grep '^zip-'
```

This writes `zip-<ver>-r0.tar.gz` (run time) and `zip-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zip
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zip-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zip-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zip`](../../tests/src/) links zip the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zip`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zip
/usr/local/bin/qnx-port-tests/test_zip
```

```
== zip ==
  ok   ...
PASS zip
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zip` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zip`](../../tests/src/) -- a program linked against zip the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> zip`.

## Versions

This recipe builds **3.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zip=<version>`), sources in `ported/zip/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zip`; port one with `scripts/port-update zip`.

<details><summary>recipe.sh</summary>

```bash
# Info-ZIP Zip 3.0: creates and updates .zip archives (zip, zipnote,
# zipsplit, zipcloak). Version as in QNX's own package list (the last
# Info-ZIP release). unzip is a separate project.
VERSION=3.0
TARBALL=https://downloads.sourceforge.net/infozip/zip30.tar.gz
SHA256=f0e8bb1f9b7eb0b01285495a2699df3a4b766784c1765a8f1aeedf63c0806369
DEPS=""
# unix/Makefile "generic": unix/configure probes the compiler (compile and
# link tests only) and writes the flags; it builds in the source tree, so
# in a copy (src/ stays clean for scripts/mkpatch). No bzip2 support (zip's
# bzip2 method is rarely used, and it would look for a bzip2 source tree).
# Two of configure's probes run a test program, which a cross build cannot:
# LARGE_FILE_SUPPORT (the answer on QNX 8: 64-bit off_t and st_size, so
# files over 2 GiB and Zip64 archives) is given here; the 16-bit UID/GID
# probe already falls back to the right answer (QNX's are 32-bit).
BUILDSYS=custom
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" -f unix/Makefile generic CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    IZ_BZIP2=/nonexistent LOCAL_ZIP="-O2 -DLARGE_FILE_SUPPORT"
}
do_install() {
  make -C "$B" -f unix/Makefile install prefix="$STAGE$PREFIX" MANDIR="$STAGE$PREFIX/share/man/man1"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
