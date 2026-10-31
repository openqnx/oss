# vlc 3.0.21

> VLC media player and libvlc multimedia engine

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://code.videolan.org/videolan/vlc.git (tag `3.0.21`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`lz4`](../lz4/) [`openssl`](../openssl/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`lz4`](../lz4/) [`openssl`](../openssl/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_vlc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `vlc` (programs, shared libraries, run-time data) and `vlc-dev` (headers, pkg-config/CMake files, static libraries; needs `vlc`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`vlc`](https://openqnx.github.io/oss/package/x86_64/vlc.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vlc.html)) · [`vlc-dev`](https://openqnx.github.io/oss/package/x86_64/vlc-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vlc-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add vlc` (dependencies come along); one version: `apk add vlc=<version>` | `apk del vlc` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install vlc` (dependencies first) | `oss-get remove vlc` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`vlc-dev` the same way, by its own name (`apk add vlc-dev`, `oss-get install vlc-dev`, by hand with its own file and list `vlc-dev.list`); remove it before `vlc`, which it needs.

```sh
# on the QNX target, as root
apk add vlc vlc-dev
apk add qnx-test-vlc && /usr/local/bin/qnx-port-tests/test_vlc    # its on-target test
apk del vlc vlc-dev qnx-test-vlc
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/vlc.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/vlc.list
rm var/lib/manual/vlc.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa vlc
ls dists/8.0/*/ | grep '^vlc-'
```

This writes `vlc-<ver>-r0.tar.gz` (run time) and `vlc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add vlc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/vlc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/vlc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_vlc`](../../tests/src/) links vlc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-vlc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-vlc
/usr/local/bin/qnx-port-tests/test_vlc
```

```
== vlc ==
  ok   ...
PASS vlc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_vlc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_vlc`](../../tests/src/) -- a program linked against vlc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libvlc_get_version is 3.0.21
   - scratch directory
   - wrote 1 s 8 kHz mono WAV
   - libvlc_new
   - libvlc_media_new_path
   - parse request
   - media parsed
   - duration is 1000 ms
   - one audio track, 8000 Hz mono
   - libvlc_media_player_new_from_media
   - libvlc_media_player_play
   - playback reached the end
   - ~8000 samples reached the audio callback
   - decoded peak is the generated 12000
   - vlc -I dummy --aout=afile exits 0
   - afile wrote an 8 kHz mono WAV
   - afile output has a RIFF/WAVE header
   - the vlc program decoded ~8000 samples
   - PCM passed through unchanged

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/vlc-3.0.21-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/vlc-3.0.21-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/vlc-3.0.21-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vlc 3.0.21-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/vlc-3.0.21-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.0.21**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add vlc=<version>`), sources in `ported/vlc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates vlc`; port one with `scripts/port-update vlc`.

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
