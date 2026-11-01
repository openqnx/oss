# can-utils 2025.01

> CAN log converters (asc2log, log2asc, log2long), can-calc-bit-timing

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/linux-can/can-utils.git (tag `v2025.01`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_can_utils`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `can-utils`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`can-utils`](https://openqnx.github.io/oss/package/x86_64/can-utils.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/can-utils.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add can-utils` (dependencies come along); one version: `apk add can-utils=<version>` | `apk del can-utils` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install can-utils` (dependencies first) | `oss-get remove can-utils` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-can-utils && /usr/local/bin/qnx-port-tests/test_can_utils    # its on-target test, from tests-8.0-<arch>
apk del can-utils qnx-test-can-utils
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/can-utils.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/can-utils.list
rm var/lib/manual/can-utils.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild can-utils` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v2025.01 --recurse-submodules --shallow-submodules \
    https://github.com/linux-can/can-utils.git src/can-utils
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/can-utils < ports/can-utils/patches/0001-qnx-linux-uapi-types.patch
patch -p1 -d src/can-utils < ports/can-utils/patches/0002-calc-bit-timing-min-max.patch
```

**[`0001-qnx-linux-uapi-types.patch`](patches/0001-qnx-linux-uapi-types.patch)** — QNX: the Linux UAPI types the shipped CAN headers need.

> can-utils builds against its own copies of the Linux UAPI headers
> (include/linux/can.h, ...), which include <linux/types.h>, <linux/socket.h>
> and <linux/stddef.h>. QNX has no <linux/...> headers: qnx/linux/ (put on the
> include path by the recipe) defines the kernel's __u8/__u32/... types from
> <stdint.h> and maps the other two onto QNX's <sys/socket.h> and <stddef.h>.
> Only the CAN frame definitions are used; nothing opens a CAN socket.

Touches: `qnx/linux/socket.h`, `qnx/linux/stddef.h`, `qnx/linux/types.h` · Upstream status: QNX-specific

**[`0002-calc-bit-timing-min-max.patch`](patches/0002-calc-bit-timing-min-max.patch)** — can-calc-bit-timing: replace QNX's min()/max() macros.

> QNX's <stdlib.h> defines untyped min()/max() macros; compat.h then
> redefines them (with its type-checked versions), which GCC warns about.
> #undef them first so compat.h's definitions are the ones in effect, as on
> Linux.

Touches: `calc-bit-timing/compat.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/can-utils; B=build/$QNX_ARCH/can-utils; mkdir -p $B
```

```bash
do_build() {
  cu_cc "$S/asc2log.c"  "$S/lib.c" "$S/canframelen.c" -o "$B/asc2log"  || return 1
  cu_cc "$S/log2asc.c"  "$S/lib.c" "$S/canframelen.c" -o "$B/log2asc"  || return 1
  cu_cc "$S/log2long.c" "$S/lib.c" "$S/canframelen.c" -o "$B/log2long" || return 1
  cu_cc -I"$S/calc-bit-timing" "$S/calc-bit-timing/can-calc-bit-timing.c" -o "$B/can-calc-bit-timing"
}
```

```bash
do_install() {
  local t
  install -d "$STAGE$PREFIX/bin"
  for t in $CU_TOOLS; do install -m 755 "$B/$t" "$STAGE$PREFIX/bin/$t" || return 1; done
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- can-utils, the Linux SocketCAN user-space tools -- only the ones that do not need a CAN socket. QNX 8.0 has no SocketCAN: io-sock has no PF_CAN / AF_CAN protocol family (<sys/socket.h> stops at AF_DIVERT; no <linux/can.h>, no can interfaces), and QNX's CAN stack is a resource manager instead (dev-can-* drivers, /dev/canN/{rx,tx}M, devctl()s from <sys/can_dcmd.h>, canctl) whose drivers come with board BSPs -- none in the SDP. candump, cansend, cangen, canplayer, the ISO-TP and J1939 tools all open socket(PF_CAN, ...), so they cannot run on QNX and are not built. What is built works on CAN data, not on a CAN bus: asc2log, log2asc   convert between candump log files and Vector ASC log2long           print a candump log in the long (candump -ta) form can-calc-bit-timing  bit-timing registers for CAN controllers from a bitrate and a clock (no device access)
- The tools include the Linux UAPI headers can-utils ships (include/linux/ can.h, ...), which need <linux/types.h>, <linux/socket.h> and <linux/stddef.h>; qnx/linux/ (added by the patch) maps those onto QNX's own types. Nothing here opens a socket.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH can-utils`.

### 5. What it installs

4 files under `/usr/local`, 4 programs.

```
/usr/local/bin/asc2log
/usr/local/bin/can-calc-bit-timing
/usr/local/bin/log2asc
/usr/local/bin/log2long
```

The complete list is `stage/<arch>/.manifest/can-utils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa can-utils
ls dists/8.0/*/ | grep '^can-utils-'
```

This writes `can-utils-<ver>-r0.tar.gz` (run time) and `can-utils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add can-utils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/can-utils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/can-utils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_can_utils`](../../tests/src/) links can-utils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-can-utils`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-can-utils
/usr/local/bin/qnx-port-tests/test_can_utils
```

```
== can-utils ==
  ok   ...
PASS can-utils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_can_utils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_can_utils`](../../tests/src/) -- a program linked against can-utils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | can-utils 2025.01-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/can-utils-2025.01-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | can-utils 2025.01-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/can-utils-2025.01-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2025.01**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add can-utils=<version>`), sources in `ported/can-utils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates can-utils`; port one with `scripts/port-update can-utils`.

<details><summary>recipe.sh</summary>

```bash
# can-utils, the Linux SocketCAN user-space tools -- only the ones that do
# not need a CAN socket. QNX 8.0 has no SocketCAN: io-sock has no PF_CAN /
# AF_CAN protocol family (<sys/socket.h> stops at AF_DIVERT; no <linux/can.h>,
# no can interfaces), and QNX's CAN stack is a resource manager instead
# (dev-can-* drivers, /dev/canN/{rx,tx}M, devctl()s from <sys/can_dcmd.h>,
# canctl) whose drivers come with board BSPs -- none in the SDP. candump,
# cansend, cangen, canplayer, the ISO-TP and J1939 tools all open
# socket(PF_CAN, ...), so they cannot run on QNX and are not built.
# What is built works on CAN data, not on a CAN bus:
#   asc2log, log2asc   convert between candump log files and Vector ASC
#   log2long           print a candump log in the long (candump -ta) form
#   can-calc-bit-timing  bit-timing registers for CAN controllers from a
#                      bitrate and a clock (no device access)
VERSION=2025.01; GIT=https://github.com/linux-can/can-utils.git; REF=v2025.01
BUILDSYS=custom
CU_TOOLS="asc2log log2asc log2long can-calc-bit-timing"
# The tools include the Linux UAPI headers can-utils ships (include/linux/
# can.h, ...), which need <linux/types.h>, <linux/socket.h> and
# <linux/stddef.h>; qnx/linux/ (added by the patch) maps those onto QNX's
# own types. Nothing here opens a socket.
cu_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall \
            -I"$S/qnx" -I"$S/include" "$@"; }
do_build() {
  cu_cc "$S/asc2log.c"  "$S/lib.c" "$S/canframelen.c" -o "$B/asc2log"  || return 1
  cu_cc "$S/log2asc.c"  "$S/lib.c" "$S/canframelen.c" -o "$B/log2asc"  || return 1
  cu_cc "$S/log2long.c" "$S/lib.c" "$S/canframelen.c" -o "$B/log2long" || return 1
  cu_cc -I"$S/calc-bit-timing" "$S/calc-bit-timing/can-calc-bit-timing.c" -o "$B/can-calc-bit-timing"
}
do_install() {
  local t
  install -d "$STAGE$PREFIX/bin"
  for t in $CU_TOOLS; do install -m 755 "$B/$t" "$STAGE$PREFIX/bin/$t" || return 1; done
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
