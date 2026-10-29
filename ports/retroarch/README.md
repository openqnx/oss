# retroarch 1.15-git-20230408

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/RetroPie/RetroArch.git |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`sdl2`](../sdl2/) [`freetype`](../freetype/) [`zlib`](../zlib/) [`fontconfig`](../fontconfig/) [`fribidi`](../fribidi/) [`libass`](../libass/) [`openssl`](../openssl/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`libass`](../libass/) [`libxkbcommon`](../libxkbcommon/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install retroarch
oss-get install retroarch-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild retroarch` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (25 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 freetype zlib fontconfig fribidi libass openssl
```

<details><summary>Full build chain (25)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libass`](../libass/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`sdl2`](../sdl2/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/RetroPie/RetroArch.git src/retroarch
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/retroarch < ports/retroarch/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/retroarch < ports/retroarch/patches/0002-qnx-no-libstdcxx.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — updated drivers

> Imported from qnx-ports/RetroArch (d625ca376267...qnx_master); original authorship kept.
>
>
> ---
>  Makefile.common                   |    9 +
>  frontend/drivers/platform_qnx.c   |   57 +-
>  frontend/drivers/platform_unix.c  |    4 +-
>  gfx/drivers_context/qnx_ctx.c     |  111 ++-
>  input/drivers/qnx_input.c         | 1176 ++++++++++++-----------------
>  input/drivers/qnx_input.h         |   95 +++
>  input/drivers/qnx_input_old.c     |  857 +++++++++++++++++++++
>  input/drivers_joypad/qnx_joypad.c |   10 +-
>  qb/config.libs.sh                 |    4 +
>  qb/qb.system.sh                   |    1 +
>  10 files changed, 1535 insertions(+), 789 deletions(-)
>  create mode 100644 input/drivers/qnx_input.h
>  create mode 100644 input/drivers/qnx_input_old.c

Touches: `Makefile.common`, `frontend/drivers/platform_qnx.c`, `frontend/drivers/platform_unix.c`, `gfx/drivers_context/qnx_ctx.c`, `input/drivers/qnx_input.c`, `input/drivers/qnx_input.h` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-no-libstdcxx.patch`](patches/0002-qnx-no-libstdcxx.patch)** — Do not link GNU libstdc++ on QNX.

> Makefile.common adds -lstdc++ for the switchres code unconditionally, which
> is GNU's C++ runtime. QNX's q++ links its own, libc++, and the objects are
> compiled against it: retroarch ended up loading both libc++.so.2 and the
> SDP's libstdc++.so.6 (scripts/check-sdp-leak), two C++ runtimes with their
> own exception handling and std:: types in one process. Leave it out on QNX.

Touches: `Makefile.common` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/retroarch; B=build/$QNX_ARCH/retroarch; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc" CXX="q++" CFLAGS="-D_QNX_SOURCE -std=c17 -V$QCC_TARGET -I$STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -std=c++17 -V${QCC_TARGET}_cxx -Wno-deprecated-definitions -I$STAGE$PREFIX/include" \
  CPPFLAGS="-D_QNX_SOURCE" LDFLAGS="-V${QCC_TARGET}_cxx -L$STAGE$PREFIX/lib" \
  PKG_CONF_PATH=pkg-config \
  ./configure --host="$QNX_TRIPLE-" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --disable-x11 --disable-wayland --disable-builtinzlib --disable-ffmpeg; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)" EGL_LIBS=-lEGL; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" EGL_LIBS=-lEGL install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- RetroArch, the libretro frontend, as packaged by RetroPie, at the upstream commit qnx-ports' QNX port is based on (patches/: QNX platform support -- Screen/EGL video, input, QNX build settings). Part of the RetroPie stack.
