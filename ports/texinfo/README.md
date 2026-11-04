# texinfo 7.2

> GNU documentation system: makeinfo/texi2any, the info reader, install-info, texindex

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/texinfo/texinfo-7.2.tar.xz |
| SHA-256 | `0329d7788fbef113fa82cb80889ca197a344ce0df7646fe000974c5d714363a6` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_texinfo`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `texinfo` (programs, shared libraries, run-time data) and `texinfo-dev` (headers, pkg-config/CMake files, static libraries; needs `texinfo`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`texinfo`](https://openqnx.github.io/oss/package/x86_64/texinfo.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/texinfo.html)) · [`texinfo-dev`](https://openqnx.github.io/oss/package/x86_64/texinfo-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/texinfo-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add texinfo` (dependencies come along); one version: `apk add texinfo=<version>` | `apk del texinfo` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install texinfo` (dependencies first) | `oss-get remove texinfo` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`texinfo-dev` the same way, by its own name (`apk add texinfo-dev`, `oss-get install texinfo-dev`, by hand with its own file and list `texinfo-dev.list`); remove it before `texinfo`, which it needs.

```sh
# on the QNX target, as root
apk add texinfo texinfo-dev
apk add qnx-test-texinfo && /usr/local/bin/qnx-port-tests/test_texinfo    # its on-target test, from tests-8.0-<arch>-m-z
apk del texinfo texinfo-dev qnx-test-texinfo
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/texinfo.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/texinfo.list
rm var/lib/manual/texinfo.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild texinfo` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ncurses
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/texinfo/texinfo-7.2.tar.xz
mkdir -p dist src/texinfo
curl -fL -o dist/$(basename $URL) $URL
echo "0329d7788fbef113fa82cb80889ca197a344ce0df7646fe000974c5d714363a6  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/texinfo --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/texinfo; B=build/$QNX_ARCH/texinfo; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  local f
  # The Perl programs start with the build machine's #! /usr/bin/perl
  # (hazard 105); a QNX target's perl is the perl port's.
  for f in texi2any pod2texi; do
    sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/bin/$f" || return 1
  done
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU Texinfo: makeinfo/texi2any (Texinfo to Info, HTML, plain text, DocBook, LaTeX), the stand-alone info reader, install-info, texindex and the texi2dvi/texi2pdf drivers (which need TeX). Version as in QNX's own package list.
- ncurses: the info reader's terminal handling (tgetent).
- texi2any, makeinfo and pod2texi are Perl programs (with Text::Unidecode, libintl-perl and Unicode::EastAsianWidth bundled in share/texi2any/lib); texindex is sh + awk.
- --disable-perl-xs: texi2any's optional XS modules (the faster C parser and converters) are built against the perl that configure runs -- the build machine's (its CORE headers, gcc, x86-64 Linux objects in a QNX package). Without them texi2any uses its pure-Perl implementation, which gives the same output. --disable-nls, am_cv_func_iconv=no: no gettext/libiconv in the C programs (the build machine's perl runs only to configure; PERL is its path). ac_cv_path_TI_AWK=awk: texindex runs the awk configure found, by its build-machine path (/usr/bin/awk); a QNX target's awk is /system/bin/awk (toybox), so texindex takes "awk" from PATH.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH texinfo`.

### 5. What it installs

495 files under `/usr/local`, 9 programs.

```
/usr/local/bin/info
/usr/local/bin/install-info
/usr/local/bin/makeinfo
/usr/local/bin/pdftexi2dvi
/usr/local/bin/pod2texi
/usr/local/bin/texi2any
/usr/local/bin/texi2dvi
/usr/local/bin/texi2pdf
/usr/local/bin/texindex
```

The complete list is `stage/<arch>/.manifest/texinfo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa texinfo
ls dists/8.0/*/ | grep '^texinfo-'
```

This writes `texinfo-<ver>-r0.tar.gz` (run time) and `texinfo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add texinfo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/texinfo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/texinfo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_texinfo`](../../tests/src/) links texinfo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-texinfo`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-texinfo
/usr/local/bin/qnx-port-tests/test_texinfo
```

```
== texinfo ==
  ok   ...
PASS texinfo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_texinfo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_texinfo`](../../tests/src/) -- a program linked against texinfo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> texinfo`.

## Versions

This recipe builds **7.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add texinfo=<version>`), sources in `ported/texinfo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates texinfo`; port one with `scripts/port-update texinfo`.

<details><summary>recipe.sh</summary>

```bash
# GNU Texinfo: makeinfo/texi2any (Texinfo to Info, HTML, plain text,
# DocBook, LaTeX), the stand-alone info reader, install-info, texindex and
# the texi2dvi/texi2pdf drivers (which need TeX). Version as in QNX's own
# package list.
VERSION=7.2
TARBALL=https://ftp.gnu.org/gnu/texinfo/texinfo-7.2.tar.xz
SHA256=0329d7788fbef113fa82cb80889ca197a344ce0df7646fe000974c5d714363a6
# ncurses: the info reader's terminal handling (tgetent).
DEPS="ncurses"
# texi2any, makeinfo and pod2texi are Perl programs (with Text::Unidecode,
# libintl-perl and Unicode::EastAsianWidth bundled in share/texi2any/lib);
# texindex is sh + awk.
RUNTIME_DEPS="perl"
BUILDSYS=autotools
# --disable-perl-xs: texi2any's optional XS modules (the faster C parser
# and converters) are built against the perl that configure runs -- the
# build machine's (its CORE headers, gcc, x86-64 Linux objects in a QNX
# package). Without them texi2any uses its pure-Perl implementation, which
# gives the same output.
# --disable-nls, am_cv_func_iconv=no: no gettext/libiconv in the C programs
# (the build machine's perl runs only to configure; PERL is its path).
# ac_cv_path_TI_AWK=awk: texindex runs the awk configure found, by its
# build-machine path (/usr/bin/awk); a QNX target's awk is /system/bin/awk
# (toybox), so texindex takes "awk" from PATH.
CONF_ENV=( PERL=/usr/bin/perl am_cv_func_iconv=no ac_cv_path_TI_AWK=awk )
CONF_ARGS_EXTRA=( --disable-perl-xs --disable-nls --disable-rpath
                  --without-libiconv-prefix --without-libintl-prefix )
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  local f
  # The Perl programs start with the build machine's #! /usr/bin/perl
  # (hazard 105); a QNX target's perl is the perl port's.
  for f in texi2any pod2texi; do
    sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/bin/$f" || return 1
  done
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
