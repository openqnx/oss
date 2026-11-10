# chrpath 0.18

> Lists, changes or deletes the RPATH/RUNPATH of ELF binaries

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://codeberg.org/pere/chrpath/archive/release-0.18.tar.gz |
| SHA-256 | `83441d1347a09a249c9f7efeac319454e17334d72e2983f18c812c7032cfef54` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_chrpath`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `chrpath` (programs, shared libraries, run-time data) and `chrpath-dev` (headers, pkg-config/CMake files, static libraries; needs `chrpath`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`chrpath`](https://openqnx.github.io/oss/package/x86_64/chrpath.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/chrpath.html)) · [`chrpath-dev`](https://openqnx.github.io/oss/package/x86_64/chrpath-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/chrpath-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add chrpath` (dependencies come along); one version: `apk add chrpath=<version>` | `apk del chrpath` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install chrpath` (dependencies first) | `oss-get remove chrpath` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`chrpath-dev` the same way, by its own name (`apk add chrpath-dev`, `oss-get install chrpath-dev`, by hand with its own file and list `chrpath-dev.list`); remove it before `chrpath`, which it needs.

```sh
# on the QNX target, as root
apk add chrpath chrpath-dev
apk add qnx-test-chrpath && /usr/local/bin/qnx-port-tests/test_chrpath    # its on-target test, from tests-8.0-<arch>
apk del chrpath chrpath-dev qnx-test-chrpath
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/chrpath.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/chrpath.list
rm var/lib/manual/chrpath.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild chrpath` performs exactly the same steps (2–5) in one command.

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
URL=https://codeberg.org/pere/chrpath/archive/release-0.18.tar.gz
mkdir -p dist src/chrpath
curl -fL -o dist/$(basename $URL) $URL
echo "83441d1347a09a249c9f7efeac319454e17334d72e2983f18c812c7032cfef54  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/chrpath --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/chrpath < ports/chrpath/patches/0001-qnx-sys-elf-h.patch
patch -p1 -d src/chrpath < ports/chrpath/patches/0002-qnx-bswap.patch
patch -p1 -d src/chrpath < ports/chrpath/patches/0003-qnx-dt-runpath-enum.patch
```

**[`0001-qnx-sys-elf-h.patch`](patches/0001-qnx-sys-elf-h.patch)** — Include <sys/elf.h> on QNX, which has no <elf.h>.

> QNX has no <elf.h>: its ELF definitions (Elf32_Ehdr, Elf64_Dyn, DT_RPATH,
> DT_RUNPATH, ELFMAG, ...) are in <sys/elf.h>, which includes
> <elfdefinitions.h>. They are the standard names chrpath uses, so only the
> include changes.

Touches: `chrpath.c`, `elf.c`, `killrpath.c`, `protos.h` · Upstream status: QNX-specific

**[`0002-qnx-bswap.patch`](patches/0002-qnx-bswap.patch)** — Byte-swap with GCC builtins on QNX, which has no <byteswap.h>.

> chrpath byte-swaps ELF fields when the file's byte order differs from the
> host's, with bswap_16/32/64 from glibc's <byteswap.h> or Solaris'
> <sys/byteorder.h>. QNX has neither (its <gulliver.h> has ENDIAN_* macros);
> without a declaration the calls are implicit and the link fails. GCC's
> __builtin_bswap* are the same operations.

Touches: `protos.h` · Upstream status: QNX-specific

**[`0003-qnx-dt-runpath-enum.patch`](patches/0003-qnx-dt-runpath-enum.patch)** — Let chrpath see QNX's DT_RUNPATH, an enum constant, not a macro.

> chrpath compiles its DT_RUNPATH support -- listing, replacing and deleting a
> RUNPATH, and -c (convert RPATH to RUNPATH) -- only "#if defined(DT_RUNPATH)".
> QNX's <elfdefinitions.h> declares the DT_* tags as enum constants, not as
> macros, so the test is false although QNX has DT_RUNPATH (29), and chrpath
> then ignores every RUNPATH: "no rpath or runpath tag found" on a program
> that has one, and no -c. A macro that names the enum constant makes the
> test true and changes nothing else.

Touches: `protos.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/chrpath; B=build/$QNX_ARCH/chrpath; mkdir -p $B
```

```bash
do_build() { make -C "$B" -j"$(nproc)" chrpath; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" docdir="$PREFIX/share/doc/chrpath" install-binPROGRAMS install-man install-docDATA; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- chrpath: lists, changes or deletes the RPATH/RUNPATH of ELF executables and shared libraries (32/64-bit, either byte order). Version as in QNX's own package list.
- Only the program, its manual and documents: "make all" also builds the testsuite programs, linked with the "-R <dir>" rpath option configure picked for the build machine (qcc has no -R), and a Debian package directory.
- Makefile.am puts the documents in $prefix/doc/chrpath-0.18: share/doc.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH chrpath`.

### 5. What it installs

8 files under `/usr/local`, 1 programs.

```
/usr/local/bin/chrpath
```

The complete list is `stage/<arch>/.manifest/chrpath` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa chrpath
ls dists/8.0/*/ | grep '^chrpath-'
```

This writes `chrpath-<ver>-r0.tar.gz` (run time) and `chrpath-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add chrpath
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/chrpath-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/chrpath-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_chrpath`](../../tests/src/) links chrpath the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-chrpath`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-chrpath
/usr/local/bin/qnx-port-tests/test_chrpath
```

```
== chrpath ==
  ok   ...
PASS chrpath
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_chrpath` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_chrpath`](../../tests/src/) -- a program linked against chrpath the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | chrpath 0.18-r1 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/aarch64le/chrpath-0.18-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | chrpath 0.18-r1 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/chrpath-0.18-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.18**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add chrpath=<version>`), sources in `ported/chrpath/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates chrpath`; port one with `scripts/port-update chrpath`.

<details><summary>recipe.sh</summary>

```bash
# chrpath: lists, changes or deletes the RPATH/RUNPATH of ELF executables
# and shared libraries (32/64-bit, either byte order). Version as in QNX's
# own package list.
VERSION=0.18
TARBALL=https://codeberg.org/pere/chrpath/archive/release-0.18.tar.gz
SHA256=83441d1347a09a249c9f7efeac319454e17334d72e2983f18c812c7032cfef54
DEPS=""
BUILDSYS=autotools
# Only the program, its manual and documents: "make all" also builds the
# testsuite programs, linked with the "-R <dir>" rpath option configure
# picked for the build machine (qcc has no -R), and a Debian package
# directory.
do_build() { make -C "$B" -j"$(nproc)" chrpath; }
# Makefile.am puts the documents in $prefix/doc/chrpath-0.18: share/doc.
do_install() { make -C "$B" DESTDIR="$STAGE" docdir="$PREFIX/share/doc/chrpath" install-binPROGRAMS install-man install-docDATA; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
