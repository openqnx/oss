# isotp-c 1.9.3

> ISO 15765-2 (ISO-TP) transport protocol for CAN in C: segmentation, flow control, CAN FD

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/SimonCahill/isotp-c.git (tag `v1.9.3`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_isotp_c`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `isotp-c` (programs, shared libraries, run-time data) and `isotp-c-dev` (headers, pkg-config/CMake files, static libraries; needs `isotp-c`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`isotp-c`](https://openqnx.github.io/oss/package/x86_64/isotp-c.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/isotp-c.html)) · [`isotp-c-dev`](https://openqnx.github.io/oss/package/x86_64/isotp-c-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/isotp-c-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add isotp-c` (dependencies come along); one version: `apk add isotp-c=<version>` | `apk del isotp-c` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install isotp-c` (dependencies first) | `oss-get remove isotp-c` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`isotp-c-dev` the same way, by its own name (`apk add isotp-c-dev`, `oss-get install isotp-c-dev`, by hand with its own file and list `isotp-c-dev.list`); remove it before `isotp-c`, which it needs.

```sh
# on the QNX target, as root
apk add isotp-c isotp-c-dev
apk add qnx-test-isotp-c && /usr/local/bin/qnx-port-tests/test_isotp_c    # its on-target test, from tests-8.0-<arch>
apk del isotp-c isotp-c-dev qnx-test-isotp-c
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/isotp-c.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/isotp-c.list
rm var/lib/manual/isotp-c.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild isotp-c` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v1.9.3 --recurse-submodules --shallow-submodules \
    https://github.com/SimonCahill/isotp-c.git src/isotp-c
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/isotp-c; B=build/$QNX_ARCH/isotp-c; mkdir -p $B
```

```bash
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -fPIC -Wall -Wextra $ISOTP_DEFS -I"$S" -c "$S/isotp.c" -o "$B/isotp.o" || return 1
  rm -f "$B/libisotp.a"; "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libisotp.a" "$B/isotp.o"
}
```

```bash
do_install() {
  install -Dm644 "$B/libisotp.a" "$STAGE$PREFIX/lib/libisotp.a" || return 1
  install -d "$STAGE$PREFIX/include/isotp_c" "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/share/licenses/isotp-c"
  install -m644 "$S"/isotp.h "$S"/isotp_config.h "$S"/isotp_defines.h "$S"/isotp_user.h "$STAGE$PREFIX/include/isotp_c/" || return 1
  install -m644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/isotp-c/" || return 1
  cat > "$STAGE$PREFIX/lib/pkgconfig/isotp-c.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: isotp-c
Description: ISO 15765-2 (ISO-TP) transport for CAN in C
Version: $VERSION
Libs: -L\${libdir} -lisotp
Cflags: -I\${includedir}/isotp_c $ISOTP_DEFS
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- isotp-c: ISO 15765-2 (ISO-TP) transport for CAN in portable C: single frames, segmented messages with flow control, block size and STmin, CAN FD frame lengths. The stack is driven by the application (isotp_poll(), isotp_on_can_message()) and sends through three callbacks the application defines (isotp_user_send_can(), isotp_user_get_us(), isotp_user_debug()), so on QNX it runs over qnx-socketcan's CAN_RAW sockets, where Linux's CAN_ISOTP kernel sockets are not available. Upstream's CMake file has no install step and builds a shared library with the callbacks left undefined; here it is a static, position independent libisotp.a, the headers in include/isotp_c/ and isotp-c.pc. The compile time configuration is part of the interface (it sizes IsoTpLink and sets the callback signature), so isotp-c.pc carries the same definitions: frames up to 64 bytes (CAN FD) received, 8-byte frames sent by default (isotp_set_tx_dl() changes it per link), padding with 0xAA, and isotp_user_send_can(id, data, size, flags, arg) with the frame flags and the per-link argument (isotp_init_link(); link->user_send_can_arg).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH isotp-c`.

### 5. What it installs

7 files under `/usr/local`, 4 headers.

The complete list is `stage/<arch>/.manifest/isotp-c` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa isotp-c
ls dists/8.0/*/ | grep '^isotp-c-'
```

This writes `isotp-c-<ver>-r0.tar.gz` (run time) and `isotp-c-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add isotp-c
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/isotp-c-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/isotp-c-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_isotp_c`](../../tests/src/) links isotp-c the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-isotp-c`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-isotp-c
/usr/local/bin/qnx-port-tests/test_isotp_c
```

```
== isotp-c ==
  ok   ...
PASS isotp-c
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_isotp_c` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_isotp_c`](../../tests/src/) -- a program linked against isotp-c the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - isotp_receive on an idle link: ISOTP_RET_NO_DATA
   - send 3 bytes
   - one Single Frame 03 01 02 03, padded with AA, id 7E0
   - the peer receives the 3 bytes
   - send 20 bytes
   - a second send during the transfer: ISOTP_RET_INPROGRESS
   - four frames on the bus (FF, FC, CF, CF)
   - First Frame 10 14 + 6 data bytes
   - Flow Control 30 08 00 from the receiver (id 7E8)
   - Consecutive Frame 21 + 7 data bytes
   - Consecutive Frame 22 + the last 7 bytes
   - the peer receives the 20 bytes
   - the sender is idle again
   - send 300 bytes A -> B
   - B receives the 300 bytes
   - 1 First Frame, 42 Consecutive Frames, 6 Flow Control frames (block size 8)
   - send 300 bytes B -> A
   - A receives the 300 bytes
   - TX_DL 64 (CAN FD)
   - TX_DL 9 is refused
   - send 100 bytes with 64-byte frames
   - one 64-byte First Frame 10 64 and one Consecutive Frame
   - the peer receives the 100 bytes
   - a message larger than the send buffer: ISOTP_RET_OVERFLOW
   - send 50 bytes on a bus without a receiver
   - ... and 3 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | isotp-c 1.9.3-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/isotp-c-1.9.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | isotp-c 1.9.3-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/isotp-c-1.9.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.9.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add isotp-c=<version>`), sources in `ported/isotp-c/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates isotp-c`; port one with `scripts/port-update isotp-c`.

<details><summary>recipe.sh</summary>

```bash
# isotp-c: ISO 15765-2 (ISO-TP) transport for CAN in portable C: single
# frames, segmented messages with flow control, block size and STmin, CAN FD
# frame lengths. The stack is driven by the application (isotp_poll(),
# isotp_on_can_message()) and sends through three callbacks the application
# defines (isotp_user_send_can(), isotp_user_get_us(), isotp_user_debug()),
# so on QNX it runs over qnx-socketcan's CAN_RAW sockets, where Linux's
# CAN_ISOTP kernel sockets are not available.
#
# Upstream's CMake file has no install step and builds a shared library with
# the callbacks left undefined; here it is a static, position independent
# libisotp.a, the headers in include/isotp_c/ and isotp-c.pc. The compile
# time configuration is part of the interface (it sizes IsoTpLink and sets
# the callback signature), so isotp-c.pc carries the same definitions:
# frames up to 64 bytes (CAN FD) received, 8-byte frames sent by default
# (isotp_set_tx_dl() changes it per link), padding with 0xAA, and
# isotp_user_send_can(id, data, size, flags, arg) with the frame flags and
# the per-link argument (isotp_init_link(); link->user_send_can_arg).
VERSION=1.9.3; GIT=https://github.com/SimonCahill/isotp-c.git; REF=v1.9.3
DEPS=""
BUILDSYS=custom
ISOTP_DEFS="-DISO_TP_MAX_CAN_FRAME_SIZE=64 -DISO_TP_DEFAULT_TX_DL=8 -DISO_TP_FRAME_PADDING -DISO_TP_FRAME_PADDING_VALUE=0xAA -DISO_TP_USER_SEND_CAN_ARG -DISO_TP_USER_SEND_CAN_FLAGS"
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -fPIC -Wall -Wextra $ISOTP_DEFS -I"$S" -c "$S/isotp.c" -o "$B/isotp.o" || return 1
  rm -f "$B/libisotp.a"; "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libisotp.a" "$B/isotp.o"
}
do_install() {
  install -Dm644 "$B/libisotp.a" "$STAGE$PREFIX/lib/libisotp.a" || return 1
  install -d "$STAGE$PREFIX/include/isotp_c" "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/share/licenses/isotp-c"
  install -m644 "$S"/isotp.h "$S"/isotp_config.h "$S"/isotp_defines.h "$S"/isotp_user.h "$STAGE$PREFIX/include/isotp_c/" || return 1
  install -m644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/isotp-c/" || return 1
  cat > "$STAGE$PREFIX/lib/pkgconfig/isotp-c.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: isotp-c
Description: ISO 15765-2 (ISO-TP) transport for CAN in C
Version: $VERSION
Libs: -L\${libdir} -lisotp
Cflags: -I\${includedir}/isotp_c $ISOTP_DEFS
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
