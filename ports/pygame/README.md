# pygame 2.6.1_git20251005

> Python game development library on SDL2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/pygame/pygame.git |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`python3`](../python3/) [`numpy`](../numpy/) [`sdl2`](../sdl2/) [`sdl2-image`](../sdl2-image/) [`sdl2-ttf`](../sdl2-ttf/) [`sdl2-mixer`](../sdl2-mixer/) [`freetype`](../freetype/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) |
| Run-time dependencies | [`freetype`](../freetype/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`sdl2`](../sdl2/) [`sdl2-image`](../sdl2-image/) [`sdl2-mixer`](../sdl2-mixer/) [`sdl2-ttf`](../sdl2-ttf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pygame`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `pygame` (programs, shared libraries, run-time data) and `pygame-dev` (headers, pkg-config/CMake files, static libraries; needs `pygame`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`pygame`](https://openqnx.github.io/oss/package/x86_64/pygame.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pygame.html)) · [`pygame-dev`](https://openqnx.github.io/oss/package/x86_64/pygame-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/pygame-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add pygame` (dependencies come along); one version: `apk add pygame=<version>` | `apk del pygame` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install pygame` (dependencies first) | `oss-get remove pygame` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`pygame-dev` the same way, by its own name (`apk add pygame-dev`, `oss-get install pygame-dev`, by hand with its own file and list `pygame-dev.list`); remove it before `pygame`, which it needs.

```sh
# on the QNX target, as root
apk add pygame pygame-dev
apk add qnx-test-pygame && /usr/local/bin/qnx-port-tests/test_pygame    # its on-target test, from tests-8.0-<arch>-m-z
apk del pygame pygame-dev qnx-test-pygame
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/pygame.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/pygame.list
rm var/lib/manual/pygame.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pygame` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (46 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 numpy sdl2 sdl2-image sdl2-ttf sdl2-mixer freetype libpng libjpeg-turbo
```

<details><summary>Full build chain (46)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`dav1d`](../dav1d/) [`expat`](../expat/) [`freetype`](../freetype/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`highway`](../highway/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libyuv`](../libyuv/) [`ncurses`](../ncurses/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`openssl`](../openssl/) [`opus`](../opus/) [`opusfile`](../opusfile/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sdl2`](../sdl2/) [`sdl2-image`](../sdl2-image/) [`sdl2-mixer`](../sdl2-mixer/) [`sdl2-ttf`](../sdl2-ttf/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/pygame/pygame.git src/pygame
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pygame < ports/pygame/patches/0001-qnx-config-from-qnx-ports.patch
patch -p1 -d src/pygame < ports/pygame/patches/0002-qnx-cross-build.patch
```

**[`0001-qnx-config-from-qnx-ports.patch`](patches/0001-qnx-config-from-qnx-ports.patch)** — QNX build configuration for pygame, from qnx-ports

> Imported from github.com/qnx-ports/pygame, branch john_qnx_support
> (commits 03675ae, 3bafe89, c1d266b on top of main 85fda3f, John at QNX,
> 2026-04-16): buildconfig/config_qnx.py -- the Unix configuration without
> X11 (pygame.scrap through SDL2's clipboard instead of libX11) and without
> the Linux V4L2 camera (an empty Setup_QNX.in) -- and the dispatch to it in
> buildconfig/config.py when sys.platform is 'qnx8'. Adapted for this tree's
> cross build in the next patch.

Touches: `buildconfig/Setup_QNX.in`, `buildconfig/config.py`, `buildconfig/config_qnx.py` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-cross-build.patch`](patches/0002-qnx-cross-build.patch)** — QNX configuration: sys.platform 'qnx' and cross builds

> qnx-ports' configuration (0001) is selected for sys.platform 'qnx8' and
> searches /usr and /usr/local, as a build on the target does. This tree's
> Python reports sys.platform 'qnx', and it cross compiles: setup.py runs on
> the build machine, whose sys.platform is 'linux' (so the Linux
> configuration, with the V4L2 camera and X11 scrap, was chosen), and whose
> /usr/include and /usr/lib would be found -- last, so winning -- next to the
> staging tree given as LOCALBASE.
>
> Select the QNX configuration for any sys.platform starting with 'qnx', or
> when _PYTHON_HOST_PLATFORM names QNX (the cross build); then search only
> LOCALBASE/PYGAME_EXTRA_BASE, skip the multiarch probe of the build
> machine's gcc, and do not add /usr/X11R6/include to SDL's flags.

