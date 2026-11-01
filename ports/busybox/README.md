# busybox 1.36.1

> Tiny versions of many common UNIX utilities in a single executable

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/vda-linux/busybox_mirror.git (tag `1_36_1`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_busybox`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `busybox`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`busybox`](https://openqnx.github.io/oss/package/x86_64/busybox.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/busybox.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add busybox` (dependencies come along); one version: `apk add busybox=<version>` | `apk del busybox` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install busybox` (dependencies first) | `oss-get remove busybox` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-busybox && /usr/local/bin/qnx-port-tests/test_busybox    # its on-target test, from tests-8.0-<arch>
apk del busybox qnx-test-busybox
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/busybox.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/busybox.list
rm var/lib/manual/busybox.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild busybox` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 1_36_1 --recurse-submodules --shallow-submodules \
    https://github.com/vda-linux/busybox_mirror.git src/busybox
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/busybox < ports/busybox/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — migrate qnx patch from aports

> Imported from qnx-ports (vda-linux/busybox_mirror 1_36_1...qnx-1_36_1_p0); original authorship kept.
>
>
>
> ---
>  coreutils/ls.c          |  6 +++---
>  coreutils/stat.c        | 17 ++++++++++-------
>  coreutils/uname.c       | 17 +++++++++++++++++
>  include/libbb.h         |  4 ++--
>  include/platform.h      | 20 ++++++++++++++++++++
>  libbb/makedev.c         |  9 ++++++++-
>  libbb/mode_string.c     | 10 +++++++++-
>  scripts/kconfig/mconf.c |  2 +-
>  8 files changed, 70 insertions(+), 15 deletions(-)

Touches: `coreutils/ls.c`, `coreutils/stat.c`, `coreutils/uname.c`, `include/libbb.h`, `include/platform.h`, `libbb/makedev.c` … · Upstream status: QNX-specific (from qnx-ports)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/busybox; B=build/$QNX_ARCH/busybox; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cp "$RECIPE_DIR/busybox/busybox.config" "$B/.config" && { yes "" || :; } | _bbmake oldconfig >/dev/null; }
```

```bash
do_build() { _bbmake busybox; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/busybox" "$STAGE$PREFIX/bin/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- BusyBox 1.36.1 with qnx-ports' QNX support (patches/: the fork's QNX commit), built with the applet configuration of the QNX Everywhere 8.0.4r1 image (busybox.config, from qnx-ports/aports core/busybox).
- oldconfig asks about options the saved .config does not mention; "yes" takes the defaults. yes ends on SIGPIPE when make is done, which under the driver's pipefail failed the whole step, hence the "|| :".

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH busybox`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/busybox
```

The complete list is `stage/<arch>/.manifest/busybox` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa busybox
ls dists/8.0/*/ | grep '^busybox-'
```

This writes `busybox-<ver>-r0.tar.gz` (run time) and `busybox-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add busybox
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/busybox-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/busybox-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_busybox`](../../tests/src/) links busybox the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-busybox`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-busybox
/usr/local/bin/qnx-port-tests/test_busybox
```

```
== busybox ==
  ok   ...
PASS busybox
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_busybox` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_busybox`](../../tests/src/) -- a program linked against busybox the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | busybox 1.36.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/busybox-1.36.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | busybox 1.36.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/busybox-1.36.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | busybox 1.36.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/busybox-1.36.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | busybox 1.36.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/busybox-1.36.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.36.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add busybox=<version>`), sources in `ported/busybox/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates busybox`; port one with `scripts/port-update busybox`.

<details><summary>recipe.sh</summary>

```bash
# BusyBox 1.36.1 with qnx-ports' QNX support (patches/: the fork's QNX
# commit), built with the applet configuration of the QNX Everywhere 8.0.4r1
# image (busybox.config, from qnx-ports/aports core/busybox).
VERSION=1.36.1; GIT=https://github.com/vda-linux/busybox_mirror.git; REF=1_36_1
BUILDSYS=custom
_bbmake() { make -C "$B" -j"$(nproc)" CROSS_COMPILE="$QNX_HOST/usr/bin/nto${QNX_CPU}-" CC="qcc -V$QCC_TARGET" \
  HOSTCC=gcc HOSTLDFLAGS="" LDFLAGS="-Wl,-z,stack-size=4194304" \
  CONFIG_EXTRA_CFLAGS="-D_QNX_SOURCE -DBB_GLOBAL_CONST=''" CONFIG_EXTRA_LDLIBS="regex socket" "$@"; }
# oldconfig asks about options the saved .config does not mention; "yes" takes
# the defaults. yes ends on SIGPIPE when make is done, which under the
# driver's pipefail failed the whole step, hence the "|| :".
do_configure() { cp -a "$S/." "$B/" && cp "$RECIPE_DIR/busybox/busybox.config" "$B/.config" && { yes "" || :; } | _bbmake oldconfig >/dev/null; }
do_build() { _bbmake busybox; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/busybox" "$STAGE$PREFIX/bin/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
