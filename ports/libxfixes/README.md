# libxfixes 6.0.1

> X Fixes extension client library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libxfixes.git (tag `libXfixes-6.0.1`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) [`xorgproto`](../xorgproto/) |
| Run-time dependencies | [`libx11`](../libx11/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxfixes`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxfixes
apk add libxfixes-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libxfixes && /usr/local/bin/qnx-port-tests/test_libxfixes    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxfixes`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxfixes` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libx11 xorgproto
```

<details><summary>Full build chain (8)</summary>

[`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b libXfixes-6.0.1 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/lib/libxfixes.git src/libxfixes
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libxfixes && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libxfixes && cd build/$QNX_ARCH/libxfixes
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxfixes/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-malloc0returnsnull \
    --enable-shared \
    --disable-static \
    --without-xmlto \
    --without-fop \
    --disable-specs
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- malloc(0) is a run-time test, which cross configure cannot perform. QNX documents it explicitly: "If size is zero, then malloc() returns a valid pointer with a size of zero" -- so it does not return NULL, and Xlib does not need its add-a-byte workaround.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxfixes`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libXfixes.so
/usr/local/lib/libXfixes.so.3
```

The complete list is `stage/<arch>/.manifest/libxfixes` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxfixes
ls dists/8.0/*/ | grep '^libxfixes-'
```

This writes `libxfixes-<ver>-r0.tar.gz` (run time) and `libxfixes-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxfixes
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxfixes-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxfixes-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libxfixes`](../../tests/src/) links libxfixes the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libxfixes`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libxfixes
/usr/local/bin/qnx-port-tests/test_libxfixes
```

```
== libxfixes ==
  ok   ...
PASS libxfixes
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxfixes` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libxfixes`](../../tests/src/) -- a program linked against libxfixes the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - headless X server (labwc + Xwayland) up
   - XFIXES extension present
   - XFIXES version >= 4
   - union of two squares: 3 bands, 15x15 bounds
   - intersection is 5x5+5+5
   - subtraction leaves an L of 2 rects in 10x10
   - cursor name set and read back
   - XFixesSelectionNotify for the new owner
   - XFixesHideCursor/ShowCursor accepted

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfixes 6.0.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxfixes-6.0.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfixes 6.0.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxfixes-6.0.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfixes 6.0.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxfixes-6.0.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfixes 6.0.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxfixes-6.0.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.0.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxfixes=<version>`), sources in `ported/libxfixes/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxfixes`; port one with `scripts/port-update libxfixes`.

<details><summary>recipe.sh</summary>

```bash
VERSION=6.0.1; GIT=https://gitlab.freedesktop.org/xorg/lib/libxfixes.git; REF=libXfixes-6.0.1
DEPS="libx11 xorgproto"
BUILDSYS=autotools
# malloc(0) is a run-time test, which cross configure cannot perform. QNX
# documents it explicitly: "If size is zero, then malloc() returns a valid
# pointer with a size of zero" -- so it does not return NULL, and Xlib does
# not need its add-a-byte workaround.
CONF_ARGS_EXTRA=( --disable-malloc0returnsnull --enable-shared --disable-static --without-xmlto --without-fop --disable-specs )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