Touches: `buildconfig/config.py`, `buildconfig/config_qnx.py` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pygame; B=build/$QNX_ARCH/pygame; mkdir -p $B
```

```bash
do_configure() {
  qnx_py_venv "setuptools>=65" "Cython>=3.0,<3.2" || return 1
  # The SDL2 headers: the sdl2 component's own (its package root), with the
  # SDL_image/ttf/mixer headers next to them, in a directory of the build's.
  # Staging can hold sdl2-compat's copies of the same files instead (a newer
  # version number): pygame compares the SDL version it was compiled with
  # against the library at import and refuses to run on an older one ("SDL
  # downgrade"), so it must be built against the sdl2 it depends on.
  local sdl="$WS/pkgroot/$QNX_ARCH/sdl2$PREFIX" h
  [ -f "$sdl/include/SDL2/SDL_version.h" ] || { echo "sdl2 package root missing: $sdl"; return 1; }
  rm -rf "$B/sdl2inc" && mkdir -p "$B/sdl2inc/SDL2" && cp -a "$sdl/include/SDL2/." "$B/sdl2inc/SDL2/" || return 1
  for h in SDL_image.h SDL_ttf.h SDL_mixer.h; do cp "$STAGE$PREFIX/include/SDL2/$h" "$B/sdl2inc/SDL2/" || return 1; done
  # sdl2-config for the configuration, answering in the three-call form
  # config_qnx.py expects (pkg-config fails on sdl2.pc's Requires.private
  # alsa, which the SDP provides without a .pc file).
  local v; v=$(sed -n 's/^#define SDL_\(MAJOR_VERSION\|MINOR_VERSION\|PATCHLEVEL\) *\([0-9]*\).*/\2/p' "$sdl/include/SDL2/SDL_version.h" | paste -sd.) || return 1
  cat > "$B/sdl2-config" <<S
#!/bin/sh
case "\$1" in
  --version) echo $v ;;
  --cflags) echo -I$B/sdl2inc/SDL2 -I$STAGE$PREFIX/include -D_REENTRANT ;;
  --libs) echo -L$sdl/lib -L$STAGE$PREFIX/lib -lSDL2 ;;
