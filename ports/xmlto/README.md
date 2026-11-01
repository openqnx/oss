# xmlto 0.0.29

> Tool for converting DocBook XML files to man pages, HTML and other formats

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://pagure.io/xmlto.git (tag `0.0.29`) |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xmlto`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `xmlto`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xmlto`](https://openqnx.github.io/oss/package/x86_64/xmlto.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xmlto.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xmlto` (dependencies come along); one version: `apk add xmlto=<version>` | `apk del xmlto` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xmlto` (dependencies first) | `oss-get remove xmlto` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-xmlto && /usr/local/bin/qnx-port-tests/test_xmlto    # its on-target test, from tests-8.0-<arch>-m-z
apk del xmlto qnx-test-xmlto
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xmlto.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xmlto.list
rm var/lib/manual/xmlto.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xmlto` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.0.29 --recurse-submodules --shallow-submodules \
    https://pagure.io/xmlto.git src/xmlto
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xmlto < ports/xmlto/patches/0001-cross-getopt-check.patch
patch -p1 -d src/xmlto < ports/xmlto/patches/0002-getopt-with-arguments.patch
```

**[`0001-cross-getopt-check.patch`](patches/0001-cross-getopt-check.patch)** — configure: do not run the target's getopt when cross compiling

> configure runs $GETOPT on the build machine to check that it supports
> --longoptions. When cross compiling, the getopt named for the target (here
> BusyBox's, at its target path) cannot run there, and configure stopped.
> Skip that run-time check when cross compiling.

Touches: `configure.ac` · Upstream status: Pending

**[`0002-getopt-with-arguments.patch`](patches/0002-getopt-with-arguments.patch)** — xmlto: allow a getopt command with arguments ("busybox getopt")

> QNX has no getopt(1); BusyBox's is the enhanced getopt xmlto needs, run
> as "busybox getopt". Quote the substituted value so that the assignment
> keeps both words; ${GETOPT} is expanded unquoted where it runs, so it is
> split back into command and argument.

Touches: `xmlto.in` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xmlto; B=build/$QNX_ARCH/xmlto; mkdir -p $B
```

```bash
do_build() { make -C "$B" xmlto xmlif/xmlif; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install-binSCRIPTS install-binPROGRAMS install-nobase_pkgdataDATA; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xmlto 0.0.29: converts DocBook XML to man pages, HTML, XHTML, plain text ... by running xsltproc with the DocBook XSL stylesheets (a bash script), plus xmlif, its small conditional-markup preprocessor (C, from flex). qnx-ports has a branch for it (qnx-0.0.29) with no QNX changes; the patches here are this tree's.
- pagure.io serves git over dumb HTTP, which cannot clone shallow (the default fetch): a full clone of the (small) repository, then the tag.
- What the script runs on the target. QNX has no getopt(1) at all; BusyBox's (built with long options) is the enhanced getopt xmlto needs. find, mktemp and tail are QNX's (toybox: find has -maxdepth); bash, xsltproc, xmllint and the DocBook XSL stylesheets come from this tree.
- configure would look the rest up on the build machine (/usr/bin/tail ...): the cache answers keep them plain names, found in the target's PATH.
- The man pages are generated by running xmlto itself on the build machine (with the target's bash and xsltproc paths): they are left out, and only the script, xmlif and the format scripts are built and installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xmlto`.

### 5. What it installs

37 files under `/usr/local`, 2 programs.

```
/usr/local/bin/xmlif
/usr/local/bin/xmlto
```

The complete list is `stage/<arch>/.manifest/xmlto` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xmlto
ls dists/8.0/*/ | grep '^xmlto-'
```

This writes `xmlto-<ver>-r0.tar.gz` (run time) and `xmlto-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xmlto
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xmlto-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xmlto-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xmlto`](../../tests/src/) links xmlto the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xmlto`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xmlto
/usr/local/bin/qnx-port-tests/test_xmlto
```

```
== xmlto ==
  ok   ...
PASS xmlto
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xmlto` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xmlto`](../../tests/src/) -- a program linked against xmlto the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xmlto 0.0.29-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/xmlto-0.0.29-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xmlto 0.0.29-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/xmlto-0.0.29-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.0.29**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xmlto=<version>`), sources in `ported/xmlto/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xmlto`; port one with `scripts/port-update xmlto`.

<details><summary>recipe.sh</summary>

```bash
# xmlto 0.0.29: converts DocBook XML to man pages, HTML, XHTML, plain text
# ... by running xsltproc with the DocBook XSL stylesheets (a bash script),
# plus xmlif, its small conditional-markup preprocessor (C, from flex).
# qnx-ports has a branch for it (qnx-0.0.29) with no QNX changes; the
# patches here are this tree's.
VERSION=0.0.29; GIT=https://pagure.io/xmlto.git; REF=0.0.29
BUILDSYS=autotools
# pagure.io serves git over dumb HTTP, which cannot clone shallow (the
# default fetch): a full clone of the (small) repository, then the tag.
do_fetch() { git clone -q "$GIT" "$SRCROOT" && git -C "$SRCROOT" -c advice.detachedHead=false checkout -q "$REF"; }
# What the script runs on the target. QNX has no getopt(1) at all; BusyBox's
# (built with long options) is the enhanced getopt xmlto needs. find,
# mktemp and tail are QNX's (toybox: find has -maxdepth); bash, xsltproc,
# xmllint and the DocBook XSL stylesheets come from this tree.
RUNTIME_DEPS="bash busybox xsltproc libxml2-utils docbook-xsl"
# configure would look the rest up on the build machine (/usr/bin/tail ...):
# the cache answers keep them plain names, found in the target's PATH.
CONF_ENV=( XMLTO_BASH_PATH="$PREFIX/bin/bash" GETOPT="$PREFIX/bin/busybox getopt"
           XSLTPROC="$PREFIX/bin/xsltproc" XMLLINT="$PREFIX/bin/xmllint" FIND=find MKTEMP=mktemp
           ac_cv_path_TAIL=tail ac_cv_path_GREP=grep ac_cv_path_SED=sed ac_cv_path_GCP=cp
           ac_cv_path_PAPER_CONF=paperconf ac_cv_path_LOCALE=locale ac_cv_path_ZIP=zip
           ac_cv_path_DBLATEX=dblatex ac_cv_path_FOP=fop ac_cv_path_XMLTEX=xmltex
           ac_cv_path_PDFXMLTEX=pdfxmltex ac_cv_path_LYNX=lynx ac_cv_path_LINKS=elinks ac_cv_path_W3M=w3m )
# The man pages are generated by running xmlto itself on the build machine
# (with the target's bash and xsltproc paths): they are left out, and only
# the script, xmlif and the format scripts are built and installed.
do_build() { make -C "$B" xmlto xmlif/xmlif; }
do_install() { make -C "$B" DESTDIR="$STAGE" install-binSCRIPTS install-binPROGRAMS install-nobase_pkgdataDATA; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
