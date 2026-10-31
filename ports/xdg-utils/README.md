# xdg-utils 1.2.1

> Desktop integration scripts such as xdg-open

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xdg/xdg-utils/-/archive/v1.2.1/xdg-utils-v1.2.1.tar.gz |
| SHA-256 | `f6b648c064464c2636884c05746e80428110a576f8daacf46ef2e554dcfdae75` |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xdg_utils`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `xdg-utils`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xdg-utils`](https://openqnx.github.io/oss/package/x86_64/xdg-utils.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xdg-utils.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xdg-utils` (dependencies come along); one version: `apk add xdg-utils=<version>` | `apk del xdg-utils` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xdg-utils` (dependencies first) | `oss-get remove xdg-utils` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-xdg-utils && /usr/local/bin/qnx-port-tests/test_xdg_utils    # its on-target test
apk del xdg-utils qnx-test-xdg-utils
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xdg-utils.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xdg-utils.list
rm var/lib/manual/xdg-utils.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xdg-utils` performs exactly the same steps (2–5) in one command.

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
URL=https://gitlab.freedesktop.org/xdg/xdg-utils/-/archive/v1.2.1/xdg-utils-v1.2.1.tar.gz
mkdir -p dist src/xdg-utils
curl -fL -o dist/$(basename $URL) $URL
echo "f6b648c064464c2636884c05746e80428110a576f8daacf46ef2e554dcfdae75  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xdg-utils --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xdg-utils < ports/xdg-utils/patches/0001-aports-busybox-sed.patch
patch -p1 -d src/xdg-utils < ports/xdg-utils/patches/0002-qnx-sh-no-char-classes.patch
```

**[`0001-aports-busybox-sed.patch`](patches/0001-aports-busybox-sed.patch)** — QNX change from QNX's aports (extra/xdg-utils/busybox-sed.patch).

> Taken from qnx-ports/aports extra/xdg-utils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> Busybox sed allows } to be a label so we need to add a ;.
>
> Ref https://gitlab.freedesktop.org/xdg/xdg-utils/-/merge_requests/126

Touches: `scripts/xdg-screensaver.in` · Upstream status: QNX-specific

**[`0002-qnx-sh-no-char-classes.patch`](patches/0002-qnx-sh-no-char-classes.patch)** — QNX's sh (pdksh) has no [[:alpha:]] in case patterns: spell the vendor-prefix check [a-zA-Z]

Touches: `scripts/xdg-utils-common.in` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xdg-utils; B=build/$QNX_ARCH/xdg-utils; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && ./configure --prefix="$PREFIX"; }
```

```bash
do_build() { make -C "$B/scripts" scripts XMLTO="python3 $RECIPE_DIR/xdg-utils/refentry2txt.py"; }
```

```bash
do_install() { make -C "$B/scripts" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xdg-utils: xdg-open and friends (shell scripts). Version as in the QNX Everywhere 8.0.4r1 image.
- The Makefiles only work in the source tree (the HTML/man pages are made from desc/*.xml by relative paths); build in a copy of it, scripts only.
- Each script embeds its --help/--manual text, which the Makefile makes with "xmlto txt" from desc/*.xml. xmlto (with the DocBook XSL and a text browser) is rarely on a build machine; refentry2txt.py renders the same text layout with Python alone.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xdg-utils`.

### 5. What it installs

8 files under `/usr/local`, 8 programs.

```
/usr/local/bin/xdg-desktop-icon
/usr/local/bin/xdg-desktop-menu
/usr/local/bin/xdg-email
/usr/local/bin/xdg-icon-resource
/usr/local/bin/xdg-mime
/usr/local/bin/xdg-open
/usr/local/bin/xdg-screensaver
/usr/local/bin/xdg-settings
```

The complete list is `stage/<arch>/.manifest/xdg-utils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xdg-utils
ls dists/8.0/*/ | grep '^xdg-utils-'
```

This writes `xdg-utils-<ver>-r0.tar.gz` (run time) and `xdg-utils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xdg-utils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xdg-utils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xdg-utils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xdg_utils`](../../tests/src/) links xdg-utils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xdg-utils`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xdg-utils
/usr/local/bin/qnx-port-tests/test_xdg_utils
```

```
== xdg-utils ==
  ok   ...
PASS xdg-utils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xdg_utils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xdg_utils`](../../tests/src/) -- a program linked against xdg-utils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xdg-utils 1.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xdg-utils-1.2.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xdg-utils 1.2.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/xdg-utils-1.2.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xdg-utils 1.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xdg-utils-1.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xdg-utils 1.2.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/xdg-utils-1.2.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xdg-utils=<version>`), sources in `ported/xdg-utils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xdg-utils`; port one with `scripts/port-update xdg-utils`.

<details><summary>recipe.sh</summary>

```bash
# xdg-utils: xdg-open and friends (shell scripts).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.2.1
TARBALL=https://gitlab.freedesktop.org/xdg/xdg-utils/-/archive/v1.2.1/xdg-utils-v1.2.1.tar.gz
SHA256=f6b648c064464c2636884c05746e80428110a576f8daacf46ef2e554dcfdae75
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# The Makefiles only work in the source tree (the HTML/man pages are made from
# desc/*.xml by relative paths); build in a copy of it, scripts only.
BUILDSYS=custom
do_configure() { cp -a "$S/." "$B/" && cd "$B" && ./configure --prefix="$PREFIX"; }
# Each script embeds its --help/--manual text, which the Makefile makes with
# "xmlto txt" from desc/*.xml. xmlto (with the DocBook XSL and a text browser)
# is rarely on a build machine; refentry2txt.py renders the same text layout
# with Python alone.
do_build() { make -C "$B/scripts" scripts XMLTO="python3 $RECIPE_DIR/xdg-utils/refentry2txt.py"; }
do_install() { make -C "$B/scripts" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
