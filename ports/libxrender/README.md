# libxrender 0.9.11

> X Render extension client library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libxrender.git (tag `libXrender-0.9.11`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) [`xorgproto`](../xorgproto/) |
| Run-time dependencies | [`libx11`](../libx11/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxrender`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libxrender` (programs, shared libraries, run-time data) and `libxrender-dev` (headers, pkg-config/CMake files, static libraries; needs `libxrender`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libxrender`](https://openqnx.github.io/oss/package/x86_64/libxrender.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libxrender.html)) · [`libxrender-dev`](https://openqnx.github.io/oss/package/x86_64/libxrender-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libxrender-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libxrender` (dependencies come along); one version: `apk add libxrender=<version>` | `apk del libxrender` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libxrender` (dependencies first) | `oss-get remove libxrender` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libxrender-dev` the same way, by its own name (`apk add libxrender-dev`, `oss-get install libxrender-dev`, by hand with its own file and list `libxrender-dev.list`); remove it before `libxrender`, which it needs.

```sh
# on the QNX target, as root
apk add libxrender libxrender-dev
apk add qnx-test-libxrender && /usr/local/bin/qnx-port-tests/test_libxrender    # its on-target test
apk del libxrender libxrender-dev qnx-test-libxrender
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libxrender.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libxrender.list
rm var/lib/manual/libxrender.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxrender` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b libXrender-0.9.11 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/lib/libxrender.git src/libxrender
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libxrender && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libxrender && cd build/$QNX_ARCH/libxrender
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxrender/configure --host=$QNX_TRIPLE --prefix=/usr/local \
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

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxrender`.

### 5. What it installs

5 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libXrender.so
/usr/local/lib/libXrender.so.1
```

The complete list is `stage/<arch>/.manifest/libxrender` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxrender
ls dists/8.0/*/ | grep '^libxrender-'
```

This writes `libxrender-<ver>-r0.tar.gz` (run time) and `libxrender-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxrender
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxrender-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxrender-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libxrender`](../../tests/src/) links libxrender the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libxrender`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libxrender
/usr/local/bin/qnx-port-tests/test_libxrender
```

```
== libxrender ==
  ok   ...
PASS libxrender
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxrender` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libxrender`](../../tests/src/) -- a program linked against libxrender the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - mkdtemp runtime dir
   - labwc up (headless Wayland compositor)
   - Xwayland up (X display on labwc)
   - XOpenDisplay
   - RENDER extension present
   - RENDER version >= 0.10
   - ARGB32 format: depth 32, a@24 r@16
   - RGB24 format: no alpha
   - A8 format
   - XRenderParseColor rgba:ff/80/00/ff (opaque)
   - XRenderParseColor rgba:ff/80/00/40 (premultiplied)
   - XRenderFillRectangle: solid blue read back
   - Over: 50% red on blue -> (128,0,127)
   - outside the composite rectangle unchanged

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxrender 0.9.11-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxrender-0.9.11-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxrender 0.9.11-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxrender-0.9.11-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxrender 0.9.11-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxrender-0.9.11-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxrender 0.9.11-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxrender-0.9.11-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.9.11**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxrender=<version>`), sources in `ported/libxrender/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxrender`; port one with `scripts/port-update libxrender`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.9.11; GIT=https://gitlab.freedesktop.org/xorg/lib/libxrender.git; REF=libXrender-0.9.11
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
