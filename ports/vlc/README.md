# vlc 3.0.21

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://code.videolan.org/videolan/vlc.git (tag `3.0.21`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`lz4`](../lz4/) [`openssl`](../openssl/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`lz4`](../lz4/) [`openssl`](../openssl/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install vlc
oss-get install vlc-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild vlc` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild bzip2 libiconv lz4 openssl xz zstd
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`lz4`](../lz4/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`xz`](../xz/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.0.21 --recurse-submodules --shallow-submodules \
    https://code.videolan.org/videolan/vlc.git src/vlc
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/vlc < ports/vlc/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — contrib: sidplay: Do not overwrite CXXFLAGS in

> configure.in
>
> resid configure has a bug which overwrites CXXFLAGS unconditionally,
> causing build failures when preset CXXFLAGS are needed.
>
> Reason: In newer autoconf, ac_test_CXXFLAGS does not use "set"
> anymore, but "y".
>
> ---
>  contrib/src/sidplay2/rules.mak              |  2 ++
>  contrib/src/sidplay2/sidplay2-cxxtest.patch | 11 +++++++++++
>  2 files changed, 13 insertions(+)
>  create mode 100644 contrib/src/sidplay2/sidplay2-cxxtest.patch

Touches: `contrib/src/sidplay2/rules.mak`, `contrib/src/sidplay2/sidplay2-cxxtest.patch`, `modules/hw/d3d11/d3d11_deinterlace.c`, `modules/video_chroma/d3d11_fmt.c`, `modules/video_chroma/d3d11_fmt.h`, `modules/video_output/win32/direct3d11.c` … · Upstream status: QNX-specific (from qnx-ports)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/vlc; B=build/$QNX_ARCH/vlc; mkdir -p $B
```

```bash
do_patch() {
  cd "$SRCROOT" && ./bootstrap || return 1
  local h="$SRCROOT/qnx-host"; mkdir -p "$h/contrib"
  # VLC's contrib Makefile installs into $(PREFIX) when the environment sets
  # it ("PREFIX ?= ..."), and env.sh exports PREFIX=/usr/local for this tree's
  # own installs: the contribs then went to the build host's /usr/local
  # ("mkdir: cannot create directory /usr/local/share/aclocal"). Unset it.
  ( cd "$h/contrib" && "$SRCROOT/contrib/bootstrap" && env -u PREFIX make fetch-all ) || return 1
  # host protoc from the contrib protobuf, with the "don't build protoc" patch reverted
  ( cd "$h/contrib" && env -u PREFIX make protobuf && cd protobuf && \
    patch -p1 -R < "$SRCROOT/contrib/src/protobuf/dont-build-protoc.patch" && autoreconf -vif && \
    ./configure --prefix="$PWD/install" && make -C src protoc && make -C src install && \
    cp install/bin/protoc "$h/protoc" ) || return 1
}
```

```bash
do_configure() {
  local h="$SRCROOT/qnx-host"
  mkdir -p "$B/contrib" && ( cd "$B/contrib" && \
    "$S/contrib/bootstrap" --host="$(_vlc_host)" --disable-qt --disable-qtsvg --disable-xcb --disable-sdl --disable-SDL_image && \
    env -u PREFIX -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_PATH \
      PROTOC="$h/protoc" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
      CFLAGS="-O2 -D_QNX_SOURCE" CXXFLAGS="-O2 -D_QNX_SOURCE" make -j"$(nproc)" $(_vlc_jobserver) ) || return 1
  # VLC itself, like its contribs, takes its libraries from the contrib tree
  # (--with-contrib sets the pkg-config path); env.sh's PKG_CONFIG_* must not
  # apply here either, or the plugins lose their include paths (mpeg2.h) --
  # and pkg-config must still be confined to the contrib tree, or it answers
  # from the build machine (libpulse.pc, sndio).
  mkdir -p "$B/build" && cd "$B/build" && \
  env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" \
  CFLAGS="-D_QNX_SOURCE -O2" CXXFLAGS="-D_QNX_SOURCE -O2" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  EGL_CFLAGS="-DDUMMY_EGL_CFLAG" EGL_LIBS="-lEGL -lscreen" \
  GLES2_CFLAGS="-DDUMMY_GLES2_CFLAG" GLES2_LIBS="-lEGL -lGLESv2 -lscreen" PROTOC="$h/protoc" \
  "$S/configure" --host="$(_vlc_host)" --build=x86_64-linux-gnu --prefix="$PREFIX" \
    --with-contrib="$B/contrib/../$(_vlc_host)" --disable-a52 --disable-xcb --disable-sdl-image --enable-gles2 \
    --disable-qt --disable-skins2 --disable-lua --disable-vdpau \
    --disable-alsa --disable-pulse --disable-sndio --disable-jack
}
```

