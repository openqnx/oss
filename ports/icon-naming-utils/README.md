# icon-naming-utils 0.8.90

> Maps legacy GNOME/KDE icon names to the freedesktop.org icon naming specification

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | http://tango.freedesktop.org/releases/icon-naming-utils-0.8.90.tar.bz2 |
| SHA-256 | `b1378679df4485b4459f609a3304238b3e92d91e43744c47b70abbe690d883d5` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_icon_naming_utils`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `icon-naming-utils` (programs, shared libraries, run-time data) and `icon-naming-utils-dev` (headers, pkg-config/CMake files, static libraries; needs `icon-naming-utils`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`icon-naming-utils`](https://openqnx.github.io/oss/package/x86_64/icon-naming-utils.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/icon-naming-utils.html)) · [`icon-naming-utils-dev`](https://openqnx.github.io/oss/package/x86_64/icon-naming-utils-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/icon-naming-utils-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add icon-naming-utils` (dependencies come along); one version: `apk add icon-naming-utils=<version>` | `apk del icon-naming-utils` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install icon-naming-utils` (dependencies first) | `oss-get remove icon-naming-utils` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`icon-naming-utils-dev` the same way, by its own name (`apk add icon-naming-utils-dev`, `oss-get install icon-naming-utils-dev`, by hand with its own file and list `icon-naming-utils-dev.list`); remove it before `icon-naming-utils`, which it needs.

```sh
# on the QNX target, as root
apk add icon-naming-utils icon-naming-utils-dev
apk add qnx-test-icon-naming-utils && /usr/local/bin/qnx-port-tests/test_icon_naming_utils    # its on-target test, from tests-8.0-<arch>
apk del icon-naming-utils icon-naming-utils-dev qnx-test-icon-naming-utils
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/icon-naming-utils.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/icon-naming-utils.list
rm var/lib/manual/icon-naming-utils.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild icon-naming-utils` performs exactly the same steps (2–5) in one command.

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
URL=http://tango.freedesktop.org/releases/icon-naming-utils-0.8.90.tar.bz2
mkdir -p dist src/icon-naming-utils
curl -fL -o dist/$(basename $URL) $URL
echo "b1378679df4485b4459f609a3304238b3e92d91e43744c47b70abbe690d883d5  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/icon-naming-utils --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/icon-naming-utils; B=build/$QNX_ARCH/icon-naming-utils; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  # #!/usr/bin/perl -w is the build machine's (hazard 105).
  sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/libexec/icon-name-mapping"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- icon-naming-utils: icon-name-mapping, the Perl script icon themes run at install time to add the legacy (GNOME/KDE) icon names as symlinks to the freedesktop.org Icon Naming Specification names, with its mapping data. Version as in QNX's own package list.
- The script uses XML::Simple (which parses with XML::Parser) to read the mapping.
- configure requires XML::Simple in the perl it runs -- the build machine's, which may not have it: the perl-xml-simple port's copy in staging (pure Perl) is on PERL5LIB for that check.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH icon-naming-utils`.

### 5. What it installs

4 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/icon-naming-utils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa icon-naming-utils
ls dists/8.0/*/ | grep '^icon-naming-utils-'
```

This writes `icon-naming-utils-<ver>-r0.tar.gz` (run time) and `icon-naming-utils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add icon-naming-utils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/icon-naming-utils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/icon-naming-utils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_icon_naming_utils`](../../tests/src/) links icon-naming-utils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-icon-naming-utils`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-icon-naming-utils
/usr/local/bin/qnx-port-tests/test_icon_naming_utils
```

```
== icon-naming-utils ==
  ok   ...
PASS icon-naming-utils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_icon_naming_utils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_icon_naming_utils`](../../tests/src/) -- a program linked against icon-naming-utils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - replace with a real check

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> icon-naming-utils`.

## Versions

This recipe builds **0.8.90**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add icon-naming-utils=<version>`), sources in `ported/icon-naming-utils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates icon-naming-utils`; port one with `scripts/port-update icon-naming-utils`.

<details><summary>recipe.sh</summary>

```bash
# icon-naming-utils: icon-name-mapping, the Perl script icon themes run at
# install time to add the legacy (GNOME/KDE) icon names as symlinks to the
# freedesktop.org Icon Naming Specification names, with its mapping data.
# Version as in QNX's own package list.
VERSION=0.8.90
TARBALL=http://tango.freedesktop.org/releases/icon-naming-utils-0.8.90.tar.bz2
SHA256=b1378679df4485b4459f609a3304238b3e92d91e43744c47b70abbe690d883d5
DEPS=""
# The script uses XML::Simple (which parses with XML::Parser) to read the
# mapping.
RUNTIME_DEPS="perl perl-xml-simple perl-xml-parser"
BUILDSYS=autotools
# configure requires XML::Simple in the perl it runs -- the build machine's,
# which may not have it: the perl-xml-simple port's copy in staging (pure
# Perl) is on PERL5LIB for that check.
CONF_ENV=( PERL=/usr/bin/perl PERL5LIB="$STAGE_SYSROOT/usr/local/lib/perl5/site_perl/5.40.2" )
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  # #!/usr/bin/perl -w is the build machine's (hazard 105).
  sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/libexec/icon-name-mapping"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
