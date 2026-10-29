# sdl3 3.4.18

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libsdl-org/SDL.git (tag `release-3.4.18`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`libx11`](../libx11/) [`libxext`](../libxext/) [`libxcursor`](../libxcursor/) [`libxi`](../libxi/) [`libxfixes`](../libxfixes/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxscrnsaver`](../libxscrnsaver/) [`libxkbcommon`](../libxkbcommon/) [`libiconv`](../libiconv/) [`libxtst`](../libxtst/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`libx11`](../libx11/) [`libxcursor`](../libxcursor/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxi`](../libxi/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxscrnsaver`](../libxscrnsaver/) [`libxtst`](../libxtst/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add sdl3
apk add sdl3-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install sdl3`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sdl3` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (24 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst
```

<details><summary>Full build chain (24)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcursor`](../libxcursor/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxi`](../libxi/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxscrnsaver`](../libxscrnsaver/) [`libxtst`](../libxtst/) [`ncurses`](../ncurses/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-3.4.18 --recurse-submodules --shallow-submodules \
    https://github.com/libsdl-org/SDL.git src/sdl3
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/sdl3 < ports/sdl3/patches/0001-qnx8-use-alsa-not-qsa.patch
```

**[`0001-qnx8-use-alsa-not-qsa.patch`](patches/0001-qnx8-use-alsa-not-qsa.patch)** — QNX 8.0 has alsa-lib, not QSA -- do not force the QSA audio backend.

> CMakeLists.txt selects src/audio/qnx unconditionally for QNX.  That
> backend is written against <sys/asoundlib.h> and snd_pcm_open_preferred(),
> the QNX 6.x/7.x sound API.  QNX 8.0 replaced QSA with a real alsa-lib, so
> the header does not exist and the build stops at:
>
>     src/audio/qnx/SDL_qsa_audio.c:37:10: fatal error:
>         sys/asoundlib.h: No such file or directory
>
> CheckALSA() runs a few lines later and finds the alsa-lib that QNX 8 does
> ship, so the generic ALSA backend covers this platform properly.  Gating
> the QSA block on QNX_USE_QSA_AUDIO keeps it available for older QNX
> releases without breaking QNX 8.  Same change as the SDL2 recipe here.

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/sdl3 -B build/$QNX_ARCH/sdl3 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSDL_SHARED=ON \
      -DSDL_STATIC=OFF \
      -DSDL_TEST_LIBRARY=OFF \
      -DSDL_TESTS=OFF \
      -DSDL_EXAMPLES=OFF \
      -DSDL_X11=ON \
      -DSDL_X11_SHARED=OFF \
      -DSDL_WAYLAND=OFF \
      -DSDL_KMSDRM=OFF \
      -DSDL_RPI=OFF \
      -DSDL_OPENGL=OFF \
      -DSDL_OPENGLES=OFF \
      -DSDL_VULKAN=OFF \
      -DSDL_PULSEAUDIO=OFF \
      -DSDL_ALSA=ON \
      -DSDL_ALSA_SHARED=OFF \
      -DSDL_JACK=OFF \
      -DSDL_PIPEWIRE=OFF \
      -DSDL_SNDIO=OFF \
      -DSDL_LIBUDEV=OFF \
      -DSDL_DBUS=OFF \
      -DSDL_IBUS=OFF
cmake --build build/$QNX_ARCH/sdl3 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/sdl3
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libiconv libxtst
- X11, not Wayland -- the opposite of the SDL2 recipe here, and not a preference.  SDL3's Wayland check requires "wayland-egl" and "egl" in its pkg-config spec unconditionally (cmake/sdlchecks.cmake, CheckWayland), and there is no EGL in this tree: QNX has no Mesa, and its own EGL belongs to Screen, which is exactly the kind of SDP prebuilt this workspace must not link.  SDL3's X11 backend has no such requirement, and the full X11 client stack is staged -- so X11 is the backend that can actually be built from source here.  GL/GLES/Vulkan stay off for the same EGL reason; SDL3 keeps its software renderer, which is what runs.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sdl3`.

### 5. What it installs

85 files under `/usr/local`: 3 shared-library entries, 75 headers.

```
/usr/local/lib/libSDL3.so
/usr/local/lib/libSDL3.so.0
/usr/local/lib/libSDL3.so.0.4.18
```

The complete list is `stage/<arch>/.manifest/sdl3` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sdl3
ls dists/8.0/*/ | grep '^sdl3-'
```

This writes `sdl3-<ver>-r0.tar.gz` (run time) and `sdl3-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sdl3
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sdl3-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sdl3-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **3.4.18**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sdl3=<version>`), sources in `ported/sdl3/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sdl3`; port one with `scripts/port-update sdl3`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.4.18; GIT=https://github.com/libsdl-org/SDL.git; REF=release-3.4.18
# Also linked at run time (found by scripts/gen-dependency-index): libiconv libxtst
DEPS="libx11 libxext libxcursor libxi libxfixes libxrandr libxrender libxscrnsaver libxkbcommon libiconv libxtst"
BUILDSYS=cmake
# X11, not Wayland -- the opposite of the SDL2 recipe here, and not a
# preference.  SDL3's Wayland check requires "wayland-egl" and "egl" in its
# pkg-config spec unconditionally (cmake/sdlchecks.cmake, CheckWayland), and
# there is no EGL in this tree: QNX has no Mesa, and its own EGL belongs to
# Screen, which is exactly the kind of SDP prebuilt this workspace must not
# link.  SDL3's X11 backend has no such requirement, and the full X11 client
# stack is staged -- so X11 is the backend that can actually be built from
# source here.  GL/GLES/Vulkan stay off for the same EGL reason; SDL3 keeps
# its software renderer, which is what runs.
CMAKE_ARGS_EXTRA=( -DSDL_SHARED=ON -DSDL_STATIC=OFF
                   -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
                   -DSDL_X11=ON -DSDL_X11_SHARED=OFF -DSDL_WAYLAND=OFF
                   -DSDL_KMSDRM=OFF -DSDL_RPI=OFF
                   -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -DSDL_VULKAN=OFF
                   -DSDL_PULSEAUDIO=OFF -DSDL_ALSA=ON -DSDL_ALSA_SHARED=OFF
                   -DSDL_JACK=OFF -DSDL_PIPEWIRE=OFF -DSDL_SNDIO=OFF
                   -DSDL_LIBUDEV=OFF -DSDL_DBUS=OFF -DSDL_IBUS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
