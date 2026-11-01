# zerotier-one 1.16.1

> ZeroTier network virtualization service

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/zerotier/ZeroTierOne.git (tag `1.16.1`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zerotier_one`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `zerotier-one` (programs, shared libraries, run-time data) and `zerotier-one-dev` (headers, pkg-config/CMake files, static libraries; needs `zerotier-one`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zerotier-one`](https://openqnx.github.io/oss/package/x86_64/zerotier-one.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zerotier-one.html)) · [`zerotier-one-dev`](https://openqnx.github.io/oss/package/x86_64/zerotier-one-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zerotier-one-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zerotier-one` (dependencies come along); one version: `apk add zerotier-one=<version>` | `apk del zerotier-one` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zerotier-one` (dependencies first) | `oss-get remove zerotier-one` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`zerotier-one-dev` the same way, by its own name (`apk add zerotier-one-dev`, `oss-get install zerotier-one-dev`, by hand with its own file and list `zerotier-one-dev.list`); remove it before `zerotier-one`, which it needs.

```sh
# on the QNX target, as root
apk add zerotier-one zerotier-one-dev
apk add qnx-test-zerotier-one && /usr/local/bin/qnx-port-tests/test_zerotier_one    # its on-target test, from tests-8.0-<arch>-m-z
apk del zerotier-one zerotier-one-dev qnx-test-zerotier-one
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zerotier-one.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zerotier-one.list
rm var/lib/manual/zerotier-one.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zerotier-one` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1.16.1 --recurse-submodules --shallow-submodules \
    https://github.com/zerotier/ZeroTierOne.git src/zerotier-one
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zerotier-one < ports/zerotier-one/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/zerotier-one < ports/zerotier-one/patches/0002-qnx-release-cross.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Add QNX support

> Imported from qnx-ports/ZeroTierOne (1.16.1+d9a7f62a5...john_qnx_support:
> b45965ebe..b73eb5c8b, 15 commits); original authorship kept. The two
> upstream commits between the 1.16.1 tag and the branch's base only touch
> rustybits/Cargo.lock, which this build does not use.
>
> make-qnx.mk (modeled on make-bsd.mk) and a Makefile case for OSTYPE=QNX;
> __UNIX_LIKE__ and the __BYTE_ORDER macros for QNX (no <endian.h>); a QNX
> EthernetTap on /dev/tap, configured with ifconfig; QNX in the BSD routing
> paths of ManagedRoute, Phy and libnatpmp; directory listing by stat() where
> struct dirent has no d_type; /dev/urandom reads that retry short reads; ARM
> crypto extensions assumed present on aarch64 (no getauxval on QNX).
>
> ---

Touches: `Makefile`, `ext/libnatpmp/getgateway.c`, `make-qnx.mk`, `node/Constants.hpp`, `node/Utils.cpp`, `osdep/EthernetTap.cpp` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-release-cross.patch`](patches/0002-qnx-release-cross.patch)** — Take OS_STRING's release from QNX_RELEASE when cross-compiling.

> make-qnx.mk compiles `uname -r` into OS_STRING (reported by the service). In a
> cross build that is the build machine's kernel release ("QNX/7.0.0-42-generic"
> on a Linux host); a QNX_RELEASE variable lets the build name the target's.

Touches: `make-qnx.mk` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/zerotier-one; B=build/$QNX_ARCH/zerotier-one; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/"; }
```

```bash
do_build() { _zt_make -j"$(nproc)" one; }
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/sbin" "$STAGE$PREFIX/share/man/man1" "$STAGE$PREFIX/share/man/man8"
  install -m 755 "$B/zerotier-one" "$STAGE$PREFIX/sbin/zerotier-one"
  ln -sf zerotier-one "$STAGE$PREFIX/sbin/zerotier-cli"
  ln -sf zerotier-one "$STAGE$PREFIX/sbin/zerotier-idtool"
  install -m 644 "$B"/doc/zerotier-cli.1 "$B"/doc/zerotier-idtool.1 "$STAGE$PREFIX/share/man/man1/"
  install -m 644 "$B"/doc/zerotier-one.8 "$STAGE$PREFIX/share/man/man8/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ZeroTier One from upstream 1.16.1 with qnx-ports' QNX support (make-qnx.mk, a /dev/tap EthernetTap, BSD routing paths); see patches/0001.
- The Makefile builds in the source tree and picks make-qnx.mk from OSTYPE, which it otherwise takes from the build machine's uname. CC_MACH: make-qnx.mk derives the architecture from `$(CC) -dumpmachine`, which qcc (a driver, not GCC) does not answer. QNX_RELEASE: the target's release for OS_STRING, not the build machine's (0002). STRIP: the QNX cross strip, not the build machine's. The bundled miniupnpc/libnatpmp are used (make-qnx.mk); the Rust-based SSO support and the network controller (non-free) are not built.
- zerotier-cli and zerotier-idtool are the same program under other names (it looks at argv[0]), installed as symlinks as upstream's install does.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zerotier-one`.

### 5. What it installs

6 files under `/usr/local`, 3 programs.

```
/usr/local/sbin/zerotier-cli
/usr/local/sbin/zerotier-idtool
/usr/local/sbin/zerotier-one
```

The complete list is `stage/<arch>/.manifest/zerotier-one` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zerotier-one
ls dists/8.0/*/ | grep '^zerotier-one-'
```

This writes `zerotier-one-<ver>-r0.tar.gz` (run time) and `zerotier-one-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zerotier-one
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zerotier-one-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zerotier-one-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zerotier_one`](../../tests/src/) links zerotier-one the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zerotier-one`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zerotier-one
/usr/local/bin/qnx-port-tests/test_zerotier_one
```

```
== zerotier-one ==
  ok   ...
PASS zerotier-one
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zerotier_one` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zerotier_one`](../../tests/src/) -- a program linked against zerotier-one the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zerotier-one 1.16.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/zerotier-one-1.16.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zerotier-one 1.16.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/zerotier-one-1.16.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.16.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zerotier-one=<version>`), sources in `ported/zerotier-one/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zerotier-one`; port one with `scripts/port-update zerotier-one`.

<details><summary>recipe.sh</summary>

```bash
# ZeroTier One from upstream 1.16.1 with qnx-ports' QNX support (make-qnx.mk,
# a /dev/tap EthernetTap, BSD routing paths); see patches/0001.
VERSION=1.16.1; GIT=https://github.com/zerotier/ZeroTierOne.git; REF=1.16.1
BUILDSYS=custom
# The Makefile builds in the source tree and picks make-qnx.mk from OSTYPE,
# which it otherwise takes from the build machine's uname.
# CC_MACH: make-qnx.mk derives the architecture from `$(CC) -dumpmachine`,
# which qcc (a driver, not GCC) does not answer.
# QNX_RELEASE: the target's release for OS_STRING, not the build machine's (0002).
# STRIP: the QNX cross strip, not the build machine's.
# The bundled miniupnpc/libnatpmp are used (make-qnx.mk); the Rust-based SSO
# support and the network controller (non-free) are not built.
_zt_make() {
  make -C "$B" OSTYPE=QNX CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
    CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" CC_MACH="$QNX_CPU" QNX_RELEASE=8.0.0 \
    STRIP="$QNX_HOST/usr/bin/$QNX_TRIPLE-strip" "$@"
}
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _zt_make -j"$(nproc)" one; }
# zerotier-cli and zerotier-idtool are the same program under other names
# (it looks at argv[0]), installed as symlinks as upstream's install does.
do_install() {
  install -d "$STAGE$PREFIX/sbin" "$STAGE$PREFIX/share/man/man1" "$STAGE$PREFIX/share/man/man8"
  install -m 755 "$B/zerotier-one" "$STAGE$PREFIX/sbin/zerotier-one"
  ln -sf zerotier-one "$STAGE$PREFIX/sbin/zerotier-cli"
  ln -sf zerotier-one "$STAGE$PREFIX/sbin/zerotier-idtool"
  install -m 644 "$B"/doc/zerotier-cli.1 "$B"/doc/zerotier-idtool.1 "$STAGE$PREFIX/share/man/man1/"
  install -m 644 "$B"/doc/zerotier-one.8 "$STAGE$PREFIX/share/man/man8/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
