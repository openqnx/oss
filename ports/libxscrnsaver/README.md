# libxscrnsaver 1.2.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libxscrnsaver.git (tag `libXScrnSaver-1.2.4`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) [`libxext`](../libxext/) [`xorgproto`](../xorgproto/) |
| Run-time dependencies | [`libx11`](../libx11/) [`libxext`](../libxext/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libxscrnsaver
oss-get install libxscrnsaver-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxscrnsaver` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libx11 libxext xorgproto
```

<details><summary>Full build chain (9)</summary>

[`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b libXScrnSaver-1.2.4 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/lib/libxscrnsaver.git src/libxscrnsaver
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libxscrnsaver && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libxscrnsaver && cd build/$QNX_ARCH/libxscrnsaver
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxscrnsaver/configure --host=$QNX_TRIPLE --prefix=/usr/local \
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

- The MIT-SCREEN-SAVER extension client library.  Here because SDL3's X11 backend lists xscrnsaver among its hard pkg-config requirements -- it uses it to inhibit the screensaver while a window has focus.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxscrnsaver`.

### 5. What it installs

16 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libXss.so
/usr/local/lib/libXss.so.1
```

The complete list is `stage/<arch>/.manifest/libxscrnsaver` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libxscrnsaver
ls repo/8.0/*/ | grep '^libxscrnsaver-'
```

This writes `libxscrnsaver-<ver>-r0.tar.gz` (run time) and `libxscrnsaver-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libxscrnsaver
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libxscrnsaver-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libxscrnsaver-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.2.4; GIT=https://gitlab.freedesktop.org/xorg/lib/libxscrnsaver.git; REF=libXScrnSaver-1.2.4
DEPS="libx11 libxext xorgproto"
BUILDSYS=autotools
# The MIT-SCREEN-SAVER extension client library.  Here because SDL3's X11
# backend lists xscrnsaver among its hard pkg-config requirements -- it uses
# it to inhibit the screensaver while a window has focus.
CONF_ARGS_EXTRA=( --disable-malloc0returnsnull --enable-shared --disable-static --without-xmlto --without-fop --disable-specs )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
