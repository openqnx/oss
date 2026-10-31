# libdisplay-info 0.3.0

> EDID and DisplayID parsing library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/emersion/libdisplay-info.git (tag `0.3.0`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libdisplay_info`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libdisplay-info` (programs, shared libraries, run-time data) and `libdisplay-info-dev` (headers, pkg-config/CMake files, static libraries; needs `libdisplay-info`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libdisplay-info`](https://openqnx.github.io/oss/package/x86_64/libdisplay-info.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdisplay-info.html)) · [`libdisplay-info-dev`](https://openqnx.github.io/oss/package/x86_64/libdisplay-info-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libdisplay-info-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libdisplay-info` (dependencies come along); one version: `apk add libdisplay-info=<version>` | `apk del libdisplay-info` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libdisplay-info` (dependencies first) | `oss-get remove libdisplay-info` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libdisplay-info-dev` the same way, by its own name (`apk add libdisplay-info-dev`, `oss-get install libdisplay-info-dev`, by hand with its own file and list `libdisplay-info-dev.list`); remove it before `libdisplay-info`, which it needs.

```sh
# on the QNX target, as root
apk add libdisplay-info libdisplay-info-dev
apk add qnx-test-libdisplay-info && /usr/local/bin/qnx-port-tests/test_libdisplay_info    # its on-target test
apk del libdisplay-info libdisplay-info-dev qnx-test-libdisplay-info
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libdisplay-info.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libdisplay-info.list
rm var/lib/manual/libdisplay-info.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libdisplay-info` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.3.0 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/emersion/libdisplay-info.git src/libdisplay-info
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libdisplay-info < ports/libdisplay-info/patches/0001-qnx-no-open-memstream.patch
```

**[`0001-qnx-no-open-memstream.patch`](patches/0001-qnx-no-open-memstream.patch)** — Work without open_memstream() (QNX).

> QNX 8's C library has no open_memstream(). The library's memory_stream
> helper writes to a tmpfile() on QNX and reads the text back into a string on
> close; di-edid-decode's join_str() builds its string directly.
>
> (QNX's own package includes <sys/memstream.h> from a QNX add-on library that
> is not part of the SDP; this keeps the port buildable with the SDP alone.)

Touches: `memory-stream.c`, `di-edid-decode/edid.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libdisplay-info src/libdisplay-info \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/libdisplay-info
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libdisplay-info install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libdisplay-info: EDID/DisplayID parsing. Version as in the QNX Everywhere image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libdisplay-info`.

### 5. What it installs

14 files under `/usr/local`: 3 shared-library entries, 1 programs, 8 headers.

```
/usr/local/lib/libdisplay-info.so
/usr/local/lib/libdisplay-info.so.0.3.0
/usr/local/lib/libdisplay-info.so.3
/usr/local/bin/di-edid-decode
```

The complete list is `stage/<arch>/.manifest/libdisplay-info` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libdisplay-info
ls dists/8.0/*/ | grep '^libdisplay-info-'
```

This writes `libdisplay-info-<ver>-r0.tar.gz` (run time) and `libdisplay-info-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libdisplay-info
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libdisplay-info-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libdisplay-info-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libdisplay_info`](../../tests/src/) links libdisplay-info the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libdisplay-info`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libdisplay-info
/usr/local/bin/qnx-port-tests/test_libdisplay_info
```

```
== libdisplay-info ==
  ok   ...
PASS libdisplay-info
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libdisplay_info` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libdisplay_info`](../../tests/src/) -- a program linked against libdisplay-info the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - di_info_parse_edid accepts the block
   - no conformity failures
   - di_info_get_make names QNX
   - di_info_get_model = \"QNX Test\"
   - di_info_get_serial = QNX12345
   - EDID version 1.4
   - vendor/product: QNX 0x1234 serial 1
   - manufactured week 1 of 2024
   - screen size 52 x 29 cm
   - one detailed timing definition
   - DTD: 1920x1080 at 148.5 MHz
   - DTD: blanking, porches and sync widths
   - DTD: 527 x 296 mm, progressive
   - 3 display descriptors, product name \"QNX Test\"
   - a bad checksum is reported
   - di_cvt_compute: CVT-RB 1920x1080@60 = 138.5 MHz, 48/32/80
   - scratch directory
   - di-edid-decode: manufacturer, product name, DTD 1920x1080 148.5 MHz, conformity PASS

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdisplay-info 0.3.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libdisplay-info-0.3.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libdisplay-info 0.3.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libdisplay-info-0.3.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdisplay-info 0.3.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libdisplay-info-0.3.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libdisplay-info 0.3.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libdisplay-info-0.3.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.3.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libdisplay-info=<version>`), sources in `ported/libdisplay-info/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libdisplay-info`; port one with `scripts/port-update libdisplay-info`.

<details><summary>recipe.sh</summary>

```bash
# libdisplay-info: EDID/DisplayID parsing. Version as in the QNX Everywhere image.
VERSION=0.3.0; GIT=https://gitlab.freedesktop.org/emersion/libdisplay-info.git; REF=0.3.0
BUILDSYS=meson
MESON_ARGS_EXTRA=( )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
