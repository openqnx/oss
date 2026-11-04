# hwdata 0.413

> Hardware identification databases (pci.ids, usb.ids, pnp.ids, OUI)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/vcrhonek/hwdata/archive/refs/tags/v0.413.tar.gz |
| SHA-256 | `5a71ab7a48c668ef912fcfdbec046949087019f8dd7fb967445d36fbeb778350` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_hwdata`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `hwdata` (programs, shared libraries, run-time data) and `hwdata-dev` (headers, pkg-config/CMake files, static libraries; needs `hwdata`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`hwdata`](https://openqnx.github.io/oss/package/x86_64/hwdata.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hwdata.html)) · [`hwdata-dev`](https://openqnx.github.io/oss/package/x86_64/hwdata-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/hwdata-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add hwdata` (dependencies come along); one version: `apk add hwdata=<version>` | `apk del hwdata` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install hwdata` (dependencies first) | `oss-get remove hwdata` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`hwdata-dev` the same way, by its own name (`apk add hwdata-dev`, `oss-get install hwdata-dev`, by hand with its own file and list `hwdata-dev.list`); remove it before `hwdata`, which it needs.

```sh
# on the QNX target, as root
apk add hwdata hwdata-dev
apk add qnx-test-hwdata && /usr/local/bin/qnx-port-tests/test_hwdata    # its on-target test, from tests-8.0-<arch>
apk del hwdata hwdata-dev qnx-test-hwdata
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/hwdata.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/hwdata.list
rm var/lib/manual/hwdata.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild hwdata` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/vcrhonek/hwdata/archive/refs/tags/v0.413.tar.gz
mkdir -p dist src/hwdata
curl -fL -o dist/$(basename $URL) $URL
echo "5a71ab7a48c668ef912fcfdbec046949087019f8dd7fb967445d36fbeb778350  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/hwdata --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/hwdata; B=build/$QNX_ARCH/hwdata; mkdir -p $B
```

```bash
do_build() { rm -rf "$B" && cp -a "$S" "$B" && cd "$B" && ./configure --prefix="$PREFIX" --disable-blacklist; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- hwdata: the hardware identification databases -- pci.ids, usb.ids, pnp.ids, the IEEE OUI/IAB registries -- in share/hwdata, as read by lspci/lsusb-style tools and libraries (and hwdata.pc to find them). Data, the same for every architecture.
- Its own configure (not autoconf) writes Makefile.inc. The kernel-module blacklist (lib/modprobe.d) is a Linux modprobe file: left out.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH hwdata`.

### 5. What it installs

6 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/hwdata` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa hwdata
ls dists/8.0/*/ | grep '^hwdata-'
```

This writes `hwdata-<ver>-r0.tar.gz` (run time) and `hwdata-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add hwdata
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/hwdata-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/hwdata-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_hwdata`](../../tests/src/) links hwdata the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-hwdata`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-hwdata
/usr/local/bin/qnx-port-tests/test_hwdata
```

```
== hwdata ==
  ok   ...
PASS hwdata
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_hwdata` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_hwdata`](../../tests/src/) -- a program linked against hwdata the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - pnp.ids: PNP -> Microsoft
   - oui.txt: 00-00-0C -> Cisco Systems

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> hwdata`.

## Versions

This recipe builds **0.413**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add hwdata=<version>`), sources in `ported/hwdata/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates hwdata`; port one with `scripts/port-update hwdata`.

<details><summary>recipe.sh</summary>

```bash
# hwdata: the hardware identification databases -- pci.ids, usb.ids,
# pnp.ids, the IEEE OUI/IAB registries -- in share/hwdata, as read by
# lspci/lsusb-style tools and libraries (and hwdata.pc to find them).
# Data, the same for every architecture.
VERSION=0.413
TARBALL=https://github.com/vcrhonek/hwdata/archive/refs/tags/v0.413.tar.gz
SHA256=5a71ab7a48c668ef912fcfdbec046949087019f8dd7fb967445d36fbeb778350
BUILDSYS=custom
# Its own configure (not autoconf) writes Makefile.inc. The kernel-module
# blacklist (lib/modprobe.d) is a Linux modprobe file: left out.
do_build() { rm -rf "$B" && cp -a "$S" "$B" && cd "$B" && ./configure --prefix="$PREFIX" --disable-blacklist; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
