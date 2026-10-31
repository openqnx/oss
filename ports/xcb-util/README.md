# xcb-util 0.4.1

> XCB utility libraries: atom cache, events and convenience functions

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://xorg.freedesktop.org/releases/individual/xcb/xcb-util-0.4.1.tar.xz |
| SHA-256 | `5abe3bbbd8e54f0fa3ec945291b7e8fa8cfd3cccc43718f8758430f94126e512` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libxcb`](../libxcb/) |
| Run-time dependencies | [`libxcb`](../libxcb/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xorg_libs`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `xcb-util` (programs, shared libraries, run-time data) and `xcb-util-dev` (headers, pkg-config/CMake files, static libraries; needs `xcb-util`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`xcb-util`](https://openqnx.github.io/oss/package/x86_64/xcb-util.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xcb-util.html)) · [`xcb-util-dev`](https://openqnx.github.io/oss/package/x86_64/xcb-util-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/xcb-util-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add xcb-util` (dependencies come along); one version: `apk add xcb-util=<version>` | `apk del xcb-util` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install xcb-util` (dependencies first) | `oss-get remove xcb-util` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`xcb-util-dev` the same way, by its own name (`apk add xcb-util-dev`, `oss-get install xcb-util-dev`, by hand with its own file and list `xcb-util-dev.list`); remove it before `xcb-util`, which it needs.

```sh
# on the QNX target, as root
apk add xcb-util xcb-util-dev
apk add qnx-test-xorg-libs && /usr/local/bin/qnx-port-tests/test_xorg_libs    # its on-target test
apk del xcb-util xcb-util-dev qnx-test-xorg-libs
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/xcb-util.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/xcb-util.list
rm var/lib/manual/xcb-util.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xcb-util` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (7 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libxcb
```

<details><summary>Full build chain (7)</summary>

[`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
URL=https://xorg.freedesktop.org/releases/individual/xcb/xcb-util-0.4.1.tar.xz
mkdir -p dist src/xcb-util
curl -fL -o dist/$(basename $URL) $URL
echo "5abe3bbbd8e54f0fa3ec945291b7e8fa8cfd3cccc43718f8758430f94126e512  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xcb-util --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/xcb-util && cd build/$QNX_ARCH/xcb-util
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/xcb-util/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xcb-util: XCB utility library (atoms, events, aux helpers).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xcb-util`.

### 5. What it installs

10 files under `/usr/local`: 2 shared-library entries, 4 headers.

```
/usr/local/lib/libxcb-util.so
/usr/local/lib/libxcb-util.so.1
```

The complete list is `stage/<arch>/.manifest/xcb-util` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xcb-util
ls dists/8.0/*/ | grep '^xcb-util-'
```

This writes `xcb-util-<ver>-r0.tar.gz` (run time) and `xcb-util-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xcb-util
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xcb-util-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xcb-util-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xorg_libs`](../../tests/src/) links xcb-util the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xorg-libs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xorg-libs
/usr/local/bin/qnx-port-tests/test_xorg_libs
```

```
== xcb-util ==
  ok   ...
PASS xcb-util
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xorg_libs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xorg_libs`](../../tests/src/) -- a program linked against xcb-util the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libxcvt_gen_mode_info
   - horizontal timing = cvt's (173.00 MHz, 2048 2248 2576)
   - vertical timing = cvt's (1083 1088 1120)
   - xshmfence_alloc_shm
   - xshmfence_map_shm
   - new fence not triggered
   - triggered
   - await on a triggered fence returns
   - reset
   - xcb_event_get_label(KEY_PRESS) = KeyPress
   - XmuCopyISOLatin1Lowered (incl. E-acute)
   - XmuCompareISOLatin1 ignores case
   - IceListenForConnections
   - network id list
   - no session manager: the documented error
   - XtCreateApplicationContext
   - XtNewString
   - XkbRF_LoadRules
   - XkbRF_GetComponents
   - keycodes = evdev
   - symbols = pc+us
   - geometry = pc(pc105)
   - xfont2_make_font_pattern_cache
   - cached pattern found
   - other pattern not found

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libice-1.1.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libsm-1.2.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxcvt-0.1.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxfont2-2.0.7-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxkbfile-1.2.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxmu-1.3.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxshmfence-1.3.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxt-1.3.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/xcb-util-0.4.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libice-1.1.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libsm-1.2.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxcvt-0.1.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxfont2-2.0.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxkbfile-1.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxmu-1.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxshmfence-1.3.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxt-1.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/xcb-util-0.4.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.4.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xcb-util=<version>`), sources in `ported/xcb-util/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xcb-util`; port one with `scripts/port-update xcb-util`.

<details><summary>recipe.sh</summary>

```bash
# xcb-util: XCB utility library (atoms, events, aux helpers).
VERSION=0.4.1
TARBALL=https://xorg.freedesktop.org/releases/individual/xcb/xcb-util-0.4.1.tar.xz
SHA256=5abe3bbbd8e54f0fa3ec945291b7e8fa8cfd3cccc43718f8758430f94126e512
DEPS="libxcb"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
