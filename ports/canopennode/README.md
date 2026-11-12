# canopennode 4.0

> CANopen protocol stack with the canopend device and the cocomm command client (CiA 301, 305, 309-3)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/CANopenNode/CANopenLinux.git (tag `v4.0`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`qnx-socketcan`](../qnx-socketcan/) |
| Run-time dependencies | [`qnx-socketcan`](../qnx-socketcan/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_canopennode`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `canopennode`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`canopennode`](https://openqnx.github.io/oss/package/x86_64/canopennode.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/canopennode.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add canopennode` (dependencies come along); one version: `apk add canopennode=<version>` | `apk del canopennode` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install canopennode` (dependencies first) | `oss-get remove canopennode` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-canopennode && /usr/local/bin/qnx-port-tests/test_canopennode    # its on-target test, from tests-8.0-<arch>
apk del canopennode qnx-test-canopennode
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/canopennode.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/canopennode.list
rm var/lib/manual/canopennode.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild canopennode` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v4.0 --recurse-submodules --shallow-submodules \
    https://github.com/CANopenNode/CANopenLinux.git src/canopennode
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/canopennode < ports/canopennode/patches/0001-qnx.patch
```

**[`0001-qnx.patch`](patches/0001-qnx.patch)** — QNX has no linux/net_tstamp.h, asm/socket.h, endian.h, linux/reboot.h, bits/getopt_core.h, EPOLLONESHOT or HOST_NAME_MAX, and its getopt() does not permute: take the receive time from SO_TIMESTAMP (qnx-socketcan), define the byte order, re-arm the command listener by hand, hand a reboot to shutdown, and accept the device name before the options.

Touches: `cocomm/cocomm.c`, `CO_driver.c`, `CO_driver_target.h`, `CO_epoll_interface.c`, `CO_main_basic.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/canopennode; B=build/$QNX_ARCH/canopennode; mkdir -p $B
```

```bash
do_build() {
  mkdir -p "$B/obj"
  local f o objs=""
  for f in $CO_SRCS; do
    o="$B/obj/$(echo "$f" | tr / _).o"; objs="$objs $o"
    co_cc -DCO_SINGLE_THREAD -I"$S" -I"$S/CANopenNode" -I"$S/CANopenNode/example" \
          -I"$STAGE$PREFIX/include/qnx-socketcan" -c "$S/$f" -o "$o" || return 1
  done
  # libqsocketcan before libsocket: it defines socket(), bind(), ... for CAN sockets.
  co_cc $objs -o "$B/canopend" -L"$STAGE$PREFIX/lib" -lqsocketcan -lsocket -lepoll -leventfd -ltimerfd || return 1
  co_cc -I"$STAGE$PREFIX/include/qnx-socketcan" "$S/cocomm/cocomm.c" -o "$B/cocomm" -L"$STAGE$PREFIX/lib" -lqsocketcan -lsocket
}
```

```bash
do_install() {
  mkdir -p "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/canopennode" "$STAGE$PREFIX/share/licenses/canopennode"
  cp "$B/canopend" "$B/cocomm" "$STAGE$PREFIX/bin/"
  cp "$S/CANopenNode/example/DS301_profile.eds" "$S/CANopenNode/example/DS301_profile.md" "$STAGE$PREFIX/share/canopennode/"
  cp "$S/LICENSE" "$STAGE$PREFIX/share/licenses/canopennode/LICENSE"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- CANopenNode: the CANopen protocol stack (CiA 301, 303-3, 304, 305, 309-3), as its Linux application CANopenLinux: canopend, a CANopen device with the example object dictionary (NMT, heartbeat, SDO server and client, PDO, SYNC, TIME, emergency, LSS master and slave) and the CiA 309-3 ASCII command interface (stdio, local socket or TCP), and cocomm, the client of that interface. Runs on qnx-socketcan (PF_CAN raw sockets; buses from its daemon qcand): `canopend vcan0 -i 4 -c local-/tmp/CO_command_socket`, then `cocomm "1 r 0x1017 0 u16"`. epoll, eventfd and timerfd are QNX's libepoll, libeventfd and libtimerfd. The source is CANopenLinux v4.0 with the CANopenNode commit it pins.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH canopennode`.

### 5. What it installs

5 files under `/usr/local`, 2 programs.

```
/usr/local/bin/canopend
/usr/local/bin/cocomm
```

The complete list is `stage/<arch>/.manifest/canopennode` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa canopennode
ls dists/8.0/*/ | grep '^canopennode-'
```

This writes `canopennode-<ver>-r0.tar.gz` (run time) and `canopennode-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add canopennode
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/canopennode-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/canopennode-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_canopennode`](../../tests/src/) links canopennode the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-canopennode`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-canopennode
/usr/local/bin/qnx-port-tests/test_canopennode
```

```
== canopennode ==
  ok   ...
PASS canopennode
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_canopennode` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_canopennode`](../../tests/src/) -- a program linked against canopennode the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | canopennode 4.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/canopennode-4.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | canopennode 4.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/canopennode-4.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add canopennode=<version>`), sources in `ported/canopennode/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates canopennode`; port one with `scripts/port-update canopennode`.

<details><summary>recipe.sh</summary>

```bash
# CANopenNode: the CANopen protocol stack (CiA 301, 303-3, 304, 305, 309-3),
# as its Linux application CANopenLinux: canopend, a CANopen device with the
# example object dictionary (NMT, heartbeat, SDO server and client, PDO,
# SYNC, TIME, emergency, LSS master and slave) and the CiA 309-3 ASCII
# command interface (stdio, local socket or TCP), and cocomm, the client
# of that interface.
#
# Runs on qnx-socketcan (PF_CAN raw sockets; buses from its daemon qcand):
# `canopend vcan0 -i 4 -c local-/tmp/CO_command_socket`, then
# `cocomm "1 r 0x1017 0 u16"`. epoll, eventfd and timerfd are QNX's
# libepoll, libeventfd and libtimerfd.
# The source is CANopenLinux v4.0 with the CANopenNode commit it pins.
VERSION=4.0; GIT=https://github.com/CANopenNode/CANopenLinux.git; REF=v4.0
DEPS="qnx-socketcan"
RUNTIME_DEPS="qnx-socketcan"
BUILDSYS=custom
CO_SRCS="CO_driver.c CO_error.c CO_epoll_interface.c CO_storageLinux.c CO_main_basic.c
  CANopenNode/301/CO_ODinterface.c CANopenNode/301/CO_NMT_Heartbeat.c CANopenNode/301/CO_HBconsumer.c
  CANopenNode/301/CO_Emergency.c CANopenNode/301/CO_SDOserver.c CANopenNode/301/CO_SDOclient.c
  CANopenNode/301/CO_TIME.c CANopenNode/301/CO_SYNC.c CANopenNode/301/CO_PDO.c
  CANopenNode/301/crc16-ccitt.c CANopenNode/301/CO_fifo.c CANopenNode/303/CO_LEDs.c
  CANopenNode/304/CO_GFC.c CANopenNode/304/CO_SRDO.c CANopenNode/305/CO_LSSslave.c
  CANopenNode/305/CO_LSSmaster.c CANopenNode/309/CO_gateway_ascii.c CANopenNode/storage/CO_storage.c
  CANopenNode/extra/CO_trace.c CANopenNode/CANopen.c CANopenNode/example/OD.c"
co_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -g -Wall "$@"; }
do_build() {
  mkdir -p "$B/obj"
  local f o objs=""
  for f in $CO_SRCS; do
    o="$B/obj/$(echo "$f" | tr / _).o"; objs="$objs $o"
    co_cc -DCO_SINGLE_THREAD -I"$S" -I"$S/CANopenNode" -I"$S/CANopenNode/example" \
          -I"$STAGE$PREFIX/include/qnx-socketcan" -c "$S/$f" -o "$o" || return 1
  done
  # libqsocketcan before libsocket: it defines socket(), bind(), ... for CAN sockets.
  co_cc $objs -o "$B/canopend" -L"$STAGE$PREFIX/lib" -lqsocketcan -lsocket -lepoll -leventfd -ltimerfd || return 1
  co_cc -I"$STAGE$PREFIX/include/qnx-socketcan" "$S/cocomm/cocomm.c" -o "$B/cocomm" -L"$STAGE$PREFIX/lib" -lqsocketcan -lsocket
}
do_install() {
  mkdir -p "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/canopennode" "$STAGE$PREFIX/share/licenses/canopennode"
  cp "$B/canopend" "$B/cocomm" "$STAGE$PREFIX/bin/"
  cp "$S/CANopenNode/example/DS301_profile.eds" "$S/CANopenNode/example/DS301_profile.md" "$STAGE$PREFIX/share/canopennode/"
  cp "$S/LICENSE" "$STAGE$PREFIX/share/licenses/canopennode/LICENSE"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
