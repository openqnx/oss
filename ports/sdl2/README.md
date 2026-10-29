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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add sdl2
apk add sdl2-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-sdl2 && /usr/local/bin/qnx-port-tests/test_sdl2    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install sdl2`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sdl2
ls dists/8.0/*/ | grep '^sdl2-'
```

This writes `sdl2-<ver>-r0.tar.gz` (run time) and `sdl2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sdl2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sdl2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sdl2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sdl2`](../../tests/src/) links sdl2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sdl2`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sdl2
/usr/local/bin/qnx-port-tests/test_sdl2
```

```
== sdl2 ==
  ok   ...
PASS sdl2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sdl2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sdl2`](../../tests/src/) -- a program linked against sdl2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - SDL_Init(TIMER|EVENTS)
   - SDL_AddTimer
   - timer callback fired
   - performance counter advances
   - SDL_GetTicks64
   - SDL_PushEvent
   - SDL_PollEvent
   - the event round-tripped
   - SDL_RWFromMem
   - SDL_RWwrite
   - SDL_RWseek
   - SDL_RWread
   - SDL_Swap32
   - SDL_GetCPUCount
   - at least one video driver is compiled in
   - at least one audio driver is compiled in
   - the ALSA backend is built (QNX 8 ships alsa-lib, not QSA)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64 virt, vanilla mkqnximage image | apk:local-dists | sdl2 2.32.10-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sdl2 2.32.10-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **2.32.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sdl2=<version>`), sources in `ported/sdl2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sdl2`; port one with `scripts/port-update sdl2`.

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
