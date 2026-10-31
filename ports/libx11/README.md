# libx11 1.8.10

> Core X11 protocol client library (Xlib)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libx11.git (tag `libX11-1.8.10`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libxcb`](../libxcb/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) |
| Run-time dependencies | [`libxcb`](../libxcb/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libx11`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libx11` (programs, shared libraries, run-time data) and `libx11-dev` (headers, pkg-config/CMake files, static libraries; needs `libx11`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libx11`](https://openqnx.github.io/oss/package/x86_64/libx11.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libx11.html)) · [`libx11-dev`](https://openqnx.github.io/oss/package/x86_64/libx11-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libx11-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libx11` (dependencies come along); one version: `apk add libx11=<version>` | `apk del libx11` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libx11` (dependencies first) | `oss-get remove libx11` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libx11-dev` the same way, by its own name (`apk add libx11-dev`, `oss-get install libx11-dev`, by hand with its own file and list `libx11-dev.list`); remove it before `libx11`, which it needs.

```sh
# on the QNX target, as root
apk add libx11 libx11-dev
apk add qnx-test-libx11 && /usr/local/bin/qnx-port-tests/test_libx11    # its on-target test
apk del libx11 libx11-dev qnx-test-libx11
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libx11.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libx11.list
rm var/lib/manual/libx11.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libx11` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libxcb xorgproto xtrans
```

<details><summary>Full build chain (7)</summary>

[`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b libX11-1.8.10 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/lib/libx11.git src/libx11
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libx11 && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libx11 && cd build/$QNX_ARCH/libx11
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libx11/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --without-xmlto \
    --without-fop \
    --disable-specs \
    --disable-malloc0returnsnull \
    --with-keysymdefdir="$STAGE$PREFIX/include/X11"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- makekeys is a build-machine tool libX11 compiles and runs during the build; makekeys is a build-machine tool libX11 compiles and runs during the build. malloc(0) is a run-time test, which cross configure cannot perform. QNX documents it explicitly: "If size is zero, then malloc() returns a valid pointer with a size of zero" -- so it does not return NULL, and Xlib does not need its add-a-byte workaround.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libx11`.

### 5. What it installs

1019 files under `/usr/local`: 4 shared-library entries, 13 headers.

```
/usr/local/lib/libX11-xcb.so
/usr/local/lib/libX11-xcb.so.1
/usr/local/lib/libX11.so
/usr/local/lib/libX11.so.6
```

The complete list is `stage/<arch>/.manifest/libx11` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libx11
ls dists/8.0/*/ | grep '^libx11-'
```

This writes `libx11-<ver>-r0.tar.gz` (run time) and `libx11-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libx11
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libx11-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libx11-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libx11`](../../tests/src/) links libx11 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libx11`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libx11
/usr/local/bin/qnx-port-tests/test_libx11
```

```
== libx11 ==
  ok   ...
PASS libx11
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libx11` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libx11`](../../tests/src/) -- a program linked against libx11 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - XStringToKeysym: a, Return, F12, udiaeresis
   - XKeysymToString: BackSpace, KP_Enter
   - unknown keysym name -> NoSymbol
   - XConvertCase: Latin, Latin-1 and Greek
   - XrmGetStringDatabase parses a resource text
   - exact resource name
   - class-based wildcard match
   - global * wildcard
   - continued line joined
   - class match on an inner component
   - unmatched resource not found
   - XrmMergeDatabases overrides and adds
   - XrmPutFileDatabase / XrmGetFileDatabase round trip
   - quarks are interned
   - XParseGeometry 800x600+10-20
   - region union bounding box 150x75
   - XPointInRegion inside/outside the L shape
   - XIntersectRegion
   - XOpenDisplay(:97) with no server returns NULL

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libx11 1.8.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libx11-1.8.10-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libx11 1.8.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libx11-1.8.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libx11 1.8.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libx11-1.8.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libx11 1.8.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libx11-1.8.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.8.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libx11=<version>`), sources in `ported/libx11/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libx11`; port one with `scripts/port-update libx11`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.8.10; GIT=https://gitlab.freedesktop.org/xorg/lib/libx11.git; REF=libX11-1.8.10
DEPS="libxcb xorgproto xtrans"
BUILDSYS=autotools
# makekeys is a build-machine tool libX11 compiles and runs during the build;
# makekeys is a build-machine tool libX11 compiles and runs during the build.
# malloc(0) is a run-time test, which cross configure cannot perform. QNX
# documents it explicitly: "If size is zero, then malloc() returns a valid
# pointer with a size of zero" -- so it does not return NULL, and Xlib does
# not need its add-a-byte workaround.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-xmlto --without-fop
                  --disable-specs --disable-malloc0returnsnull
                  --with-keysymdefdir="$STAGE$PREFIX/include/X11" )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
