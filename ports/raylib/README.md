# raylib 6.2

> Simple and easy-to-use library to enjoy videogames programming

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/raysan5/raylib.git (tag `6.2`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | [`sdl2`](../sdl2/) |
| Run-time dependencies | [`sdl2`](../sdl2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_raylib`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `raylib` (programs, shared libraries, run-time data) and `raylib-dev` (headers, pkg-config/CMake files, static libraries; needs `raylib`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`raylib`](https://openqnx.github.io/oss/package/x86_64/raylib.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/raylib.html)) · [`raylib-dev`](https://openqnx.github.io/oss/package/x86_64/raylib-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/raylib-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add raylib` (dependencies come along); one version: `apk add raylib=<version>` | `apk del raylib` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install raylib` (dependencies first) | `oss-get remove raylib` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`raylib-dev` the same way, by its own name (`apk add raylib-dev`, `oss-get install raylib-dev`, by hand with its own file and list `raylib-dev.list`); remove it before `raylib`, which it needs.

```sh
# on the QNX target, as root
apk add raylib raylib-dev
apk add qnx-test-raylib && /usr/local/bin/qnx-port-tests/test_raylib    # its on-target test, from tests-8.0-<arch>-m-z
apk del raylib raylib-dev qnx-test-raylib
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/raylib.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/raylib.list
rm var/lib/manual/raylib.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild raylib` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2
```

<details><summary>Full build chain (19)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sdl2`](../sdl2/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 6.2 --recurse-submodules --shallow-submodules \
    https://github.com/raysan5/raylib.git src/raylib
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/raylib < ports/raylib/patches/0001-miniaudio-qnx-alsa.patch
patch -p1 -d src/raylib < ports/raylib/patches/0002-rlsw-read-screen-pixels.patch
```

**[`0001-miniaudio-qnx-alsa.patch`](patches/0001-miniaudio-qnx-alsa.patch)** — miniaudio: use the ALSA backend on QNX (libasound.so.5), not PulseAudio

> miniaudio (raudio's device layer) enables ALSA only for Linux and turns on
> PulseAudio and JACK for every other Unix, so on QNX it compiled the
> PulseAudio backend (which failed: "unknown type name 'int64_t'") and had no
> backend for the sound system QNX actually has. QNX 8.0 ships alsa-lib, as
> libasound.so.5: enable the ALSA backend on QNX instead of PulseAudio/JACK,
> and add that soname to the libraries the backend dlopens. Loading it at run
> time keeps libraylib loadable on images without libasound (InitAudioDevice
> then reports that there is no playback device).

Touches: `src/external/miniaudio.h` · Upstream status: QNX-specific

**[`0002-rlsw-read-screen-pixels.patch`](patches/0002-rlsw-read-screen-pixels.patch)** — rlReadScreenPixels: the software renderer's pixels are top-down BGRA

> With the software renderer (OPENGL_VERSION=Software), LoadImageFromScreen()
> and TakeScreenshot() returned the frame upside down with red and blue
> swapped. rlReadScreenPixels() treats glReadPixels() as OpenGL's (bottom row
> first, RGBA) and flips the rows; rlsw's swReadPixels() instead hands out
> the framebuffer the way it is presented -- rcore_desktop_sdl copies it to
> the SDL window surface with the same call -- top row first and, with
> RLSW_FRAMEBUFFER_OUTPUT_BGRA (the default), as BGRA. For the software
> renderer, swap red and blue back and do not flip. Found by this tree's
> raylib test, which draws a frame on SDL's dummy driver and reads it back;
> not QNX-specific.

Touches: `src/rlgl.h` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/raylib -B build/$QNX_ARCH/raylib -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_EXAMPLES=OFF \
      -DPLATFORM=SDL \
      -DOPENGL_VERSION=Software \
      -DCMAKE_DISABLE_FIND_PACKAGE_SDL3=ON
cmake --build build/$QNX_ARCH/raylib -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/raylib
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- raylib: a simple C library for videogames programming (2D/3D drawing, textures, text, models, audio, math). Platform layer: SDL2 (PLATFORM=SDL) with raylib's software renderer (rlsw, OPENGL_VERSION=Software). QNX has no OpenGL for Wayland clients -- the SDP's EGL/GLES drive QNX Screen -- so raylib's GLFW and DRM platforms could open no context; on SDL the software renderer draws into the SDL window surface, which works on SDL2's Wayland backend (shared memory) and on its dummy driver (headless). SDL3 is not used: here it is X11-only (find_package would prefer it).
- Against the sdl2 component's own headers and CMake package (its package root), not whatever is in staging: sdl2-compat installs the same files, with a newer version number, over sdl2's there, and a program built against them refuses to run on the sdl2 it is packaged with ("SDL downgrade").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH raylib`.

### 5. What it installs

12 files under `/usr/local`: 3 shared-library entries, 4 headers.

```
/usr/local/lib/libraylib.so
/usr/local/lib/libraylib.so.6.2.0
/usr/local/lib/libraylib.so.620
```

The complete list is `stage/<arch>/.manifest/raylib` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa raylib
ls dists/8.0/*/ | grep '^raylib-'
```

This writes `raylib-<ver>-r0.tar.gz` (run time) and `raylib-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add raylib
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/raylib-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/raylib-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_raylib`](../../tests/src/) links raylib the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-raylib`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-raylib
/usr/local/bin/qnx-port-tests/test_raylib
```

```
== raylib ==
  ok   ...
PASS raylib
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_raylib` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_raylib`](../../tests/src/) -- a program linked against raylib the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temporary directory
   - InitWindow 160x120 on SDL's dummy driver
   - screen 160x120
   - LoadImageFromScreen: 160x120
   - rectangle pixel red
   - circle centre green
   - background blue
   - text drawn with the default font
   - MeasureText with the default font
   - GetTime/WaitTime
   - ExportImage PNG
   - PNG loaded back with the drawn pixels
   - ImageResizeNN to 32x16
   - ImageFlipHorizontal
   - GenImageChecked
   - x cross y = z
   - distance (0,0)-(3,4) = 5
   - rotate (1,0,0) 90 deg about z -> (0,1,0)
   - translate then scale: (1,1,1) -> (4,6,8)
   - quaternion 180 deg about z: (1,0,0) -> (-1,0,0)
   - Clamp, Lerp, Remap
   - TextFormat, TextToInteger
   - TextSplit into 3
   - CRC-32(\"123456789\") = cbf43926
   - MD5(\"abc\") = 900150983cd24fb0d6963f7d28e17f72
   - ... and 8 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | raylib 6.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/raylib-6.2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | raylib 6.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/raylib-6.2-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add raylib=<version>`), sources in `ported/raylib/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates raylib`; port one with `scripts/port-update raylib`.

