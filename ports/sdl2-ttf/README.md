# sdl2-ttf 2.24.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libsdl-org/SDL_ttf.git (tag `release-2.24.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`sdl2`](../sdl2/) [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) |
| Run-time dependencies | [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`sdl2`](../sdl2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install sdl2-ttf
oss-get install sdl2-ttf-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sdl2-ttf` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (21 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 freetype harfbuzz
```

<details><summary>Full build chain (21)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`sdl2`](../sdl2/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-2.24.0 --recurse-submodules --shallow-submodules \
    https://github.com/libsdl-org/SDL_ttf.git src/sdl2-ttf
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/sdl2-ttf -B build/$QNX_ARCH/sdl2-ttf -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DSDL2TTF_SAMPLES=OFF \
      -DSDL2TTF_VENDORED=OFF \
      -DSDL2TTF_HARFBUZZ=ON
cmake --build build/$QNX_ARCH/sdl2-ttf -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/sdl2-ttf
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sdl2-ttf`.

### 5. What it installs

11 files under `/usr/local`: 4 shared-library entries, 1 headers.

```
/usr/local/lib/libSDL2_ttf-2.0.so
/usr/local/lib/libSDL2_ttf-2.0.so.0
/usr/local/lib/libSDL2_ttf-2.0.so.0.2400.0
/usr/local/lib/libSDL2_ttf.so
```

The complete list is `stage/<arch>/.manifest/sdl2-ttf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH sdl2-ttf
ls repo/8.0/*/ | grep '^sdl2-ttf-'
```

This writes `sdl2-ttf-<ver>-r0.tar.gz` (run time) and `sdl2-ttf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install sdl2-ttf
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/sdl2-ttf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/sdl2-ttf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.24.0; GIT=https://github.com/libsdl-org/SDL_ttf.git; REF=release-2.24.0
SRCROOT="$SRC_DIR/sdl2-ttf"
DEPS="sdl2 freetype harfbuzz"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DSDL2TTF_SAMPLES=OFF -DSDL2TTF_VENDORED=OFF
                   -DSDL2TTF_HARFBUZZ=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
