# qnx-socketcan 1.0

> Linux SocketCAN for QNX programs: CAN_RAW sockets, <linux/can.h>, virtual CAN buses

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qnx_socketcan`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `qnx-socketcan` (programs, shared libraries, run-time data) and `qnx-socketcan-dev` (headers, pkg-config/CMake files, static libraries; needs `qnx-socketcan`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`qnx-socketcan`](https://openqnx.github.io/oss/package/x86_64/qnx-socketcan.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qnx-socketcan.html)) · [`qnx-socketcan-dev`](https://openqnx.github.io/oss/package/x86_64/qnx-socketcan-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qnx-socketcan-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add qnx-socketcan` (dependencies come along); one version: `apk add qnx-socketcan=<version>` | `apk del qnx-socketcan` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install qnx-socketcan` (dependencies first) | `oss-get remove qnx-socketcan` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`qnx-socketcan-dev` the same way, by its own name (`apk add qnx-socketcan-dev`, `oss-get install qnx-socketcan-dev`, by hand with its own file and list `qnx-socketcan-dev.list`); remove it before `qnx-socketcan`, which it needs.

```sh
# on the QNX target, as root
apk add qnx-socketcan qnx-socketcan-dev
apk add qnx-test-qnx-socketcan && /usr/local/bin/qnx-port-tests/test_qnx_socketcan    # its on-target test, from tests-8.0-<arch>-m-z
apk del qnx-socketcan qnx-socketcan-dev qnx-test-qnx-socketcan
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/qnx-socketcan.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/qnx-socketcan.list
rm var/lib/manual/qnx-socketcan.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qnx-socketcan` performs exactly the same steps (2–5) in one command.

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

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-socketcan/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qnx-socketcan; B=build/$QNX_ARCH/qnx-socketcan; mkdir -p $B
```

```bash
do_build() {
  qs_cc -fPIC -shared -Wl,-soname,libqsocketcan.so.1 "$S/src/qsocketcan.c" -o "$B/libqsocketcan.so.1" -lsocket || return 1
  qs_cc "$S/src/qcand.c" -o "$B/qcand" -lsocket || return 1
  qs_cc "$S/src/qcanctl.c" -o "$B/qcanctl" -lsocket || return 1
}
```

```bash
do_install() {
  local inc="$STAGE$PREFIX/include/qnx-socketcan/linux"
  install -Dm755 "$B/libqsocketcan.so.1" "$STAGE$PREFIX/lib/libqsocketcan.so.1" || return 1
  ln -sf libqsocketcan.so.1 "$STAGE$PREFIX/lib/libqsocketcan.so"
  install -Dm755 "$B/qcand" "$STAGE$PREFIX/bin/qcand" || return 1
  install -Dm755 "$B/qcanctl" "$STAGE$PREFIX/bin/qcanctl" || return 1
  install -d "$inc/can" "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/share/licenses/qnx-socketcan"
  install -m644 "$S"/include/linux/*.h "$inc/" || return 1
  install -m644 "$S"/include/linux/can/*.h "$inc/can/" || return 1
  sed -e "s|@PREFIX@|$PREFIX|" -e "s|@VERSION@|$VERSION|" "$S/qnx-socketcan.pc.in" > "$STAGE$PREFIX/lib/pkgconfig/qnx-socketcan.pc"
  install -m644 "$S/BSD-3-Clause" "$S"/*.txt "$STAGE$PREFIX/share/licenses/qnx-socketcan/" || return 1
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- qnx-socketcan: Linux SocketCAN for programs on QNX 8.0. QNX has no SocketCAN: io-sock has no PF_CAN protocol family and there is no <linux/can.h>; QNX's CAN is a resource manager per controller (/dev/canN/..., devctl()s) whose drivers come with board BSPs. Programs written for SocketCAN (can-utils, python-can, ROS 2 CAN drivers, ...) open socket(PF_CAN, SOCK_RAW, CAN_RAW) and read and write struct can_frame. This component (its own source: files/) gives them that API in user space: libqsocketcan.so   defines socket(), bind(), setsockopt(), read(), write(), recvmsg(), ioctl(), if_nametoindex(), ...: CAN sockets are served here, every other descriptor goes on to libsocket and libc qcand              the CAN bus daemon: virtual CAN interfaces (as Linux's vcan; default vcan0, CAN FD capable), frames from one socket to the others with CAN_RAW's filters, loopback, own-message and error-mask rules qcanctl            list, add, remove interfaces (ip link ... type vcan) <linux/can.h>, <linux/can/raw.h>, <linux/can/error.h>  the Linux UAPI headers (from can-utils' copy of them), with the <linux/types.h>, <linux/socket.h>, <linux/stddef.h> they include -- in include/qnx-socketcan/, off the default include path: `pkg-config qnx-socketcan` Link order: -lqsocketcan before -lsocket (pkg-config does that). Builds that name -lc ahead of it (this workspace's CMake toolchain) work too: a message on a CAN socket's connection to qcand is the CAN frame as it is, so libc's read()/write() carry frames, and <linux/socket.h> redirects ioctl(); what differs then is listed in files/src/qsocketcan.c. A real PF_CAN protocol family is out of reach: SDP 8.0 has no public <sys/sockmsg.h> to serve /dev/socket/<af> with. Scope: CAN_RAW with classic CAN and CAN FD frames on virtual interfaces. Not there: CAN_BCM, CAN_ISOTP, CAN_J1939 (Linux kernel protocols), CAN XL, and a bridge to a hardware controller's /dev/canN (docs/STATUS.md).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qnx-socketcan`.