<details><summary>recipe.sh</summary>

```bash
# raylib: a simple C library for videogames programming (2D/3D drawing,
# textures, text, models, audio, math).
# Platform layer: SDL2 (PLATFORM=SDL) with raylib's software renderer
# (rlsw, OPENGL_VERSION=Software). QNX has no OpenGL for Wayland clients --
# the SDP's EGL/GLES drive QNX Screen -- so raylib's GLFW and DRM platforms
# could open no context; on SDL the software renderer draws into the SDL
# window surface, which works on SDL2's Wayland backend (shared memory) and
# on its dummy driver (headless). SDL3 is not used: here it is X11-only
# (find_package would prefer it).
VERSION=6.2; GIT=https://github.com/raysan5/raylib.git; REF=6.2
DEPS="sdl2"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF
                   -DPLATFORM=SDL -DOPENGL_VERSION=Software
                   -DCMAKE_DISABLE_FIND_PACKAGE_SDL3=ON )
# Against the sdl2 component's own headers and CMake package (its package
# root), not whatever is in staging: sdl2-compat installs the same files,
# with a newer version number, over sdl2's there, and a program built against
# them refuses to run on the sdl2 it is packaged with ("SDL downgrade").
CMAKE_ARGS_EXTRA+=( -DSDL2_DIR="$WS/pkgroot/$QNX_ARCH/sdl2$PREFIX/lib/cmake/SDL2" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
