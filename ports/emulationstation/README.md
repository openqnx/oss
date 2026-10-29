# emulationstation 2.11-git-20241212

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/RetroPie/EmulationStation.git |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`sdl2`](../sdl2/) [`freeimage`](../freeimage/) [`freetype`](../freetype/) [`curl`](../curl/) [`pugixml`](../pugixml/) [`rapidjson`](../rapidjson/) [`nanosvg`](../nanosvg/) [`vlc`](../vlc/) |
| Run-time dependencies | [`curl`](../curl/) [`freeimage`](../freeimage/) [`freetype`](../freetype/) [`pugixml`](../pugixml/) [`sdl2`](../sdl2/) [`vlc`](../vlc/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install emulationstation
oss-get install emulationstation-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild emulationstation` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (27 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 freeimage freetype curl pugixml rapidjson nanosvg vlc
```

<details><summary>Full build chain (27)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`expat`](../expat/) [`freeimage`](../freeimage/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`lz4`](../lz4/) [`nanosvg`](../nanosvg/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pugixml`](../pugixml/) [`rapidjson`](../rapidjson/) [`sdl2`](../sdl2/) [`vlc`](../vlc/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/RetroPie/EmulationStation.git src/emulationstation
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/emulationstation < ports/emulationstation/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — QNX related changes to commandline and filesystem utils

> Imported from qnx-ports/EmulationStation (51b6f4162d8e...qnx_master); original authorship kept.
>
>
> ---
>  CMakeLists.txt                       | 17 ++++++++++++++---
>  es-app/src/ScraperCmdLine.cpp        |  2 +-
>  es-core/src/utils/FileSystemUtil.cpp |  9 +++++++++
>  3 files changed, 24 insertions(+), 4 deletions(-)

Touches: `CMakeLists.txt`, `es-app/src/ScraperCmdLine.cpp`, `es-core/src/utils/FileSystemUtil.cpp`, `CMakeLists.txt`, `es-app/src/main.cpp`, `es-app/src/views/SystemView.cpp` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/emulationstation -B build/$QNX_ARCH/emulationstation -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DGLES=ON \
      -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON \
      -DCPU="$QNX_CPU" \
      -DSDL2_LIBRARY="$STAGE$PREFIX/lib/libSDL2.so" \
      -DSDL2_INCLUDE_DIR="$STAGE$PREFIX/include/SDL2" \
      -DSDL2MAIN_LIBRARY="$STAGE$PREFIX/lib/libSDL2main.a" \
      -DSDL2_NO_DEFAULT_PATH=ON \
      -DFreeImage_LIBRARY="$STAGE$PREFIX/lib/libfreeimage.so" \
      -DFreeImage_INCLUDE_DIR="$STAGE$PREFIX/include"
cmake --build build/$QNX_ARCH/emulationstation -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/emulationstation
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- EmulationStation, RetroPie's game browser, at the upstream commit qnx-ports' port is based on (patches/: QNX support). Uses GLES2 (from the QNX SDP) and SDL2. Part of the RetroPie stack.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH emulationstation`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/emulationstation
```

The complete list is `stage/<arch>/.manifest/emulationstation` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH emulationstation
ls repo/8.0/*/ | grep '^emulationstation-'
```

This writes `emulationstation-<ver>-r0.tar.gz` (run time) and `emulationstation-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install emulationstation
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/emulationstation-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/emulationstation-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# EmulationStation, RetroPie's game browser, at the upstream commit qnx-ports'
# port is based on (patches/: QNX support). Uses GLES2 (from the QNX SDP) and
# SDL2. Part of the RetroPie stack.
VERSION=2.11-git-20241212; GIT=https://github.com/RetroPie/EmulationStation.git; COMMIT=51b6f4162d8e89be2ef7a2c9d7c232ac5b31022b
DEPS="sdl2 freeimage freetype curl pugixml rapidjson nanosvg vlc"
CMAKE_ARGS_EXTRA=( -DGLES=ON -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON -DCPU="$QNX_CPU"
                   -DSDL2_LIBRARY="$STAGE$PREFIX/lib/libSDL2.so" -DSDL2_INCLUDE_DIR="$STAGE$PREFIX/include/SDL2"
                   -DSDL2MAIN_LIBRARY="$STAGE$PREFIX/lib/libSDL2main.a" -DSDL2_NO_DEFAULT_PATH=ON
                   -DFreeImage_LIBRARY="$STAGE$PREFIX/lib/libfreeimage.so" -DFreeImage_INCLUDE_DIR="$STAGE$PREFIX/include" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