```bash
do_build() { env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" make -C "$B/build" -j"$(nproc)" $(_vlc_jobserver); }
```

```bash
do_install() { make -C "$B/build" DESTDIR="$STAGE" install && \
  sed -i "s|-L$B/$(_vlc_host)/lib *||g" "$STAGE$PREFIX/lib/pkgconfig/vlc-plugin.pc"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- VLC 3.0.21 with qnx-ports' QNX support (patches/: a Screen + EGL/GLES2 video output, QNX fixes in the core and modules, and QNX recipes in VLC's contrib system). Built the way the fork's qnx/ makefiles do it: 1. ./bootstrap; fetch every contrib tarball (VLC's contrib system checks each one against its own SHA512 list); 2. build protoc natively from the contrib protobuf (the cross build of the protobuf contrib needs a matching protoc); 3. build the contribs for the target, then VLC against them. Audio output modules are off (ALSA's uses IEC958 rates QNX's alsa-lib does not define; Pulse, sndio and JACK do not exist on QNX). The Qt GUI is left out (it needs a Qt 5 contrib build): libvlc, the plugins and the command-line player (cvlc) are built. Audio output is not supported on QNX yet (upstream of this port). Beyond its contribs, VLC links these components' shared libraries (found on the toolchain's default library path), so its package depends on them.
- The contribs find each other through the pkg-config paths VLC's contrib Makefile sets; env.sh's PKG_CONFIG_* (this tree's staging) must not apply -- its sysroot was prefixed to the contribs' own paths ("libopenjp2 >= 2.1.0 not found" for FFmpeg). SDL 1.2 and SDL_image are left out: they only serve the SDL image decoder module, and SDL_image's configure found no SDL.h for the contrib SDL 1.2. The contribs get -D_QNX_SOURCE through CFLAGS/CXXFLAGS (their Meson cross file and configure scripts take them from there): without it QNX's headers hide POSIX interfaces such as clock_gettime, and dav1d's configure stopped. The make that runs first in PATH may be the SDP's 4.4, which hands its children a "fifo:" jobserver; contribs built with CMake run gmake, which can be the build machine's 4.3 and stops on it ("invalid --jobserver-auth string"). The pipe style is understood by both.
- vlc-plugin.pc carries -L for the contrib tree in the build directory (where libintl/libiconv were found): meaningless on any other machine, and scripts/check-artifacts rejects build paths in installed metadata.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH vlc`.

### 5. What it installs

569 files under `/usr/local`: 4 shared-library entries, 4 programs, 98 headers.

```
/usr/local/lib/libvlc.so
/usr/local/lib/libvlc.so.11
/usr/local/lib/libvlccore.so
/usr/local/lib/libvlccore.so.9
/usr/local/bin/cvlc
/usr/local/bin/rvlc
/usr/local/bin/vlc
/usr/local/bin/vlc-wrapper
```

The complete list is `stage/<arch>/.manifest/vlc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH vlc
ls repo/8.0/*/ | grep '^vlc-'
```

