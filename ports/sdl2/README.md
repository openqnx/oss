# sdl2 2.32.10

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/libsdl-org/SDL.git (tag `release-2.32.10`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`wayland`](../wayland/) [`libxkbcommon`](../libxkbcommon/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sdl2`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install sdl2
oss-get install sdl2-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sdl2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild wayland libxkbcommon
```

<details><summary>Full build chain (11)</summary>

[`expat`](../expat/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-2.32.10 --recurse-submodules --shallow-submodules \
    https://github.com/libsdl-org/SDL.git src/sdl2
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/sdl2 < ports/sdl2/patches/0001-qnx8-use-alsa-not-qsa.patch
```

**[`0001-qnx8-use-alsa-not-qsa.patch`](patches/0001-qnx8-use-alsa-not-qsa.patch)** — SDL2: use the ALSA backend on QNX 8.0.

> The QSA backend targets the <sys/asoundlib.h> of QNX 6.x/7.x. QNX 8.0 ships
> alsa-lib (<alsa/asoundlib.h>, libasound.so.5), which SDL's generic ALSA backend
> uses; keep QSA behind QNX_USE_QSA_AUDIO.

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/sdl2 -B build/$QNX_ARCH/sdl2 -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DSDL_SHARED=ON \
      -DSDL_STATIC=OFF \
      -DSDL_TEST=OFF \
      -DSDL_X11=OFF \
      -DSDL_WAYLAND=ON \
      -DSDL_WAYLAND_SHARED=OFF \
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
cmake --build build/$QNX_ARCH/sdl2 -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/sdl2
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- sdl2-compat installs the same libSDL2-2.0 and headers: install one or the other.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sdl2`.

### 5. What it installs

95 files under `/usr/local`: 4 shared-library entries, 1 programs, 79 headers.

```
/usr/local/lib/libSDL2-2.0.so
/usr/local/lib/libSDL2-2.0.so.0
/usr/local/lib/libSDL2-2.0.so.0.3200.10
/usr/local/lib/libSDL2.so
/usr/local/bin/sdl2-config
```

The complete list is `stage/<arch>/.manifest/sdl2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH sdl2
ls repo/8.0/*/ | grep '^sdl2-'
```

This writes `sdl2-<ver>-r0.tar.gz` (run time) and `sdl2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install sdl2
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/sdl2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/sdl2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_sdl2`](../../tests/src/) links sdl2 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sdl2 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_sdl2'
```

```
== sdl2 ==
  ok   ...
PASS sdl2
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.32.10; GIT=https://github.com/libsdl-org/SDL.git; REF=release-2.32.10
# sdl2-compat installs the same libSDL2-2.0 and headers: install one or the other.
CONFLICTS="sdl2-compat"
DEPS="wayland libxkbcommon"
CMAKE_ARGS_EXTRA=( -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF
                   -DSDL_X11=OFF -DSDL_WAYLAND=ON -DSDL_WAYLAND_SHARED=OFF
                   -DSDL_KMSDRM=OFF -DSDL_RPI=OFF -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF
                   -DSDL_VULKAN=OFF -DSDL_PULSEAUDIO=OFF -DSDL_ALSA=ON -DSDL_ALSA_SHARED=OFF
                   -DSDL_JACK=OFF -DSDL_PIPEWIRE=OFF -DSDL_SNDIO=OFF
                   -DSDL_LIBUDEV=OFF -DSDL_DBUS=OFF -DSDL_IBUS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
