# sdl2-mixer 2.8.2

> Audio mixer library for SDL2 (WAV, FLAC, MP3, Ogg Vorbis, Opus, MIDI)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libsdl-org/SDL_mixer.git (tag `release-2.8.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`sdl2`](../sdl2/) [`opusfile`](../opusfile/) |
| Run-time dependencies | [`opusfile`](../opusfile/) [`sdl2`](../sdl2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sdl2_mixer`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `sdl2-mixer` (programs, shared libraries, run-time data) and `sdl2-mixer-dev` (headers, pkg-config/CMake files, static libraries; needs `sdl2-mixer`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`sdl2-mixer`](https://openqnx.github.io/oss/package/x86_64/sdl2-mixer.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sdl2-mixer.html)) · [`sdl2-mixer-dev`](https://openqnx.github.io/oss/package/x86_64/sdl2-mixer-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sdl2-mixer-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add sdl2-mixer` (dependencies come along); one version: `apk add sdl2-mixer=<version>` | `apk del sdl2-mixer` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install sdl2-mixer` (dependencies first) | `oss-get remove sdl2-mixer` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`sdl2-mixer-dev` the same way, by its own name (`apk add sdl2-mixer-dev`, `oss-get install sdl2-mixer-dev`, by hand with its own file and list `sdl2-mixer-dev.list`); remove it before `sdl2-mixer`, which it needs.

```sh
# on the QNX target, as root
apk add sdl2-mixer sdl2-mixer-dev
apk add qnx-test-sdl2-mixer && /usr/local/bin/qnx-port-tests/test_sdl2_mixer    # its on-target test, from tests-8.0-<arch>-m-z
apk del sdl2-mixer sdl2-mixer-dev qnx-test-sdl2-mixer
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/sdl2-mixer.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/sdl2-mixer.list
rm var/lib/manual/sdl2-mixer.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sdl2-mixer` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (22 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 opusfile
```

<details><summary>Full build chain (22)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`opus`](../opus/) [`opusfile`](../opusfile/) [`python3`](../python3/) [`readline`](../readline/) [`sdl2`](../sdl2/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-2.8.2 --recurse-submodules --shallow-submodules \
    https://github.com/libsdl-org/SDL_mixer.git src/sdl2-mixer
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/sdl2-mixer -B build/$QNX_ARCH/sdl2-mixer -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DSDL2MIXER_SAMPLES=OFF \
      -DSDL2MIXER_VENDORED=OFF \
      -DSDL2MIXER_DEPS_SHARED=OFF \
      -DSDL2MIXER_FLAC=ON \
      -DSDL2MIXER_FLAC_DRFLAC=ON \
      -DSDL2MIXER_FLAC_LIBFLAC=OFF \
      -DSDL2MIXER_MP3=ON \
      -DSDL2MIXER_MP3_MINIMP3=ON \
      -DSDL2MIXER_MP3_MPG123=OFF \
      -DSDL2MIXER_VORBIS=STB \
      -DSDL2MIXER_OPUS=ON \
      -DSDL2MIXER_MOD=OFF \
      -DSDL2MIXER_GME=OFF \
      -DSDL2MIXER_WAVPACK=OFF \
      -DSDL2MIXER_MIDI=ON \
      -DSDL2MIXER_MIDI_TIMIDITY=ON \
      -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF
cmake --build build/$QNX_ARCH/sdl2-mixer -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/sdl2-mixer
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- SDL_mixer 2: multi-channel audio mixing and music playback on SDL2 (needed by pygame.mixer). Decoders: WAV/AIFF/VOC (built in), FLAC (dr_flac), MP3 (minimp3), Ogg Vorbis (stb_vorbis), MIDI (Timidity) -- all bundled with SDL_mixer, no library needed -- and Opus through the ported opusfile, linked rather than dlopened (SDL2MIXER_DEPS_SHARED=OFF: what is loaded at run time is invisible to the package's dependencies). MOD (libxmp), WavPack and game-music-emu are off: not ported here.
- Against the sdl2 component's own headers and CMake package (its package root), not whatever is in staging: sdl2-compat installs the same files, with a newer version number, over sdl2's there, and a program built against them refuses to run on the sdl2 it is packaged with ("SDL downgrade").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sdl2-mixer`.

### 5. What it installs

11 files under `/usr/local`: 4 shared-library entries, 1 headers.

```
/usr/local/lib/libSDL2_mixer-2.0.so
/usr/local/lib/libSDL2_mixer-2.0.so.0
/usr/local/lib/libSDL2_mixer-2.0.so.0.800.2
/usr/local/lib/libSDL2_mixer.so
```

The complete list is `stage/<arch>/.manifest/sdl2-mixer` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sdl2-mixer
ls dists/8.0/*/ | grep '^sdl2-mixer-'
```

This writes `sdl2-mixer-<ver>-r0.tar.gz` (run time) and `sdl2-mixer-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sdl2-mixer
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sdl2-mixer-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sdl2-mixer-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sdl2_mixer`](../../tests/src/) links sdl2-mixer the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sdl2-mixer`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sdl2-mixer
/usr/local/bin/qnx-port-tests/test_sdl2_mixer
```

```
== sdl2-mixer ==
  ok   ...
PASS sdl2-mixer
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sdl2_mixer` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sdl2_mixer`](../../tests/src/) -- a program linked against sdl2-mixer the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - linked version matches the header
   - SDL_Init(AUDIO) with the dummy driver
   - FLAC, MP3, OGG and Opus support initialised
   - Mix_OpenAudioDevice 22050 Hz S16 stereo
   - Mix_QuerySpec: 22050, S16, 2
   - music decoders include WAVE, OGG and OPUS
   - Mix_LoadWAV_RW of an 11025 Hz mono WAV
   - converted to the device format: 0.5 s = 44100 bytes
   - Mix_PlayChannel: playing
   - channel volume 64
   - chunk volume 128 -> 32
   - Mix_SetPanning hard left
   - post-mix callback saw the mixed tone
   - Mix_HaltChannel
   - WAV loaded as music: MUS_WAV
   - music duration 0.5 s
   - Mix_PlayMusic
   - 16 mixing channels

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sdl2-mixer 2.8.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/sdl2-mixer-2.8.2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sdl2-mixer 2.8.2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/sdl2-mixer-2.8.2-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.8.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sdl2-mixer=<version>`), sources in `ported/sdl2-mixer/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sdl2-mixer`; port one with `scripts/port-update sdl2-mixer`.

<details><summary>recipe.sh</summary>

```bash
# SDL_mixer 2: multi-channel audio mixing and music playback on SDL2 (needed
# by pygame.mixer). Decoders: WAV/AIFF/VOC (built in), FLAC (dr_flac), MP3
# (minimp3), Ogg Vorbis (stb_vorbis), MIDI (Timidity) -- all bundled with
# SDL_mixer, no library needed -- and Opus through the ported opusfile,
# linked rather than dlopened (SDL2MIXER_DEPS_SHARED=OFF: what is loaded at
# run time is invisible to the package's dependencies). MOD (libxmp),
# WavPack and game-music-emu are off: not ported here.
VERSION=2.8.2; GIT=https://github.com/libsdl-org/SDL_mixer.git; REF=release-2.8.2
SRCROOT="$SRC_DIR/sdl2-mixer"
DEPS="sdl2 opusfile"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DSDL2MIXER_SAMPLES=OFF -DSDL2MIXER_VENDORED=OFF
                   -DSDL2MIXER_DEPS_SHARED=OFF
                   -DSDL2MIXER_FLAC=ON -DSDL2MIXER_FLAC_DRFLAC=ON -DSDL2MIXER_FLAC_LIBFLAC=OFF
                   -DSDL2MIXER_MP3=ON -DSDL2MIXER_MP3_MINIMP3=ON -DSDL2MIXER_MP3_MPG123=OFF
                   -DSDL2MIXER_VORBIS=STB -DSDL2MIXER_OPUS=ON
                   -DSDL2MIXER_MOD=OFF -DSDL2MIXER_GME=OFF -DSDL2MIXER_WAVPACK=OFF
                   -DSDL2MIXER_MIDI=ON -DSDL2MIXER_MIDI_TIMIDITY=ON -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF )
# Against the sdl2 component's own headers and CMake package (its package
# root), not whatever is in staging: sdl2-compat installs the same files,
# with a newer version number, over sdl2's there, and a program built against
# them refuses to run on the sdl2 it is packaged with ("SDL downgrade").
CMAKE_ARGS_EXTRA+=( -DSDL2_DIR="$WS/pkgroot/$QNX_ARCH/sdl2$PREFIX/lib/cmake/SDL2" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
