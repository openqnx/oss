# mame 0.281

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/mamedev/mame.git (tag `mame0281`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`sdl2`](../sdl2/) [`sdl2-ttf`](../sdl2-ttf/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`zlib`](../zlib/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`sdl2`](../sdl2/) [`sdl2-ttf`](../sdl2-ttf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add mame
apk add mame-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install mame`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mame` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (23 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 sdl2-ttf fontconfig freetype zlib
```

<details><summary>Full build chain (23)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`sdl2`](../sdl2/) [`sdl2-ttf`](../sdl2-ttf/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b mame0281 --recurse-submodules --shallow-submodules \
    https://github.com/mamedev/mame.git src/mame
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mame < ports/mame/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — Port mame to QNX 8.0

> ---
>  3rdparty/asio/include/asio/detail/config.hpp  |  5 ++
>  .../include/asio/detail/impl/posix_thread.ipp |  5 ++
>  .../asio/include/asio/detail/socket_types.hpp |  8 +-
>  .../cpp11/local/fd_passing_stream_client.cpp  |  8 ++
>  .../cpp11/local/fd_passing_stream_server.cpp  |  8 ++
>  3rdparty/asio/src/tests/unit/ip/multicast.cpp |  9 ++-
>  .../3rdparty/dear-imgui/widgets/file_list.inl | 47 ++++++++++++
>  .../bgfx/examples/common/entry/entry_sdl.cpp  |  2 +
>  3rdparty/bgfx/include/bgfx/embedded_shader.h  |  1 +
>  3rdparty/bgfx/src/bgfx.cpp                    |  1 +
>  3rdparty/bgfx/src/glcontext_egl.cpp           | 21 +++++-
>  3rdparty/bgfx/src/renderer_gl.h               |  1 +
>  3rdparty/bx/include/bx/platform.h             | 15 ++++
>  3rdparty/bx/src/os.cpp                        | 49 +++++++++++-
>  3rdparty/bx/src/thread.cpp                    |  2 +-
>  3rdparty/genie/src/host/scripts.c             |  4 +-
>  3rdparty/genie/src/tools/gcc.lua              |  2 +-
>  3rdparty/lua/src/lopcodes.h                   | 70 +++++++++---------
>  3rdparty/lzma/C/CpuArch.c                     | 19 ++++-
>  3rdparty/portmidi/pm_common/pmutil.c          | 28 +++----
>  3rdparty/portmidi/pm_linux/pmlinuxalsa.c      | 74 +++++++++----------
>  3rdparty/portmidi/porttime/ptlinux.c          | 26 ++++---
>  makefile                                      | 38 ++++++++--
>  scripts/genie.lua                             | 12 ++-
>  scripts/src/3rdparty.lua                      | 19 ++++-
>  scripts/src/main.lua                          | 19 ++++-
>  scripts/src/osd/modules.lua                   |  8 +-
>  scripts/src/osd/sdl.lua                       | 21 +++++-
>  scripts/toolchain.lua                         | 42 +++++++++++
>  src/devices/cpu/z180/z180.cpp                 |  4 +
>  src/devices/cpu/z180/z180dd.hxx               |  4 +
>  src/devices/cpu/z180/z180fd.hxx               |  4 +
>  src/devices/cpu/z180/z180op.hxx               | 16 ++++
>  src/devices/cpu/z180/z180ops.h                |  8 ++
>  src/osd/modules/file/posixdir.cpp             | 27 ++++++-
>  src/osd/modules/file/posixfile.cpp            | 14 ++--
>  src/osd/modules/file/posixptty.cpp            |  6 +-
>  src/osd/modules/render/drawbgfx.cpp           | 10 +++
>  38 files changed, 514 insertions(+), 143 deletions(-)

Touches: `3rdparty/asio/include/asio/detail/config.hpp`, `3rdparty/asio/include/asio/detail/impl/posix_thread.ipp`, `3rdparty/asio/include/asio/detail/socket_types.hpp`, `3rdparty/asio/src/examples/cpp11/local/fd_passing_stream_client.cpp`, `3rdparty/asio/src/examples/cpp11/local/fd_passing_stream_server.cpp`, `3rdparty/asio/src/tests/unit/ip/multicast.cpp` … · Upstream status: QNX-specific (from qnx-ports)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/mame; B=build/$QNX_ARCH/mame; mkdir -p $B
```

```bash
do_build() {
  local t=$([ "$QNX_ARCH" = aarch64le ] && echo qnx_arm64 || echo qnx_x64)
  cd "$S" && PKG_CONFIG_PATH= make "$t" -j"$(nproc)" TARGETOS=qnx TESTS=0 SYMBOLS=0 DEBUG=0 \
    ARCHOPTS="-D_QNX_SOURCE -DSDL_DISABLE_IMMINTRIN_H -Wno-error=cpp -DCATCH_CONFIG_CPP11_NO_SHUFFLE -Wno-error=format-overflow -Wno-error=alloc-size-larger-than= -Wno-error=parentheses -Wno-error=pointer-arith -Wno-error=unused-value" \
    LDOPTS="-Wl,-rpath-link=$STAGE$PREFIX/lib -L$STAGE$PREFIX/lib -lsocket" SDL_INSTALL_ROOT="$STAGE$PREFIX" \
    USE_QTDEBUG=0 USE_TAPTUN=1 NO_USE_XINPUT=1 NO_X11=1 NO_USE_MIDI=1 USE_SYSTEM_LIB_ZLIB=1 \
    BUILDDIR="$(realpath -m --relative-to="$S" "$B/build")" ${MAME_SUBTARGET:+SUBTARGET=$MAME_SUBTARGET}
}
```

```bash
do_install() {
  local exe=mame; [ -n "$MAME_SUBTARGET" ] && exe=$MAME_SUBTARGET
  install -d "$STAGE$PREFIX/bin" && install -m755 "$S/$exe" "$STAGE$PREFIX/bin/" && rm -f "$S/$exe"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- MAME 0.281 with qnx-ports' QNX support (patches/: a QNX target OS in MAME's GENie build scripts and the qnx_x64/qnx_arm64 make targets, OS fixes in osd/). Video and audio through SDL2. A full build of every driver is very large (hours, tens of GB); set MAME_SUBTARGET (e.g. "tiny") to build a subset.
- -Wno-error=cpp: SDL_config.h (sdl2-compat's, when that is the SDL2 in the tree) carries a #warning, which MAME's -Werror made fatal in osd/sdl.
- BUILDDIR is given relative to the source tree: GENie writes the project files under an absolute BUILDDIR with its leading "/" dropped ("home/qnx/.../gmake-nto-x86_64-o"), and make then cannot find them.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mame`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/mame
```

The complete list is `stage/<arch>/.manifest/mame` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mame
ls dists/8.0/*/ | grep '^mame-'
```

This writes `mame-<ver>-r0.tar.gz` (run time) and `mame-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mame
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mame-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mame-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mame 0.281-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mame 0.281-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.281**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mame=<version>`), sources in `ported/mame/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mame`; port one with `scripts/port-update mame`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.281; GIT=https://github.com/mamedev/mame.git; REF=mame0281
DEPS="sdl2 sdl2-ttf fontconfig freetype zlib"
BUILDSYS=custom
# MAME 0.281 with qnx-ports' QNX support (patches/: a QNX target OS in MAME's
# GENie build scripts and the qnx_x64/qnx_arm64 make targets, OS fixes in
# osd/). Video and audio through SDL2. A full build of every driver is very
# large (hours, tens of GB); set MAME_SUBTARGET (e.g. "tiny") to build a
# subset.
MAME_SUBTARGET=${MAME_SUBTARGET:-}
# -Wno-error=cpp: SDL_config.h (sdl2-compat's, when that is the SDL2 in the
# tree) carries a #warning, which MAME's -Werror made fatal in osd/sdl.
do_build() {
  local t=$([ "$QNX_ARCH" = aarch64le ] && echo qnx_arm64 || echo qnx_x64)
  cd "$S" && PKG_CONFIG_PATH= make "$t" -j"$(nproc)" TARGETOS=qnx TESTS=0 SYMBOLS=0 DEBUG=0 \
    ARCHOPTS="-D_QNX_SOURCE -DSDL_DISABLE_IMMINTRIN_H -Wno-error=cpp -DCATCH_CONFIG_CPP11_NO_SHUFFLE -Wno-error=format-overflow -Wno-error=alloc-size-larger-than= -Wno-error=parentheses -Wno-error=pointer-arith -Wno-error=unused-value" \
    LDOPTS="-Wl,-rpath-link=$STAGE$PREFIX/lib -L$STAGE$PREFIX/lib -lsocket" SDL_INSTALL_ROOT="$STAGE$PREFIX" \
    USE_QTDEBUG=0 USE_TAPTUN=1 NO_USE_XINPUT=1 NO_X11=1 NO_USE_MIDI=1 USE_SYSTEM_LIB_ZLIB=1 \
    BUILDDIR="$(realpath -m --relative-to="$S" "$B/build")" ${MAME_SUBTARGET:+SUBTARGET=$MAME_SUBTARGET}
}
# BUILDDIR is given relative to the source tree: GENie writes the project
# files under an absolute BUILDDIR with its leading "/" dropped
# ("home/qnx/.../gmake-nto-x86_64-o"), and make then cannot find them.
do_install() {
  local exe=mame; [ -n "$MAME_SUBTARGET" ] && exe=$MAME_SUBTARGET
  install -d "$STAGE$PREFIX/bin" && install -m755 "$S/$exe" "$STAGE$PREFIX/bin/" && rm -f "$S/$exe"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