### 5. What it installs

15 files under `/usr/local`: 2 shared-library entries, 2 programs, 7 headers.

```
/usr/local/lib/libqsocketcan.so
/usr/local/lib/libqsocketcan.so.1
/usr/local/bin/qcanctl
/usr/local/bin/qcand
```

The complete list is `stage/<arch>/.manifest/qnx-socketcan` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qnx-socketcan
ls dists/8.0/*/ | grep '^qnx-socketcan-'
```

This writes `qnx-socketcan-<ver>-r0.tar.gz` (run time) and `qnx-socketcan-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qnx-socketcan
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qnx-socketcan-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qnx-socketcan-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qnx_socketcan`](../../tests/src/) links qnx-socketcan the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qnx-socketcan`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qnx-socketcan
/usr/local/bin/qnx-port-tests/test_qnx_socketcan
```

```
== qnx-socketcan ==
  ok   ...
PASS qnx-socketcan
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qnx_socketcan` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qnx_socketcan`](../../tests/src/) -- a program linked against qnx-socketcan the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - no daemon: socket(PF_CAN) fails with ENETDOWN
   - qcand is up; if_nametoindex finds tcan0 and tcan1
   - if_indextoname
   - if_nametoindex still finds lo0, and no nocan9
   - CAN_BCM: EPROTONOSUPPORT
   - three CAN_RAW sockets bound (two on tcan0, one on tcan1)
   - bind to an unknown interface index: ENODEV
   - ioctl SIOCGIFINDEX
   - tcan0 MTU is 72 (CAN FD)
   - tcan1 MTU is 16 (classic)
   - SIOCGIFINDEX of an unknown name: ENODEV
   - write a classic frame (16 bytes)
   - the other socket of the bus reads 123#DEADBEEF
   - the sender does not receive its own frame
   - the socket on the other bus receives nothing
   - SIOCGSTAMP: the frame's time
   - an extended frame keeps its 29-bit id and flag
   - a remote frame
   - setsockopt CAN_RAW_RECV_OWN_MSGS, SO_TIMESTAMP
   - recvmsg: the own frame, flagged MSG_CONFIRM, from tcan0
   - recvmsg: an SCM_TIMESTAMP control message with the current time
   - the other socket gets it without MSG_CONFIRM
   - getsockopt: RECV_OWN_MSGS 0, LOOPBACK 1 (the default)
   - setsockopt CAN_RAW_FILTER (200/700, 555/7FF)
   - of 123 205 554 555 2FF the filter passes 205 555 2FF
   - ... and 32 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qnx-socketcan 1.0-r0 | ❌ FAIL | 2026-11-11 | [log](../../tests/logs/aarch64le/qnx-socketcan-1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qnx-socketcan 1.0-r1 | ❌ FAIL | 2026-11-11 | [log](../../tests/logs/aarch64le/qnx-socketcan-1.0-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qnx-socketcan 1.0-r2 | ✅ PASS | 2026-11-11 | [log](../../tests/logs/aarch64le/qnx-socketcan-1.0-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qnx-socketcan 1.0-r0 | ❌ FAIL | 2026-11-11 | [log](../../tests/logs/x86_64/qnx-socketcan-1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qnx-socketcan 1.0-r1 | ❌ FAIL | 2026-11-11 | [log](../../tests/logs/x86_64/qnx-socketcan-1.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qnx-socketcan 1.0-r2 | ✅ PASS | 2026-11-11 | [log](../../tests/logs/x86_64/qnx-socketcan-1.0-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qnx-socketcan=<version>`), sources in `ported/qnx-socketcan/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qnx-socketcan`; port one with `scripts/port-update qnx-socketcan`.

<details><summary>recipe.sh</summary>

```bash
# qnx-socketcan: Linux SocketCAN for programs on QNX 8.0.
#
# QNX has no SocketCAN: io-sock has no PF_CAN protocol family and there is
# no <linux/can.h>; QNX's CAN is a resource manager per controller
# (/dev/canN/..., devctl()s) whose drivers come with board BSPs. Programs
# written for SocketCAN (can-utils, python-can, ROS 2 CAN drivers, ...) open
# socket(PF_CAN, SOCK_RAW, CAN_RAW) and read and write struct can_frame.
# This component (its own source: files/) gives them that API in user space:
#
#   libqsocketcan.so   defines socket(), bind(), setsockopt(), read(),
#                      write(), recvmsg(), ioctl(), if_nametoindex(), ...:
#                      CAN sockets are served here, every other descriptor
#                      goes on to libsocket and libc
#   qcand              the CAN bus daemon: virtual CAN interfaces (as Linux's
#                      vcan; default vcan0, CAN FD capable), frames from one
#                      socket to the others with CAN_RAW's filters, loopback,
#                      own-message and error-mask rules
#   qcanctl            list, add, remove interfaces (ip link ... type vcan)
#   <linux/can.h>, <linux/can/raw.h>, <linux/can/error.h>  the Linux UAPI
#                      headers (from can-utils' copy of them), with the
#                      <linux/types.h>, <linux/socket.h>, <linux/stddef.h>
#                      they include -- in include/qnx-socketcan/, off the
#                      default include path: `pkg-config qnx-socketcan`
#
# Link order: -lqsocketcan before -lsocket (pkg-config does that). Builds
# that name -lc ahead of it (this workspace's CMake toolchain) work too: a
# message on a CAN socket's connection to qcand is the CAN frame as it is,
# so libc's read()/write() carry frames, and <linux/socket.h> redirects
# ioctl(); what differs then is listed in files/src/qsocketcan.c. A real
# PF_CAN protocol family is out of reach: SDP 8.0 has no public
# <sys/sockmsg.h> to serve /dev/socket/<af> with.
#
# Scope: CAN_RAW with classic CAN and CAN FD frames on virtual interfaces.
# Not there: CAN_BCM, CAN_ISOTP, CAN_J1939 (Linux kernel protocols), CAN XL,
# and a bridge to a hardware controller's /dev/canN (docs/STATUS.md).
VERSION=1.0
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-socketcan/files/." "$SRCROOT/"; }
qs_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -DQCAN_NO_IOCTL_REDIRECT -O2 -Wall -Wno-missing-braces -I"$S/include" -I"$S/src" "$@"; }
do_build() {
  qs_cc -fPIC -shared -Wl,-soname,libqsocketcan.so.1 "$S/src/qsocketcan.c" -o "$B/libqsocketcan.so.1" -lsocket || return 1
  qs_cc "$S/src/qcand.c" -o "$B/qcand" -lsocket || return 1
  qs_cc "$S/src/qcanctl.c" -o "$B/qcanctl" -lsocket || return 1
}
do_install() {
  local inc="$STAGE$PREFIX/include/qnx-socketcan/linux"
  install -Dm755 "$B/libqsocketcan.so.1" "$STAGE$PREFIX/lib/libqsocketcan.so.1" || return 1
  ln -sf libqsocketcan.so.1 "$STAGE$PREFIX/lib/libqsocketcan.so"
  install -Dm755 "$B/qcand" "$STAGE$PREFIX/bin/qcand" || return 1
  install -Dm755 "$B/qcanctl" "$STAGE$PREFIX/bin/qcanctl" || return 1
  install -d "$inc/can" "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/share/licenses/qnx-socketcan"
  install -m644 "$S"/include/linux/*.h "$inc/" || return 1
  install -m644 "$S"/include/linux/can/*.h "$inc/can/" || return 1
  sed -e "s|@PREFIX@|$PREFIX|" -e "s|@VERSION@|$VERSION|" "$S/qnx-socketcan.pc.in" > "$STAGE$PREFIX/lib/pkgconfig/qnx-socketcan.pc"
  install -m644 "$S/BSD-3-Clause" "$S"/*.txt "$STAGE$PREFIX/share/licenses/qnx-socketcan/" || return 1
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
