# libxext 1.3.6

> X11 library for common protocol extensions

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libxext.git (tag `libXext-1.3.6`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) [`xorgproto`](../xorgproto/) |
| Run-time dependencies | [`libx11`](../libx11/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxext`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libxext` (programs, shared libraries, run-time data) and `libxext-dev` (headers, pkg-config/CMake files, static libraries; needs `libxext`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libxext`](https://openqnx.github.io/oss/package/x86_64/libxext.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libxext.html)) · [`libxext-dev`](https://openqnx.github.io/oss/package/x86_64/libxext-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libxext-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libxext` (dependencies come along); one version: `apk add libxext=<version>` | `apk del libxext` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libxext` (dependencies first) | `oss-get remove libxext` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libxext-dev` the same way, by its own name (`apk add libxext-dev`, `oss-get install libxext-dev`, by hand with its own file and list `libxext-dev.list`); remove it before `libxext`, which it needs.

```sh
# on the QNX target, as root
apk add libxext libxext-dev
apk add qnx-test-libxext && /usr/local/bin/qnx-port-tests/test_libxext    # its on-target test, from tests-8.0-<arch>
apk del libxext libxext-dev qnx-test-libxext
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libxext.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libxext.list
rm var/lib/manual/libxext.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxext` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b libXext-1.3.6 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/lib/libxext.git src/libxext
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libxext && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libxext && cd build/$QNX_ARCH/libxext
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxext/configure --host=$QNX_TRIPLE --prefix=/usr/local \
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

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxext`.

### 5. What it installs

80 files under `/usr/local`: 2 shared-library entries, 16 headers.

```
/usr/local/lib/libXext.so
/usr/local/lib/libXext.so.6
```

The complete list is `stage/<arch>/.manifest/libxext` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxext
ls dists/8.0/*/ | grep '^libxext-'
```

This writes `libxext-<ver>-r0.tar.gz` (run time) and `libxext-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxext
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxext-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxext-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libxext`](../../tests/src/) links libxext the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libxext`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libxext
/usr/local/bin/qnx-port-tests/test_libxext
```

```
== libxext ==
  ok   ...
PASS libxext
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxext` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libxext`](../../tests/src/) -- a program linked against libxext the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - headless X server (labwc + Xwayland) up
   - SHAPE extension present
   - XShapeQueryVersion >= 1.0
   - XShapeGetRectangles returns the two rectangles set
   - XShapeQueryExtents: shaped, 80x80 extents
   - SYNC extension initialised
   - sync counter created at 42
   - XSyncChangeCounter +8 -> 50
   - system counter SERVERTIME listed

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxext 1.3.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxext-1.3.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxext 1.3.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxext-1.3.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxext 1.3.6-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxext-1.3.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxext 1.3.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxext-1.3.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxext=<version>`), sources in `ported/libxext/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxext`; port one with `scripts/port-update libxext`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.3.6; GIT=https://gitlab.freedesktop.org/xorg/lib/libxext.git; REF=libXext-1.3.6
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
