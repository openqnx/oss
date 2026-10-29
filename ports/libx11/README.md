# libx11 1.8.10

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libx11.git (tag `libX11-1.8.10`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libxcb`](../libxcb/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) |
| Run-time dependencies | [`libxcb`](../libxcb/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libx11
apk add libx11-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libx11`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libx11 1.8.10-r0 | ✅ PASS | 2026-10-30 |

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