esac
S
  chmod +x "$B/sdl2-config"
  rm -f "$S/Setup"
}
```

```bash
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  LOCALBASE="$STAGE$PREFIX" SDL_CONFIG="$B/sdl2-config" PORTMIDI_INC_PORTTIME=0 \
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
```

```bash
do_install() { qnx_py_install && qnx_py_install_data; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pygame: Python game library on SDL2 -- surfaces, drawing, image load/save (SDL2_image), fonts (SDL2_ttf, FreeType), sound (SDL2_mixer), events, joysticks, and Wayland windows through SDL2's Wayland backend. The QNX build configuration is qnx-ports' (patches/0001), adapted for a cross build (0002). Built from the commit qnx-ports ported (pygame main after 2.6.1, which has the Python 3.12+ fixes 2.6.1 lacks); the Cython modules (_sdl2, pypm, _sprite) are generated from it on the build machine. Without portmidi (not ported) pygame.midi is left out; there is no camera module (Linux V4L2).
- numpy is optional at run time (pygame.surfarray, sndarray) but built against.
- The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
- !/bin/sh
- LOCALBASE: where config_qnx.py looks for headers and libraries (0002 stops it adding the build machine's /usr and /usr/local when cross compiling).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pygame`.

### 5. What it installs

352 files under `/usr/local`, 31 headers.

The complete list is `stage/<arch>/.manifest/pygame` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pygame
ls dists/8.0/*/ | grep '^pygame-'
```

This writes `pygame-<ver>-r0.tar.gz` (run time) and `pygame-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pygame
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pygame-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pygame-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pygame`](../../tests/src/) links pygame the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pygame`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pygame
/usr/local/bin/qnx-port-tests/test_pygame
```

```
== pygame ==
  ok   ...
PASS pygame
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pygame` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pygame`](../../tests/src/) -- a program linked against pygame the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pygame 2.6.1_git20251005-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/pygame-2.6.1_git20251005-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pygame 2.6.1_git20251005-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/pygame-2.6.1_git20251005-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.6.1_git20251005**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pygame=<version>`), sources in `ported/pygame/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pygame`; port one with `scripts/port-update pygame`.

<details><summary>recipe.sh</summary>

```bash
# pygame: Python game library on SDL2 -- surfaces, drawing, image load/save
# (SDL2_image), fonts (SDL2_ttf, FreeType), sound (SDL2_mixer), events,
# joysticks, and Wayland windows through SDL2's Wayland backend.
# The QNX build configuration is qnx-ports' (patches/0001), adapted for a
# cross build (0002). Built from the commit qnx-ports ported (pygame main
# after 2.6.1, which has the Python 3.12+ fixes 2.6.1 lacks); the Cython
# modules (_sdl2, pypm, _sprite) are generated from it on the build machine.
# Without portmidi (not ported) pygame.midi is left out; there is no camera
# module (Linux V4L2).
VERSION=2.6.1_git20251005; GIT=https://github.com/pygame/pygame.git; COMMIT=85fda3f719d437cf27106afae8c890e6b88ba5f5
DEPS="python3 numpy sdl2 sdl2-image sdl2-ttf sdl2-mixer freetype libpng libjpeg-turbo"
# numpy is optional at run time (pygame.surfarray, sndarray) but built against.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
# The wheel's .data/ (share/ files, scripts, headers) goes where pip puts it.
source "$WS/scripts/lib/pywheel-data.sh"
do_configure() {
  qnx_py_venv "setuptools>=65" "Cython>=3.0,<3.2" || return 1
  # The SDL2 headers: the sdl2 component's own (its package root), with the
  # SDL_image/ttf/mixer headers next to them, in a directory of the build's.
  # Staging can hold sdl2-compat's copies of the same files instead (a newer
  # version number): pygame compares the SDL version it was compiled with
  # against the library at import and refuses to run on an older one ("SDL
  # downgrade"), so it must be built against the sdl2 it depends on.
  local sdl="$WS/pkgroot/$QNX_ARCH/sdl2$PREFIX" h
  [ -f "$sdl/include/SDL2/SDL_version.h" ] || { echo "sdl2 package root missing: $sdl"; return 1; }
  rm -rf "$B/sdl2inc" && mkdir -p "$B/sdl2inc/SDL2" && cp -a "$sdl/include/SDL2/." "$B/sdl2inc/SDL2/" || return 1
  for h in SDL_image.h SDL_ttf.h SDL_mixer.h; do cp "$STAGE$PREFIX/include/SDL2/$h" "$B/sdl2inc/SDL2/" || return 1; done
  # sdl2-config for the configuration, answering in the three-call form
  # config_qnx.py expects (pkg-config fails on sdl2.pc's Requires.private
  # alsa, which the SDP provides without a .pc file).
  local v; v=$(sed -n 's/^#define SDL_\(MAJOR_VERSION\|MINOR_VERSION\|PATCHLEVEL\) *\([0-9]*\).*/\2/p' "$sdl/include/SDL2/SDL_version.h" | paste -sd.) || return 1
  cat > "$B/sdl2-config" <<S
#!/bin/sh
case "\$1" in
  --version) echo $v ;;
  --cflags) echo -I$B/sdl2inc/SDL2 -I$STAGE$PREFIX/include -D_REENTRANT ;;
  --libs) echo -L$sdl/lib -L$STAGE$PREFIX/lib -lSDL2 ;;
esac
S
  chmod +x "$B/sdl2-config"
  rm -f "$S/Setup"
}
# LOCALBASE: where config_qnx.py looks for headers and libraries (0002 stops
# it adding the build machine's /usr and /usr/local when cross compiling).
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  LOCALBASE="$STAGE$PREFIX" SDL_CONFIG="$B/sdl2-config" PORTMIDI_INC_PORTTIME=0 \
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install && qnx_py_install_data; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
