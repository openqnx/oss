# libcanard 5.0.0

> OpenCyphal/CAN (formerly UAVCAN) protocol stack in C: transfers over classic CAN and CAN FD

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/OpenCyphal/libcanard.git (tag `v5.0.0`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libcanard`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libcanard` (programs, shared libraries, run-time data) and `libcanard-dev` (headers, pkg-config/CMake files, static libraries; needs `libcanard`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libcanard`](https://openqnx.github.io/oss/package/x86_64/libcanard.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libcanard.html)) · [`libcanard-dev`](https://openqnx.github.io/oss/package/x86_64/libcanard-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libcanard-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libcanard` (dependencies come along); one version: `apk add libcanard=<version>` | `apk del libcanard` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libcanard` (dependencies first) | `oss-get remove libcanard` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libcanard-dev` the same way, by its own name (`apk add libcanard-dev`, `oss-get install libcanard-dev`, by hand with its own file and list `libcanard-dev.list`); remove it before `libcanard`, which it needs.

```sh
# on the QNX target, as root
apk add libcanard libcanard-dev
apk add qnx-test-libcanard && /usr/local/bin/qnx-port-tests/test_libcanard    # its on-target test, from tests-8.0-<arch>
apk del libcanard libcanard-dev qnx-test-libcanard
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libcanard.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libcanard.list
rm var/lib/manual/libcanard.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libcanard` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v5.0.0 --recurse-submodules --shallow-submodules \
    https://github.com/OpenCyphal/libcanard.git src/libcanard
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libcanard; B=build/$QNX_ARCH/libcanard; mkdir -p $B
```

```bash
do_build() {
  lc_cc -c "$S/libcanard/canard.c" -o "$B/canard.o" || return 1
  rm -f "$B/libcanard.a"; "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libcanard.a" "$B/canard.o" || return 1
  lc_cc -shared -Wl,-soname,libcanard.so.5 "$B/canard.o" -o "$B/libcanard.so.5"
}
```

```bash
do_install() {
  install -Dm755 "$B/libcanard.so.5" "$STAGE$PREFIX/lib/libcanard.so.5" || return 1
  ln -sf libcanard.so.5 "$STAGE$PREFIX/lib/libcanard.so"
  install -Dm644 "$B/libcanard.a" "$STAGE$PREFIX/lib/libcanard.a" || return 1
  install -Dm644 "$S/libcanard/canard.h" "$STAGE$PREFIX/include/canard.h" || return 1
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/libcanard/LICENSE" || return 1
  install -d "$STAGE$PREFIX/lib/pkgconfig"
  cat > "$STAGE$PREFIX/lib/pkgconfig/libcanard.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: libcanard
Description: OpenCyphal/CAN and DroneCAN protocol stack in C
Version: $VERSION
Libs: -L\${libdir} -lcanard
Cflags: -I\${includedir}
PC
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libcanard: the OpenCyphal/CAN (and DroneCAN, the former UAVCAN v0) protocol stack in C for embedded and real-time systems: transfers are split into and reassembled from classic CAN and CAN FD frames, with priorities, multi-frame CRC, transfer-ID deduplication, redundant interfaces and node-ID allocation. It has no I/O of its own: the application hands it received frames (canard_ingest_frame()) and sends the frames it asks for (the tx vtable), here through qnx-socketcan's CAN_RAW. Upstream distributes two files to copy into a project (canard.c, canard.h; its CMake file is for library development: clang-tidy, cppcheck, tests). Here they are a library: libcanard.so.5 and libcanard.a, canard.h and libcanard.pc. The AVL tree header (lib/cavl2) is internal to canard.c.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libcanard`.

### 5. What it installs

6 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libcanard.so
/usr/local/lib/libcanard.so.5
```

The complete list is `stage/<arch>/.manifest/libcanard` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libcanard
ls dists/8.0/*/ | grep '^libcanard-'
```

This writes `libcanard-<ver>-r0.tar.gz` (run time) and `libcanard-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libcanard
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libcanard-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libcanard-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libcanard`](../../tests/src/) links libcanard the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libcanard`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libcanard
/usr/local/bin/qnx-port-tests/test_libcanard
```

```
== libcanard ==
  ok   ...
PASS libcanard
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libcanard` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libcanard`](../../tests/src/) -- a program linked against libcanard the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - canard_new: three nodes
   - node-IDs 10, 20, 30
   - the anonymous node-ID is refused
   - node 20 subscribes to subject 1234 (13-bit)
   - canard_find_subscription
   - node 10 publishes 5 bytes, transfer-ID 7
   - one frame on the bus
   - CAN id 0x1064D20A: priority nominal, subject 1234, source node 10
   - payload + tail byte 0xE7 (start, end, toggle, transfer-ID 7)
   - the subscriber receives the 5 bytes
   - source node 10, transfer-ID 7, priority nominal
   - node 10 publishes 100 bytes
   - 15 classic frames (100 bytes + 2 bytes CRC, 7 per frame)
   - the subscriber receives the 100 bytes, priority high
   - node 10 publishes 100 bytes with CAN FD frames
   - 2 CAN FD frames, the first 64 bytes long
   - the subscriber receives the 100 bytes (and the padding of the last frame)
   - a subscription with extent 16 gets the first 16 bytes
   - node 20 subscribes to subject 1234 (16-bit, Cyphal v1.1)
   - node 10 publishes on the 16-bit subject
   - the 16-bit subscriber receives it
   - the 13-bit subscriber of the same number does not
   - node 10 requests service 77 of node 20
   - node 20 receives the request from node 10, transfer-ID 3
   - node 20 responds
   - ... and 9 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libcanard 5.0.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/libcanard-5.0.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libcanard 5.0.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/libcanard-5.0.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.0.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libcanard=<version>`), sources in `ported/libcanard/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libcanard`; port one with `scripts/port-update libcanard`.

<details><summary>recipe.sh</summary>

```bash
# libcanard: the OpenCyphal/CAN (and DroneCAN, the former UAVCAN v0)
# protocol stack in C for embedded and real-time systems: transfers are
# split into and reassembled from classic CAN and CAN FD frames, with
# priorities, multi-frame CRC, transfer-ID deduplication, redundant
# interfaces and node-ID allocation. It has no I/O of its own: the
# application hands it received frames (canard_ingest_frame()) and sends the
# frames it asks for (the tx vtable), here through qnx-socketcan's CAN_RAW.
#
# Upstream distributes two files to copy into a project (canard.c, canard.h;
# its CMake file is for library development: clang-tidy, cppcheck, tests).
# Here they are a library: libcanard.so.5 and libcanard.a, canard.h and
# libcanard.pc. The AVL tree header (lib/cavl2) is internal to canard.c.
VERSION=5.0.0; GIT=https://github.com/OpenCyphal/libcanard.git; REF=v5.0.0
DEPS=""
BUILDSYS=custom
lc_cc() { "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -D_QNX_SOURCE -std=gnu11 -O2 -fPIC -Wall -Wextra -I"$S/libcanard" -I"$S/lib/cavl2" "$@"; }
do_build() {
  lc_cc -c "$S/libcanard/canard.c" -o "$B/canard.o" || return 1
  rm -f "$B/libcanard.a"; "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libcanard.a" "$B/canard.o" || return 1
  lc_cc -shared -Wl,-soname,libcanard.so.5 "$B/canard.o" -o "$B/libcanard.so.5"
}
do_install() {
  install -Dm755 "$B/libcanard.so.5" "$STAGE$PREFIX/lib/libcanard.so.5" || return 1
  ln -sf libcanard.so.5 "$STAGE$PREFIX/lib/libcanard.so"
  install -Dm644 "$B/libcanard.a" "$STAGE$PREFIX/lib/libcanard.a" || return 1
  install -Dm644 "$S/libcanard/canard.h" "$STAGE$PREFIX/include/canard.h" || return 1
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/libcanard/LICENSE" || return 1
  install -d "$STAGE$PREFIX/lib/pkgconfig"
  cat > "$STAGE$PREFIX/lib/pkgconfig/libcanard.pc" <<PC
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include

Name: libcanard
Description: OpenCyphal/CAN and DroneCAN protocol stack in C
Version: $VERSION
Libs: -L\${libdir} -lcanard
Cflags: -I\${includedir}
PC
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
