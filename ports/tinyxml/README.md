# tinyxml 2.6.2

> Simple, small, C++ XML parser (the original TinyXML)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.tar.gz |
| SHA-256 | `15bdfdcec58a7da30adc87ac2b078e4417dbe5392f3afb719f9ba6d062645593` |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_tinyxml`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `tinyxml` (programs, shared libraries, run-time data) and `tinyxml-dev` (headers, pkg-config/CMake files, static libraries; needs `tinyxml`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`tinyxml`](https://openqnx.github.io/oss/package/x86_64/tinyxml.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tinyxml.html)) · [`tinyxml-dev`](https://openqnx.github.io/oss/package/x86_64/tinyxml-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tinyxml-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add tinyxml` (dependencies come along); one version: `apk add tinyxml=<version>` | `apk del tinyxml` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install tinyxml` (dependencies first) | `oss-get remove tinyxml` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`tinyxml-dev` the same way, by its own name (`apk add tinyxml-dev`, `oss-get install tinyxml-dev`, by hand with its own file and list `tinyxml-dev.list`); remove it before `tinyxml`, which it needs.

```sh
# on the QNX target, as root
apk add tinyxml tinyxml-dev
apk add qnx-test-tinyxml && /usr/local/bin/qnx-port-tests/test_tinyxml    # its on-target test, from tests-8.0-<arch>-m-z
apk del tinyxml tinyxml-dev qnx-test-tinyxml
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/tinyxml.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/tinyxml.list
rm var/lib/manual/tinyxml.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tinyxml` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.tar.gz
mkdir -p dist src/tinyxml
curl -fL -o dist/$(basename $URL) $URL
echo "15bdfdcec58a7da30adc87ac2b078e4417dbe5392f3afb719f9ba6d062645593  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tinyxml --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/tinyxml < ports/tinyxml/patches/0001-always-use-stl.patch
patch -p1 -d src/tinyxml < ports/tinyxml/patches/0002-parser-cve-fixes.patch
```

**[`0001-always-use-stl.patch`](patches/0001-always-use-stl.patch)** — Always build and use the STL interface (TIXML_USE_STL)

