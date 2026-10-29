# libxft 2.3.8

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/lib/libxft.git (tag `libXft-2.3.8`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libx11`](../libx11/) [`libxrender`](../libxrender/) [`freetype`](../freetype/) [`fontconfig`](../fontconfig/) [`xorgproto`](../xorgproto/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`libx11`](../libx11/) [`libxrender`](../libxrender/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install libxft
oss-get install libxft-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxft` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (16 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libx11 libxrender freetype fontconfig xorgproto
```

<details><summary>Full build chain (16)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`libpng`](../libpng/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxrender`](../libxrender/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b libXft-2.3.8 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/lib/libxft.git src/libxft
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/libxft && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/libxft && cd build/$QNX_ARCH/libxft
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxft/configure --host=$QNX_TRIPLE --prefix=/usr/local \
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

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxft`.

### 5. What it installs

83 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libXft.so
/usr/local/lib/libXft.so.2
```

The complete list is `stage/<arch>/.manifest/libxft` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH libxft
ls repo/8.0/*/ | grep '^libxft-'
```

This writes `libxft-<ver>-r0.tar.gz` (run time) and `libxft-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install libxft
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/libxft-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/libxft-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.3.8; GIT=https://gitlab.freedesktop.org/xorg/lib/libxft.git; REF=libXft-2.3.8
DEPS="libx11 libxrender freetype fontconfig xorgproto"
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
