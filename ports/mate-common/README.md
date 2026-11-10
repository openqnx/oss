# mate-common 1.28.0

> Autoconf macros and bootstrap scripts for MATE desktop modules

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://pub.mate-desktop.org/releases/1.28/mate-common-1.28.0.tar.xz |
| SHA-256 | `42b7c2cee268f71d7e1c1ad4f69bcda0ecd65578a96722187c6b76378d26ba0a` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mate_common`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `mate-common` (programs, shared libraries, run-time data) and `mate-common-dev` (headers, pkg-config/CMake files, static libraries; needs `mate-common`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`mate-common`](https://openqnx.github.io/oss/package/x86_64/mate-common.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mate-common.html)) · [`mate-common-dev`](https://openqnx.github.io/oss/package/x86_64/mate-common-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mate-common-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add mate-common` (dependencies come along); one version: `apk add mate-common=<version>` | `apk del mate-common` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install mate-common` (dependencies first) | `oss-get remove mate-common` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`mate-common-dev` the same way, by its own name (`apk add mate-common-dev`, `oss-get install mate-common-dev`, by hand with its own file and list `mate-common-dev.list`); remove it before `mate-common`, which it needs.

```sh
# on the QNX target, as root
apk add mate-common mate-common-dev
apk add qnx-test-mate-common && /usr/local/bin/qnx-port-tests/test_mate_common    # its on-target test, from tests-8.0-<arch>-m-z
apk del mate-common mate-common-dev qnx-test-mate-common
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/mate-common.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/mate-common.list
rm var/lib/manual/mate-common.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mate-common` performs exactly the same steps (2–5) in one command.

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
URL=https://pub.mate-desktop.org/releases/1.28/mate-common-1.28.0.tar.xz
mkdir -p dist src/mate-common
curl -fL -o dist/$(basename $URL) $URL
echo "42b7c2cee268f71d7e1c1ad4f69bcda0ecd65578a96722187c6b76378d26ba0a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mate-common --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mate-common && cd build/$QNX_ARCH/mate-common
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/mate-common/configure --host=$QNX_TRIPLE --prefix=/usr/local
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- mate-common: the autoconf macros (MATE_COMPILE_WARNINGS, MATE_DEBUG_CHECK, MATE_MAINTAINER_MODE_DEFINES, ...) and the mate-autogen / mate-doc-common scripts that MATE desktop modules bootstrap with. Data and shell scripts. Version as in QNX's own package list.
- mate-autogen runs autoreconf, aclocal and automake: on a target, the autoconf and automake ports.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mate-common`.

### 5. What it installs

9 files under `/usr/local`, 2 programs.

```
/usr/local/bin/mate-autogen
/usr/local/bin/mate-doc-common
```

The complete list is `stage/<arch>/.manifest/mate-common` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mate-common
ls dists/8.0/*/ | grep '^mate-common-'
```

This writes `mate-common-<ver>-r0.tar.gz` (run time) and `mate-common-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mate-common
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mate-common-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mate-common-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mate_common`](../../tests/src/) links mate-common the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mate-common`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mate-common
/usr/local/bin/qnx-port-tests/test_mate_common
```

```
== mate-common ==
  ok   ...
PASS mate-common
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mate_common` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mate_common`](../../tests/src/) -- a program linked against mate-common the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - replace with a real check

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mate-common 1.28.0-r0 | ✅ PASS | 2026-11-10 | [log](../../tests/logs/aarch64le/mate-common-1.28.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mate-common 1.28.0-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/mate-common-1.28.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.28.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mate-common=<version>`), sources in `ported/mate-common/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mate-common`; port one with `scripts/port-update mate-common`.

<details><summary>recipe.sh</summary>

```bash
# mate-common: the autoconf macros (MATE_COMPILE_WARNINGS, MATE_DEBUG_CHECK,
# MATE_MAINTAINER_MODE_DEFINES, ...) and the mate-autogen / mate-doc-common
# scripts that MATE desktop modules bootstrap with. Data and shell scripts.
# Version as in QNX's own package list.
VERSION=1.28.0
TARBALL=https://pub.mate-desktop.org/releases/1.28/mate-common-1.28.0.tar.xz
SHA256=42b7c2cee268f71d7e1c1ad4f69bcda0ecd65578a96722187c6b76378d26ba0a
DEPS=""
# mate-autogen runs autoreconf, aclocal and automake: on a target, the
# autoconf and automake ports.
RUNTIME_DEPS="autoconf automake"
BUILDSYS=autotools
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
