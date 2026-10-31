# libxshmfence 1.3.3

> Shared memory fence synchronization library for X

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://xorg.freedesktop.org/releases/individual/lib/libxshmfence-1.3.3.tar.xz |
| SHA-256 | `d4a4df096aba96fea02c029ee3a44e11a47eb7f7213c1a729be83e85ec3fde10` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`xorgproto`](../xorgproto/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xorg_libs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxshmfence
apk add libxshmfence-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xorg-libs && /usr/local/bin/qnx-port-tests/test_xorg_libs    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxshmfence`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxshmfence` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild xorgproto
```

### 2. Get the upstream source

```bash
URL=https://xorg.freedesktop.org/releases/individual/lib/libxshmfence-1.3.3.tar.xz
mkdir -p dist src/libxshmfence
curl -fL -o dist/$(basename $URL) $URL
echo "d4a4df096aba96fea02c029ee3a44e11a47eb7f7213c1a729be83e85ec3fde10  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libxshmfence --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libxshmfence && cd build/$QNX_ARCH/libxshmfence
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxshmfence/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libxshmfence: shared-memory fences, the synchronisation X servers and DRI3 clients share (futex-like, over a shared file).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxshmfence`.

### 5. What it installs

5 files under `/usr/local`: 3 shared-library entries, 1 headers.

```
/usr/local/lib/libxshmfence.so
/usr/local/lib/libxshmfence.so.1
/usr/local/lib/libxshmfence.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/libxshmfence` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxshmfence
ls dists/8.0/*/ | grep '^libxshmfence-'
```

This writes `libxshmfence-<ver>-r0.tar.gz` (run time) and `libxshmfence-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxshmfence
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxshmfence-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxshmfence-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xorg_libs`](../../tests/src/) links libxshmfence the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xorg-libs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xorg-libs
/usr/local/bin/qnx-port-tests/test_xorg_libs
```

```
== libxshmfence ==
  ok   ...
PASS libxshmfence
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xorg_libs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xorg_libs`](../../tests/src/) -- a program linked against libxshmfence the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libice-1.1.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libsm-1.2.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxcvt-0.1.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxfont2-2.0.7-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxkbfile-1.2.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxmu-1.3.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxshmfence-1.3.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxt-1.3.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xcb-util-0.4.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libice-1.1.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libsm-1.2.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxcvt-0.1.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxfont2-2.0.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxkbfile-1.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxmu-1.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxshmfence-1.3.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxt-1.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xcb-util-0.4.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxshmfence=<version>`), sources in `ported/libxshmfence/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxshmfence`; port one with `scripts/port-update libxshmfence`.

<details><summary>recipe.sh</summary>

```bash
# libxshmfence: shared-memory fences, the synchronisation X servers and
# DRI3 clients share (futex-like, over a shared file).
VERSION=1.3.3
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libxshmfence-1.3.3.tar.xz
SHA256=d4a4df096aba96fea02c029ee3a44e11a47eb7f7213c1a729be83e85ec3fde10
DEPS="xorgproto"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
