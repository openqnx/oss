# libsass 3.6.6

> LibSass, a C/C++ Sass (SCSS) to CSS compiler library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/sass/libsass/archive/3.6.6.tar.gz |
| SHA-256 | `11f0bb3709a4f20285507419d7618f3877a425c0131ea8df40fe6196129df15d` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libsass`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libsass` (programs, shared libraries, run-time data) and `libsass-dev` (headers, pkg-config/CMake files, static libraries; needs `libsass`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libsass`](https://openqnx.github.io/oss/package/x86_64/libsass.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsass.html)) · [`libsass-dev`](https://openqnx.github.io/oss/package/x86_64/libsass-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsass-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libsass` (dependencies come along); one version: `apk add libsass=<version>` | `apk del libsass` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libsass` (dependencies first) | `oss-get remove libsass` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libsass-dev` the same way, by its own name (`apk add libsass-dev`, `oss-get install libsass-dev`, by hand with its own file and list `libsass-dev.list`); remove it before `libsass`, which it needs.

```sh
# on the QNX target, as root
apk add libsass libsass-dev
apk add qnx-test-libsass && /usr/local/bin/qnx-port-tests/test_libsass    # its on-target test, from tests-8.0-<arch>
apk del libsass libsass-dev qnx-test-libsass
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libsass.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libsass.list
rm var/lib/manual/libsass.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libsass` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/sass/libsass/archive/3.6.6.tar.gz
mkdir -p dist src/libsass
curl -fL -o dist/$(basename $URL) $URL
echo "11f0bb3709a4f20285507419d7618f3877a425c0131ea8df40fe6196129df15d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libsass --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libsass; B=build/$QNX_ARCH/libsass; mkdir -p $B
```

```bash
do_build() {
  cp "$B/include/sass/version.h" "$S/include/sass/version.h" && make -C "$B" -j"$(nproc)"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- LibSass: the C/C++ implementation of the Sass CSS preprocessor (libsass.so.1 and its C API, sass.h / sass/context.h), as used by sassc, GTK theme builds and language bindings. Version as in QNX's own package list (the last LibSass release; the project is deprecated in favour of Dart Sass, which needs a Dart VM).
- The archive's own name is 3.6.6.tar.gz: kept in dist/ under a name of its own.
- configure.ac takes the version from version.sh, which asks git first: the GitHub archive has no .git, so git would answer with the version of whatever repository the source sits in. LIBSASS_VERSION answers first.
- configure writes include/sass/version.h (LIBSASS_VERSION) into the build tree, but the archive ships one in the source tree that says "[NA]", and -I$(top_srcdir)/include finds that one first (upstream builds in the source tree): libsass_version() answered "[NA]". The generated header replaces the shipped one before compiling, and is the one installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libsass`.

### 5. What it installs

11 files under `/usr/local`: 2 shared-library entries, 7 headers.

```
/usr/local/lib/libsass.so
/usr/local/lib/libsass.so.1
```

The complete list is `stage/<arch>/.manifest/libsass` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libsass
ls dists/8.0/*/ | grep '^libsass-'
```

This writes `libsass-<ver>-r0.tar.gz` (run time) and `libsass-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libsass
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libsass-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libsass-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libsass`](../../tests/src/) links libsass the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libsass`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libsass
/usr/local/bin/qnx-port-tests/test_libsass
```

```
== libsass ==
  ok   ...
PASS libsass
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libsass` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libsass`](../../tests/src/) -- a program linked against libsass the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libsass_version() is 3.6.6
   - scratch directory
   - compressed: variables, nesting, mixin, @function, @each, darken/lighten, @import
   - expanded style
   - without the include path the @import fails
   - undefined variable: error with line:column

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> libsass`.

## Versions

This recipe builds **3.6.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libsass=<version>`), sources in `ported/libsass/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libsass`; port one with `scripts/port-update libsass`.

<details><summary>recipe.sh</summary>

```bash
# LibSass: the C/C++ implementation of the Sass CSS preprocessor
# (libsass.so.1 and its C API, sass.h / sass/context.h), as used by sassc,
# GTK theme builds and language bindings. Version as in QNX's own package
# list (the last LibSass release; the project is deprecated in favour of
# Dart Sass, which needs a Dart VM).
VERSION=3.6.6
TARBALL=https://github.com/sass/libsass/archive/3.6.6.tar.gz
FETCH_NAME=libsass-3.6.6.tar.gz
SHA256=11f0bb3709a4f20285507419d7618f3877a425c0131ea8df40fe6196129df15d
BUILDSYS=autotools
# The archive's own name is 3.6.6.tar.gz: kept in dist/ under a name of its own.
do_fetch() {
  local tb="$WS/dist/$FETCH_NAME"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL --retry 3 -o "$tb" "$TARBALL" || { rm -f "$tb"; return 1; }
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "checksum mismatch: $tb"; rm -f "$tb"; return 1; }
  tar xzf "$tb" -C "$SRCROOT" --strip-components=1
}
DEPS=""
# configure.ac takes the version from version.sh, which asks git first:
# the GitHub archive has no .git, so git would answer with the version of
# whatever repository the source sits in. LIBSASS_VERSION answers first.
export LIBSASS_VERSION=$VERSION
CONF_ARGS_EXTRA=( --disable-tests )
# configure writes include/sass/version.h (LIBSASS_VERSION) into the build
# tree, but the archive ships one in the source tree that says "[NA]", and
# -I$(top_srcdir)/include finds that one first (upstream builds in the
# source tree): libsass_version() answered "[NA]". The generated header
# replaces the shipped one before compiling, and is the one installed.
do_build() {
  cp "$B/include/sass/version.h" "$S/include/sass/version.h" && make -C "$B" -j"$(nproc)"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