This writes `vlc-<ver>-r0.tar.gz` (run time) and `vlc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install vlc
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/vlc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/vlc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.0.21; GIT=https://code.videolan.org/videolan/vlc.git; REF=3.0.21
# VLC 3.0.21 with qnx-ports' QNX support (patches/: a Screen + EGL/GLES2 video
# output, QNX fixes in the core and modules, and QNX recipes in VLC's contrib
# system). Built the way the fork's qnx/ makefiles do it:
#   1. ./bootstrap; fetch every contrib tarball (VLC's contrib system checks
#      each one against its own SHA512 list);
#   2. build protoc natively from the contrib protobuf (the cross build of the
#      protobuf contrib needs a matching protoc);
#   3. build the contribs for the target, then VLC against them.
# Audio output modules are off (ALSA's uses IEC958 rates QNX's alsa-lib does
# not define; Pulse, sndio and JACK do not exist on QNX).
# The Qt GUI is left out (it needs a Qt 5 contrib build): libvlc, the plugins
# and the command-line player (cvlc) are built. Audio output is not supported
# on QNX yet (upstream of this port).
# Beyond its contribs, VLC links these components' shared libraries (found on
# the toolchain's default library path), so its package depends on them.
DEPS="bzip2 libiconv lz4 openssl xz zstd"
BUILDSYS=custom
_vlc_host() { echo "$QNX_TRIPLE"; }
do_patch() {
  cd "$SRCROOT" && ./bootstrap || return 1
  local h="$SRCROOT/qnx-host"; mkdir -p "$h/contrib"
  # VLC's contrib Makefile installs into $(PREFIX) when the environment sets
  # it ("PREFIX ?= ..."), and env.sh exports PREFIX=/usr/local for this tree's
  # own installs: the contribs then went to the build host's /usr/local
  # ("mkdir: cannot create directory /usr/local/share/aclocal"). Unset it.
  ( cd "$h/contrib" && "$SRCROOT/contrib/bootstrap" && env -u PREFIX make fetch-all ) || return 1
  # host protoc from the contrib protobuf, with the "don't build protoc" patch reverted
  ( cd "$h/contrib" && env -u PREFIX make protobuf && cd protobuf && \
    patch -p1 -R < "$SRCROOT/contrib/src/protobuf/dont-build-protoc.patch" && autoreconf -vif && \
    ./configure --prefix="$PWD/install" && make -C src protoc && make -C src install && \
    cp install/bin/protoc "$h/protoc" ) || return 1
}
# The contribs find each other through the pkg-config paths VLC's contrib
# Makefile sets; env.sh's PKG_CONFIG_* (this tree's staging) must not apply --
# its sysroot was prefixed to the contribs' own paths ("libopenjp2 >= 2.1.0
# not found" for FFmpeg).
# SDL 1.2 and SDL_image are left out: they only serve the SDL image decoder
# module, and SDL_image's configure found no SDL.h for the contrib SDL 1.2.
# The contribs get -D_QNX_SOURCE through CFLAGS/CXXFLAGS (their Meson cross
# file and configure scripts take them from there): without it QNX's headers
# hide POSIX interfaces such as clock_gettime, and dav1d's configure stopped.
# The make that runs first in PATH may be the SDP's 4.4, which hands its
# children a "fifo:" jobserver; contribs built with CMake run gmake, which can
# be the build machine's 4.3 and stops on it ("invalid --jobserver-auth
# string"). The pipe style is understood by both.
_vlc_jobserver() { make --help 2>/dev/null | grep -q -- '--jobserver-style' && echo --jobserver-style=pipe; }
do_configure() {
  local h="$SRCROOT/qnx-host"
  mkdir -p "$B/contrib" && ( cd "$B/contrib" && \
    "$S/contrib/bootstrap" --host="$(_vlc_host)" --disable-qt --disable-qtsvg --disable-xcb --disable-sdl --disable-SDL_image && \
    env -u PREFIX -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_PATH \
      PROTOC="$h/protoc" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
      CFLAGS="-O2 -D_QNX_SOURCE" CXXFLAGS="-O2 -D_QNX_SOURCE" make -j"$(nproc)" $(_vlc_jobserver) ) || return 1
  # VLC itself, like its contribs, takes its libraries from the contrib tree
  # (--with-contrib sets the pkg-config path); env.sh's PKG_CONFIG_* must not
  # apply here either, or the plugins lose their include paths (mpeg2.h) --
  # and pkg-config must still be confined to the contrib tree, or it answers
  # from the build machine (libpulse.pc, sndio).
  mkdir -p "$B/build" && cd "$B/build" && \
  env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" \
  CFLAGS="-D_QNX_SOURCE -O2" CXXFLAGS="-D_QNX_SOURCE -O2" CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" \
  EGL_CFLAGS="-DDUMMY_EGL_CFLAG" EGL_LIBS="-lEGL -lscreen" \
  GLES2_CFLAGS="-DDUMMY_GLES2_CFLAG" GLES2_LIBS="-lEGL -lGLESv2 -lscreen" PROTOC="$h/protoc" \
  "$S/configure" --host="$(_vlc_host)" --build=x86_64-linux-gnu --prefix="$PREFIX" \
    --with-contrib="$B/contrib/../$(_vlc_host)" --disable-a52 --disable-xcb --disable-sdl-image --enable-gles2 \
    --disable-qt --disable-skins2 --disable-lua --disable-vdpau \
    --disable-alsa --disable-pulse --disable-sndio --disable-jack
}
do_build() { env -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH PKG_CONFIG_LIBDIR="$B/$(_vlc_host)/lib/pkgconfig" make -C "$B/build" -j"$(nproc)" $(_vlc_jobserver); }
# vlc-plugin.pc carries -L for the contrib tree in the build directory (where
# libintl/libiconv were found): meaningless on any other machine, and
# scripts/check-artifacts rejects build paths in installed metadata.
do_install() { make -C "$B/build" DESTDIR="$STAGE" install && \
  sed -i "s|-L$B/$(_vlc_host)/lib *||g" "$STAGE$PREFIX/lib/pkgconfig/vlc-plugin.pc"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