- RetroArch's configure is its own (not autoconf) and builds in the source tree.
- LDFLAGS -V..._cxx: the program is linked by q++, and the plain -V variant is GNU libstdc++'s -- next to the libc++ every C++ object here is compiled against, so the binary loaded both C++ runtimes (scripts/check-sdp-leak). PKG_CONF_PATH: configure looks for <triple>-pkg-config; the tree's pkg-config (PKG_CONFIG_LIBDIR/SYSROOT from env.sh) is the one to use. --disable-ffmpeg: this RetroArch (2023, the RetroPie revision qnx-ports ports) uses the FFmpeg API before 7.0 (AVCodecContext::channel_layout); the tree's FFmpeg is 7. It only provides the ffmpeg core and recording. --disable-builtinzlib: the zlib component (a DEPS entry), not the copy in deps/libz, whose prototypes clash with the staged zlib.h. --disable-x11 --disable-wayland: on QNX RetroArch draws through Screen/SDL2. Its Wayland input maps Linux evdev key codes (<linux/input.h>); its X11 drivers include the XShm one, which needs SysV shared memory (<sys/shm.h>) that QNX does not have. EGL_LIBS=-lEGL: configure finds QNX's EGL (HAVE_EGL) through its headers but has no pkg-config file to take the library from, so the link missed it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH retroarch`.

### 5. What it installs

10 files under `/usr/local`, 2 programs.

```
/usr/local/bin/retroarch
/usr/local/bin/retroarch-cg2glsl
```

The complete list is `stage/<arch>/.manifest/retroarch` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH retroarch
ls repo/8.0/*/ | grep '^retroarch-'
```

This writes `retroarch-<ver>-r0.tar.gz` (run time) and `retroarch-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install retroarch
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/retroarch-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/retroarch-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# RetroArch, the libretro frontend, as packaged by RetroPie, at the upstream
# commit qnx-ports' QNX port is based on (patches/: QNX platform support --
# Screen/EGL video, input, QNX build settings). Part of the RetroPie stack.
VERSION=1.15-git-20230408; GIT=https://github.com/RetroPie/RetroArch.git; COMMIT=d625ca376267cef2c304a8a8f033f21003c8df02
DEPS="sdl2 freetype zlib fontconfig fribidi libass openssl"
BUILDSYS=custom
# RetroArch's configure is its own (not autoconf) and builds in the source tree.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc" CXX="q++" CFLAGS="-D_QNX_SOURCE -std=c17 -V$QCC_TARGET -I$STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -std=c++17 -V${QCC_TARGET}_cxx -Wno-deprecated-definitions -I$STAGE$PREFIX/include" \
  CPPFLAGS="-D_QNX_SOURCE" LDFLAGS="-V${QCC_TARGET}_cxx -L$STAGE$PREFIX/lib" \
  PKG_CONF_PATH=pkg-config \
  ./configure --host="$QNX_TRIPLE-" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --disable-x11 --disable-wayland --disable-builtinzlib --disable-ffmpeg; }
# LDFLAGS -V..._cxx: the program is linked by q++, and the plain -V variant is
# GNU libstdc++'s -- next to the libc++ every C++ object here is compiled
# against, so the binary loaded both C++ runtimes (scripts/check-sdp-leak).
# PKG_CONF_PATH: configure looks for <triple>-pkg-config; the tree's pkg-config
# (PKG_CONFIG_LIBDIR/SYSROOT from env.sh) is the one to use.
# --disable-ffmpeg: this RetroArch (2023, the RetroPie revision qnx-ports
# ports) uses the FFmpeg API before 7.0 (AVCodecContext::channel_layout); the
# tree's FFmpeg is 7. It only provides the ffmpeg core and recording.
# --disable-builtinzlib: the zlib component (a DEPS entry), not the copy in
# deps/libz, whose prototypes clash with the staged zlib.h.
# --disable-x11 --disable-wayland: on QNX RetroArch draws through Screen/SDL2.
# Its Wayland input maps Linux evdev key codes (<linux/input.h>); its X11 drivers
# include the XShm one, which needs SysV shared memory (<sys/shm.h>) that QNX
# does not have.
# EGL_LIBS=-lEGL: configure finds QNX's EGL (HAVE_EGL) through its headers but
# has no pkg-config file to take the library from, so the link missed it.
do_build() { make -C "$B" -j"$(nproc)" EGL_LIBS=-lEGL; }
do_install() { make -C "$B" DESTDIR="$STAGE" EGL_LIBS=-lEGL install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
