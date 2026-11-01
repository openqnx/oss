# elfutils 0.193

> Library to read and write ELF files (libelf from elfutils)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://sourceware.org/elfutils/ftp/0.193/elfutils-0.193.tar.bz2 |
| SHA-256 | `7857f44b624f4d8d421df851aaae7b1402cfe6bcdd2d8049f15fc07d3dde7635` |
| Build system | GNU Autotools |
| QNX patches | 4 |
| Build dependencies | [`zlib`](../zlib/) |
| Run-time dependencies | [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_elfutils`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `elfutils` (programs, shared libraries, run-time data) and `elfutils-dev` (headers, pkg-config/CMake files, static libraries; needs `elfutils`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`elfutils`](https://openqnx.github.io/oss/package/x86_64/elfutils.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/elfutils.html)) · [`elfutils-dev`](https://openqnx.github.io/oss/package/x86_64/elfutils-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/elfutils-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add elfutils` (dependencies come along); one version: `apk add elfutils=<version>` | `apk del elfutils` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install elfutils` (dependencies first) | `oss-get remove elfutils` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`elfutils-dev` the same way, by its own name (`apk add elfutils-dev`, `oss-get install elfutils-dev`, by hand with its own file and list `elfutils-dev.list`); remove it before `elfutils`, which it needs.

```sh
# on the QNX target, as root
apk add elfutils elfutils-dev
apk add qnx-test-elfutils && /usr/local/bin/qnx-port-tests/test_elfutils    # its on-target test, from tests-8.0-<arch>
apk del elfutils elfutils-dev qnx-test-elfutils
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/elfutils.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/elfutils.list
rm var/lib/manual/elfutils.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild elfutils` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib
```

### 2. Get the upstream source

```bash
URL=https://sourceware.org/elfutils/ftp/0.193/elfutils-0.193.tar.bz2
mkdir -p dist src/elfutils
curl -fL -o dist/$(basename $URL) $URL
echo "7857f44b624f4d8d421df851aaae7b1402cfe6bcdd2d8049f15fc07d3dde7635  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/elfutils --strip-components=1
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/elfutils < ports/elfutils/patches/0001-qnx-byteswap-endian.patch
patch -p1 -d src/elfutils < ports/elfutils/patches/0002-qnx-no-nls-no-dgettext.patch
patch -p1 -d src/elfutils < ports/elfutils/patches/0003-qnx-argp-helpers-optional.patch
patch -p1 -d src/elfutils < ports/elfutils/patches/0004-qnx-tdestroy.patch
```

**[`0001-qnx-byteswap-endian.patch`](patches/0001-qnx-byteswap-endian.patch)** — Take byte order and byte swaps from QNX's own headers.

> QNX has neither <byteswap.h> nor <endian.h>. The byte order macros
> (BYTE_ORDER, LITTLE_ENDIAN, BIG_ENDIAN) are in <sys/param.h> and the
> htobe64()/be64toh() family in <net/netbyte.h> (hazard 13); bswap_16/32/64
> are GCC builtins.

Touches: `lib/system.h` · Upstream status: QNX-specific

**[`0002-qnx-no-nls-no-dgettext.patch`](patches/0002-qnx-no-nls-no-dgettext.patch)** — Do not call dgettext when NLS is disabled on QNX.

> QNX's libc has no gettext, and libelf is built without NLS there. eu-config.h
> still routed every message through dgettext(); with the gettext port's
> <libintl.h> on the include path that became an undefined libintl_dgettext.
> Without NLS the messages are simply the untranslated strings.

Touches: `lib/eu-config.h` · Upstream status: QNX-specific

**[`0003-qnx-argp-helpers-optional.patch`](patches/0003-qnx-argp-helpers-optional.patch)** — Compile the argp helpers only where <argp.h> exists (QNX).

> color.c and printversion.c are option-parsing helpers of the eu-* tools,
> built on glibc's argp, which QNX does not have. They are compiled into
> libeu.a, whose objects libelf also takes, though libelf uses neither: on
> QNX without <argp.h> they compile to nothing (the tools are not built).

Touches: `lib/color.c`, `lib/printversion.c` · Upstream status: QNX-specific

**[`0004-qnx-tdestroy.patch`](patches/0004-qnx-tdestroy.patch)** — Free a search tree without tdestroy() on QNX.

> eu_tdestroy() frees a search tree with tdestroy(), a GNU extension QNX's
> libc does not have (libelf's elf_end() uses it for its raw-chunk tree).
> The QNX branch empties the tree with standard tdelete(): a comparison that
> always matches makes tdelete() remove the root node, and the key is the
> first member of a node (POSIX: what tsearch() returns points at it).

Touches: `lib/eu-search.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/elfutils; B=build/$QNX_ARCH/elfutils; mkdir -p $B
```

```bash
do_build() { make -C "$B/lib" -j6 && make -C "$B/libelf" -j6 && make -C "$B/config" libelf.pc; }
```

```bash
do_install() { make -C "$B/libelf" DESTDIR="$STAGE" install && \
  install -m644 "$S/libelf/elf.h" "$STAGE$PREFIX/opt/elfutils/include/elf.h" && \
  install -Dm644 "$B/config/libelf.pc" "$STAGE$PREFIX/opt/elfutils/lib/pkgconfig/libelf.pc"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libelf only. The rest of elfutils (libdw, libasm and the eu-* tools) is built on glibc's argp, fts and obstack, which QNX does not have; those would be three more libraries (argp-standalone, musl-fts, musl-obstack) plus the glibc-isms musl distributions patch out. configure insists on all three even for libelf, which uses none: its answers say they are present, and only lib/ (libelf's helper objects) and libelf/ are built.
- The cross-GCC driver, not qcc: elfutils' Makefiles compile with "-MD -MP -MF <file> -c -o <obj>", which qcc misparses as several input files (hazard 18). A prefix of its own, /usr/local/opt/elfutils (include/, lib/, pkgconfig): the SDP has a libelf of its own (elftoolchain's, a static libelf.a with its own <libelf.h>/<gelf.h>), which glib, emacs and make find when they probe -lelf. In the shared prefix this libelf.so would replace it in whatever is built after it -- a run-time dependency on elfutils that depends on build order. Consumers that want it say so: PKG_CONFIG_PATH=.../opt/elfutils/lib/pkgconfig (libelf.pc). Compressed sections: zlib only (no bzip2/lzma/zstd).
- <libelf.h> includes <elf.h>, the ELF definitions that glibc provides and QNX does not (it has its own <sys/elf.h> and elftoolchain's <elfdefinitions.h>). elfutils carries its own copy (libelf/elf.h, from glibc) and builds with it: it is installed alongside, so consumers of <libelf.h>/<gelf.h> compile.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH elfutils`.

### 5. What it installs

10 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/elfutils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa elfutils
ls dists/8.0/*/ | grep '^elfutils-'
```

This writes `elfutils-<ver>-r0.tar.gz` (run time) and `elfutils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add elfutils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/elfutils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/elfutils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_elfutils`](../../tests/src/) links elfutils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-elfutils`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-elfutils
/usr/local/bin/qnx-port-tests/test_elfutils
```

```
== elfutils ==
  ok   ...
PASS elfutils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_elfutils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_elfutils`](../../tests/src/) -- a program linked against elfutils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - elf_version(EV_CURRENT)
   - open own executable
   - elf_begin: ELF_K_ELF
   - ELF64 for this machine
   - little-endian executable
   - section count and string table
   - .text found by name, executable PROGBITS
   - symbol table names elf_begin or main
   - elf_getdata_rawchunk: the ELF magic
   - elf_end (frees the raw-chunk tree)
   - temp file
   - elf64_newehdr
   - elf_update writes the file
   - read back: ET_REL, machine
   - read back: section .qnxdata with the payload

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | elfutils 0.193-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/elfutils-0.193-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | elfutils 0.193-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/elfutils-0.193-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.193**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add elfutils=<version>`), sources in `ported/elfutils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates elfutils`; port one with `scripts/port-update elfutils`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.193
TARBALL=https://sourceware.org/elfutils/ftp/0.193/elfutils-0.193.tar.bz2
SHA256=7857f44b624f4d8d421df851aaae7b1402cfe6bcdd2d8049f15fc07d3dde7635
BUILDSYS=autotools
DEPS="zlib"
# libelf only. The rest of elfutils (libdw, libasm and the eu-* tools) is
# built on glibc's argp, fts and obstack, which QNX does not have; those
# would be three more libraries (argp-standalone, musl-fts, musl-obstack)
# plus the glibc-isms musl distributions patch out. configure insists on
# all three even for libelf, which uses none: its answers say they are
# present, and only lib/ (libelf's helper objects) and libelf/ are built.
CONF_ENV=( ac_cv_search_argp_parse="none required" ac_cv_search_fts_close="none required"
           ac_cv_search__obstack_free="none required"
           CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -D_QNX_SOURCE" )
# The cross-GCC driver, not qcc: elfutils' Makefiles compile with
# "-MD -MP -MF <file> -c -o <obj>", which qcc misparses as several input
# files (hazard 18).
# A prefix of its own, /usr/local/opt/elfutils (include/, lib/, pkgconfig):
# the SDP has a libelf of its own (elftoolchain's, a static libelf.a with
# its own <libelf.h>/<gelf.h>), which glib, emacs and make find when they
# probe -lelf. In the shared prefix this libelf.so would replace it in
# whatever is built after it -- a run-time dependency on elfutils that
# depends on build order. Consumers that want it say so:
# PKG_CONFIG_PATH=.../opt/elfutils/lib/pkgconfig (libelf.pc).
# Compressed sections: zlib only (no bzip2/lzma/zstd).
CONF_ARGS_EXTRA=( --prefix="$PREFIX/opt/elfutils" --disable-debuginfod --disable-libdebuginfod --disable-nls --disable-demangler
                  --without-bzlib --without-lzma --without-zstd --disable-stacktrace )
do_build() { make -C "$B/lib" -j6 && make -C "$B/libelf" -j6 && make -C "$B/config" libelf.pc; }
# <libelf.h> includes <elf.h>, the ELF definitions that glibc provides and
# QNX does not (it has its own <sys/elf.h> and elftoolchain's
# <elfdefinitions.h>). elfutils
# carries its own copy (libelf/elf.h, from glibc) and builds with it: it is
# installed alongside, so consumers of <libelf.h>/<gelf.h> compile.
do_install() { make -C "$B/libelf" DESTDIR="$STAGE" install && \
  install -m644 "$S/libelf/elf.h" "$STAGE$PREFIX/opt/elfutils/include/elf.h" && \
  install -Dm644 "$B/config/libelf.pc" "$STAGE$PREFIX/opt/elfutils/lib/pkgconfig/libelf.pc"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
