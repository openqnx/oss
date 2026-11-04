# gumbo-parser 0.13.2

> HTML5 parsing library in pure C99

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://codeberg.org/grisha/gumbo-parser.git (tag `0.13.2`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gumbo_parser`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gumbo-parser` (programs, shared libraries, run-time data) and `gumbo-parser-dev` (headers, pkg-config/CMake files, static libraries; needs `gumbo-parser`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gumbo-parser`](https://openqnx.github.io/oss/package/x86_64/gumbo-parser.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gumbo-parser.html)) · [`gumbo-parser-dev`](https://openqnx.github.io/oss/package/x86_64/gumbo-parser-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gumbo-parser-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gumbo-parser` (dependencies come along); one version: `apk add gumbo-parser=<version>` | `apk del gumbo-parser` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gumbo-parser` (dependencies first) | `oss-get remove gumbo-parser` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gumbo-parser-dev` the same way, by its own name (`apk add gumbo-parser-dev`, `oss-get install gumbo-parser-dev`, by hand with its own file and list `gumbo-parser-dev.list`); remove it before `gumbo-parser`, which it needs.

```sh
# on the QNX target, as root
apk add gumbo-parser gumbo-parser-dev
apk add qnx-test-gumbo-parser && /usr/local/bin/qnx-port-tests/test_gumbo_parser    # its on-target test, from tests-8.0-<arch>
apk del gumbo-parser gumbo-parser-dev qnx-test-gumbo-parser
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gumbo-parser.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gumbo-parser.list
rm var/lib/manual/gumbo-parser.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gumbo-parser` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.13.2 --recurse-submodules --shallow-submodules \
    https://codeberg.org/grisha/gumbo-parser.git src/gumbo-parser
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gumbo-parser src/gumbo-parser \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dexamples=false \
      -Dfuzz=false
ninja -C build/$QNX_ARCH/gumbo-parser
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gumbo-parser install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- gumbo-parser: an HTML5 parsing library in pure C99 (the WHATWG parsing algorithm), originally Google's, maintained on Codeberg. litehtml parses its documents with it.
- The shared library only: its tests need gtest, the examples are demos.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gumbo-parser`.

### 5. What it installs

7 files under `/usr/local`: 3 shared-library entries, 2 headers.

```
/usr/local/lib/libgumbo.so
/usr/local/lib/libgumbo.so.3
/usr/local/lib/libgumbo.so.3.0.0
```

The complete list is `stage/<arch>/.manifest/gumbo-parser` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gumbo-parser
ls dists/8.0/*/ | grep '^gumbo-parser-'
```

This writes `gumbo-parser-<ver>-r0.tar.gz` (run time) and `gumbo-parser-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gumbo-parser
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gumbo-parser-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gumbo-parser-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gumbo_parser`](../../tests/src/) links gumbo-parser the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gumbo-parser`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gumbo-parser
/usr/local/bin/qnx-port-tests/test_gumbo_parser
```

```
== gumbo-parser ==
  ok   ...
PASS gumbo-parser
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gumbo_parser` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gumbo_parser`](../../tests/src/) -- a program linked against gumbo-parser the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - root is an implied <html>
   - implied <head> and <body>
   - <title> in head, &amp; decoded to 'T&Q'
   - the unclosed <p> is closed: two sibling <p>
   - unquoted attributes class=a id=first
   - texts 'one' and 'two <3>'
   - <a href=x.html>link</a> inside the second <p>
   - source position of the first <p>
   - foster parenting: stray table text moved before the table
   - implied <tbody>
   - tag names

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> gumbo-parser`.

## Versions

This recipe builds **0.13.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gumbo-parser=<version>`), sources in `ported/gumbo-parser/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gumbo-parser`; port one with `scripts/port-update gumbo-parser`.

<details><summary>recipe.sh</summary>

```bash
# gumbo-parser: an HTML5 parsing library in pure C99 (the WHATWG parsing
# algorithm), originally Google's, maintained on Codeberg. litehtml parses
# its documents with it.
VERSION=0.13.2; GIT=https://codeberg.org/grisha/gumbo-parser.git; REF=0.13.2
DEPS=""
BUILDSYS=meson
# The shared library only: its tests need gtest, the examples are demos.
MESON_ARGS_EXTRA=( -Dtests=false -Dexamples=false -Dfuzz=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
