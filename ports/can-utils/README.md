# can-utils 2025.01

> SocketCAN tools: candump, cansend, cangen, canplayer, cansniffer, canbusload, log converters

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/linux-can/can-utils.git (tag `v2025.01`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`qnx-socketcan`](../qnx-socketcan/) |
| Run-time dependencies | [`qnx-socketcan`](../qnx-socketcan/) |
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

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild qnx-socketcan
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2025.01 --recurse-submodules --shallow-submodules \
    https://github.com/linux-can/can-utils.git src/can-utils
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/can-utils < ports/can-utils/patches/0001-qnx-epoll-and-libc-compat.patch
patch -p1 -d src/can-utils < ports/can-utils/patches/0002-calc-bit-timing-min-max.patch
```

**[`0001-qnx-epoll-and-libc-compat.patch`](patches/0001-qnx-epoll-and-libc-compat.patch)** — QNX: epoll on poll() for candump, and the libc bits Linux has.

> candump waits for its CAN sockets with epoll, which QNX does not have:
> qnx/sys/epoll.h (on the include path, set by the recipe) implements the
> epoll_create/epoll_ctl/epoll_wait candump uses on poll(). qnx/qnx-compat.h
> (included ahead of every source, -include) gives cangen a CLOCK_TAI (QNX's
> monotonic clock) and canbusload MIN/MAX, which <sys/param.h> does not
> define under _QNX_SOURCE.
>
> The Linux CAN socket API itself (PF_CAN sockets, <linux/types.h>,
> <linux/socket.h>, <linux/sockios.h>) comes from qnx-socketcan.

Touches: `qnx/qnx-compat.h`, `qnx/sys/epoll.h` · Upstream status: QNX-specific

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
  local t
  for t in $CU_SOCK_TOOLS; do
    cu_cc "$S/$t.c" "$S/lib.c" "$S/canframelen.c" -o "$B/$t" \
          -L"$STAGE$PREFIX/lib" -Wl,-rpath-link,"$STAGE$PREFIX/lib" -lqsocketcan -lsocket -lm || return 1
  done
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

- can-utils, the Linux SocketCAN user-space tools. QNX 8.0 has no SocketCAN of its own (io-sock has no PF_CAN; QNX's CAN is a resource manager per controller). The tools here run on qnx-socketcan: its libqsocketcan gives them socket(PF_CAN, SOCK_RAW, CAN_RAW) and <linux/can.h>, and its daemon qcand the CAN buses (virtual ones, as Linux's vcan: start `qcand`, then `candump vcan0`, `cansend vcan0 123#11`). Built -- the CAN_RAW tools and the ones that work on CAN data: candump, cansend, cangen, canplayer, cansniffer, canbusload, cansequence, canfdtest, canlogserver asc2log, log2asc, log2long    candump log files <-> Vector ASC, long form can-calc-bit-timing           bit-timing registers for CAN controllers Not built: what needs a Linux kernel protocol or driver that qnx-socketcan does not provide -- bcmserver, cangw (CAN_BCM, CAN gateway via netlink), isotp* (CAN_ISOTP), j1939*/testj1939/isobusfs (CAN_J1939), slcan* (the slcan tty line discipline), mcp251xfd-dump.
- Include path: qnx/ (the patch: <sys/epoll.h> on poll()), qnx-socketcan's <linux/types.h>, <linux/socket.h>, <linux/sockios.h>, then the Linux UAPI CAN headers can-utils ships (include/linux/can.h, ...). libqsocketcan goes before libsocket: it defines socket(), bind(), read(), ... for CAN sockets and passes every other descriptor on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH can-utils`.

### 5. What it installs

13 files under `/usr/local`, 13 programs.

```
/usr/local/bin/asc2log
/usr/local/bin/can-calc-bit-timing
/usr/local/bin/canbusload
/usr/local/bin/candump
/usr/local/bin/canfdtest
/usr/local/bin/cangen
/usr/local/bin/canlogserver
/usr/local/bin/canplayer
/usr/local/bin/cansend
/usr/local/bin/cansequence
/usr/local/bin/cansniffer
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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | can-utils 2025.01-r1 | ✅ PASS | 2026-11-11 | [log](../../tests/logs/aarch64le/can-utils-2025.01-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | can-utils 2025.01-r2 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/can-utils-2025.01-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | can-utils 2025.01-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/can-utils-2025.01-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | can-utils 2025.01-r1 | ✅ PASS | 2026-11-11 | [log](../../tests/logs/x86_64/can-utils-2025.01-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | can-utils 2025.01-r2 | ✅ PASS | 2026-11-11 | [log](../../tests/logs/x86_64/can-utils-2025.01-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2025.01**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add can-utils=<version>`), sources in `ported/can-utils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates can-utils`; port one with `scripts/port-update can-utils`.

<details><summary>recipe.sh</summary>

```bash
# can-utils, the Linux SocketCAN user-space tools.
#
# QNX 8.0 has no SocketCAN of its own (io-sock has no PF_CAN; QNX's CAN is a
# resource manager per controller). The tools here run on qnx-socketcan:
# its libqsocketcan gives them socket(PF_CAN, SOCK_RAW, CAN_RAW) and
# <linux/can.h>, and its daemon qcand the CAN buses (virtual ones, as
# Linux's vcan: start `qcand`, then `candump vcan0`, `cansend vcan0 123#11`).
#
# Built -- the CAN_RAW tools and the ones that work on CAN data:
#   candump, cansend, cangen, canplayer, cansniffer, canbusload,
#   cansequence, canfdtest, canlogserver
#   asc2log, log2asc, log2long    candump log files <-> Vector ASC, long form
#   can-calc-bit-timing           bit-timing registers for CAN controllers
# Not built: what needs a Linux kernel protocol or driver that qnx-socketcan
# does not provide -- bcmserver, cangw (CAN_BCM, CAN gateway via netlink),
# isotp* (CAN_ISOTP), j1939*/testj1939/isobusfs (CAN_J1939), slcan*
# (the slcan tty line discipline), mcp251xfd-dump.
VERSION=2025.01; GIT=https://github.com/linux-can/can-utils.git; REF=v2025.01
DEPS="qnx-socketcan"
BUILDSYS=custom
CU_SOCK_TOOLS="candump cansend cangen canplayer cansniffer canbusload cansequence canfdtest canlogserver"
CU_TOOLS="$CU_SOCK_TOOLS asc2log log2asc log2long can-calc-bit-timing"
# Include path: qnx/ (the patch: <sys/epoll.h> on poll()), qnx-socketcan's
# <linux/types.h>, <linux/socket.h>, <linux/sockios.h>, then the Linux UAPI
# CAN headers can-utils ships (include/linux/can.h, ...). libqsocketcan
# goes before libsocket: it defines socket(), bind(), read(), ... for CAN
# sockets and passes every other descriptor on.
cu_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -D_GNU_SOURCE -O2 -Wall \
            -include "$S/qnx/qnx-compat.h" -I"$S/qnx" -I"$STAGE$PREFIX/include/qnx-socketcan" -I"$S/include" "$@"; }
do_build() {
  local t
  for t in $CU_SOCK_TOOLS; do
    cu_cc "$S/$t.c" "$S/lib.c" "$S/canframelen.c" -o "$B/$t" \
          -L"$STAGE$PREFIX/lib" -Wl,-rpath-link,"$STAGE$PREFIX/lib" -lqsocketcan -lsocket -lm || return 1
  done
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