> TIXML_USE_STL changes the layout of TiXmlString-holding classes (std::string
> instead of TinyXML's own TiXmlString) and adds std::string/iostream methods.
> The shared library is compiled with it, so a program that includes
> tinyxml.h without -DTIXML_USE_STL would use a different class layout than
> the library it links. The header now defines it itself, as the
> distributions' libtinyxml packages do.

Touches: `tinyxml.h` · Upstream status: Inappropriate (packaging: upstream ships no library build)

**[`0002-parser-cve-fixes.patch`](patches/0002-parser-cve-fixes.patch)** — Fix two parser denial-of-service bugs (CVE-2021-42260, CVE-2023-34194)

> TinyXML 2.6.2 is the last release and upstream is unmaintained, so the two
> known parser bugs are fixed here:
>
> - CVE-2021-42260: TiXmlParsingData::Stamp() never advanced over a UTF-8
>   lead byte 0xEF followed by fewer than two more bytes, looping forever on
>   such input (UTF-8 encoding only). The byte is now stepped over.
> - CVE-2023-34194: TiXmlDeclaration::Parse() called StringEqual() on the end
>   of the input when a declaration was cut short after white space, which is
>   an assertion failure (abort) in builds with assertions on. The loop now
>   stops there; the caller reports the unterminated declaration.

Touches: `tinyxmlparser.cpp` · Upstream status: Inappropriate (upstream inactive since 2011)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tinyxml; B=build/$QNX_ARCH/tinyxml; mkdir -p $B
```

```bash
do_build() {
  local f objs=
  for f in $TX_SRCS; do
    q++ -V${QCC_TARGET}_cxx -O2 -fPIC -DNDEBUG -D_QNX_SOURCE -DTIXML_USE_STL -c "$S/$f" -o "$B/${f%.cpp}.o" || return 1
    objs="$objs $B/${f%.cpp}.o"
  done
  q++ -V${QCC_TARGET}_cxx -shared -Wl,-soname,libtinyxml.so.0 -o "$B/libtinyxml.so.0.$VERSION" $objs -lm || return 1
  # upstream's test program, linked only to prove the library links (not installed)
  q++ -V${QCC_TARGET}_cxx -O2 -D_QNX_SOURCE -DTIXML_USE_STL -I"$S" "$S/xmltest.cpp" -o "$B/xmltest" -L"$B" -l:libtinyxml.so.0.$VERSION
}
```

```bash
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" &&
  install -m755 "$B/libtinyxml.so.0.$VERSION" "$L/" &&
  ln -sf "libtinyxml.so.0.$VERSION" "$L/libtinyxml.so.0" && ln -sf libtinyxml.so.0 "$L/libtinyxml.so" &&
  install -m644 "$S/tinyxml.h" "$S/tinystr.h" "$STAGE$PREFIX/include/" &&
  cat > "$L/pkgconfig/tinyxml.pc" <<PC
prefix=$PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: TinyXml
Description: simple, small, C++ XML parser
Version: $VERSION
Libs: -L\${libdir} -ltinyxml
Cflags: -I\${includedir} -DTIXML_USE_STL=YES
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The original TinyXML (not TinyXML-2, which is the tinyxml2 component and installs tinyxml2.h / libtinyxml2: the two coexist). Upstream ships only a Makefile for its test program, so the library is built here the way the Linux distributions build it: libtinyxml.so.0 with the STL interface (patches/0001 makes the header say so), plus tinyxml.pc. patches/0002 fixes the two published parser bugs; upstream has been inactive since 2011.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tinyxml`.

### 5. What it installs

6 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libtinyxml.so
/usr/local/lib/libtinyxml.so.0
/usr/local/lib/libtinyxml.so.0.2.6.2
```

The complete list is `stage/<arch>/.manifest/tinyxml` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tinyxml
ls dists/8.0/*/ | grep '^tinyxml-'
```

This writes `tinyxml-<ver>-r0.tar.gz` (run time) and `tinyxml-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tinyxml
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tinyxml-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tinyxml-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_tinyxml`](../../tests/src/) links tinyxml the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-tinyxml`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-tinyxml
/usr/local/bin/qnx-port-tests/test_tinyxml
```

```
== tinyxml ==
  ok   ...
PASS tinyxml
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tinyxml` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_tinyxml`](../../tests/src/) -- a program linked against tinyxml the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - Parse() of a well-formed document
   - root element is <config>
   - typed attributes version=3, ratio=0.75
   - a missing attribute: TIXML_NO_ATTRIBUTE
   - two <item>s, entity &amp; decoded
   - TiXmlHandle path to the second item
   - declaration version 1.0
   - the comment node
   - mismatched tags: an error, with its row
   - SaveFile
   - LoadFile reads it back
   - operator>> (STL interface)
   - truncated UTF-8 lead byte: an error, no hang (CVE-2021-42260)
   - cut-off XML declaration: no abort (CVE-2023-34194)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tinyxml 2.6.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/tinyxml-2.6.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tinyxml 2.6.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/tinyxml-2.6.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.6.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tinyxml=<version>`), sources in `ported/tinyxml/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tinyxml`; port one with `scripts/port-update tinyxml`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.6.2
TARBALL=https://downloads.sourceforge.net/project/tinyxml/tinyxml/2.6.2/tinyxml_2_6_2.tar.gz
SHA256=15bdfdcec58a7da30adc87ac2b078e4417dbe5392f3afb719f9ba6d062645593
BUILDSYS=custom
# The original TinyXML (not TinyXML-2, which is the tinyxml2 component and
# installs tinyxml2.h / libtinyxml2: the two coexist). Upstream ships only a
# Makefile for its test program, so the library is built here the way the
# Linux distributions build it: libtinyxml.so.0 with the STL interface
# (patches/0001 makes the header say so), plus tinyxml.pc. patches/0002 fixes
# the two published parser bugs; upstream has been inactive since 2011.
TX_SRCS="tinyxml.cpp tinystr.cpp tinyxmlerror.cpp tinyxmlparser.cpp"
do_build() {
  local f objs=
  for f in $TX_SRCS; do
    q++ -V${QCC_TARGET}_cxx -O2 -fPIC -DNDEBUG -D_QNX_SOURCE -DTIXML_USE_STL -c "$S/$f" -o "$B/${f%.cpp}.o" || return 1
    objs="$objs $B/${f%.cpp}.o"
  done
  q++ -V${QCC_TARGET}_cxx -shared -Wl,-soname,libtinyxml.so.0 -o "$B/libtinyxml.so.0.$VERSION" $objs -lm || return 1
  # upstream's test program, linked only to prove the library links (not installed)
  q++ -V${QCC_TARGET}_cxx -O2 -D_QNX_SOURCE -DTIXML_USE_STL -I"$S" "$S/xmltest.cpp" -o "$B/xmltest" -L"$B" -l:libtinyxml.so.0.$VERSION
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" &&
  install -m755 "$B/libtinyxml.so.0.$VERSION" "$L/" &&
  ln -sf "libtinyxml.so.0.$VERSION" "$L/libtinyxml.so.0" && ln -sf libtinyxml.so.0 "$L/libtinyxml.so" &&
  install -m644 "$S/tinyxml.h" "$S/tinystr.h" "$STAGE$PREFIX/include/" &&
  cat > "$L/pkgconfig/tinyxml.pc" <<PC
prefix=$PREFIX
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: TinyXml
Description: simple, small, C++ XML parser
Version: $VERSION
Libs: -L\${libdir} -ltinyxml
Cflags: -I\${includedir} -DTIXML_USE_STL=YES
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
