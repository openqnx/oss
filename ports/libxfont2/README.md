# libxfont2 2.0.7

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://xorg.freedesktop.org/releases/individual/lib/libXfont2-2.0.7.tar.xz |
| SHA-256 | `8b7b82fdeba48769b69433e8e3fbb984a5f6bf368b0d5f47abeec49de3e58efb` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`freetype`](../freetype/) [`zlib`](../zlib/) [`fontenc`](../fontenc/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) |
| Run-time dependencies | [`fontenc`](../fontenc/) [`freetype`](../freetype/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xorg_libs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxfont2
apk add libxfont2-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xorg-libs && /usr/local/bin/qnx-port-tests/test_xorg_libs    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxfont2`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxfont2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild freetype zlib fontenc xorgproto xtrans
```

<details><summary>Full build chain (8)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`fontenc`](../fontenc/) [`freetype`](../freetype/) [`libpng`](../libpng/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://xorg.freedesktop.org/releases/individual/lib/libXfont2-2.0.7.tar.xz
mkdir -p dist src/libxfont2
curl -fL -o dist/$(basename $URL) $URL
echo "8b7b82fdeba48769b69433e8e3fbb984a5f6bf368b0d5f47abeec49de3e58efb  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libxfont2 --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libxfont2 && cd build/$QNX_ARCH/libxfont2
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxfont2/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-fc \
    --without-xmlto \
    --without-fop \
    --disable-devel-docs
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libxfont2: X font rendering (PCF, BDF, FreeType fonts) as X servers and Xwayland use it. --disable-fc: the xfs font-server client (fslibos.h) needs NOFILES_MAX and fd_set internals QNX does not have; font servers are a relic, local fonts work.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxfont2`.

### 5. What it installs

4 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libXfont2.so
/usr/local/lib/libXfont2.so.2
```

The complete list is `stage/<arch>/.manifest/libxfont2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxfont2
ls dists/8.0/*/ | grep '^libxfont2-'
```

This writes `libxfont2-<ver>-r0.tar.gz` (run time) and `libxfont2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxfont2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxfont2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxfont2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xorg_libs`](../../tests/src/) links libxfont2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xorg-libs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xorg-libs
/usr/local/bin/qnx-port-tests/test_xorg_libs
```

```
== libxfont2 ==
  ok   ...
PASS libxfont2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xorg_libs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xorg_libs`](../../tests/src/) -- a program linked against libxfont2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

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

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libice 1.1.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsm 1.2.6-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxcvt 0.1.3-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfont2 2.0.7-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxkbfile 1.2.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxmu 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxshmfence 1.3.3-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxt 1.3.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xcb-util 0.4.1-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.0.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxfont2=<version>`), sources in `ported/libxfont2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxfont2`; port one with `scripts/port-update libxfont2`.

<details><summary>recipe.sh</summary>

```bash
# libxfont2: X font rendering (PCF, BDF, FreeType fonts) as X servers and Xwayland use it.
# --disable-fc: the xfs font-server client (fslibos.h) needs NOFILES_MAX and
# fd_set internals QNX does not have; font servers are a relic, local fonts work.
VERSION=2.0.7
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libXfont2-2.0.7.tar.xz
SHA256=8b7b82fdeba48769b69433e8e3fbb984a5f6bf368b0d5f47abeec49de3e58efb
DEPS="freetype zlib fontenc xorgproto xtrans"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-fc --without-xmlto --without-fop --disable-devel-docs )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
