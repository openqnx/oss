# retropie 0.3

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`retroarch`](../retroarch/) [`emulationstation`](../emulationstation/) [`openttd`](../openttd/) [`libretro-2048`](../libretro-2048/) [`libretro-samples`](../libretro-samples/) [`libretro-mrboom`](../libretro-mrboom/) [`libretro-retro8`](../libretro-retro8/) [`libretro-fake08`](../libretro-fake08/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install retropie
oss-get install retropie-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild retropie` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (44 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08
```

<details><summary>Full build chain (44)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`emulationstation`](../emulationstation/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freeimage`](../freeimage/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libass`](../libass/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libretro-2048`](../libretro-2048/) [`libretro-fake08`](../libretro-fake08/) [`libretro-mrboom`](../libretro-mrboom/) [`libretro-retro8`](../libretro-retro8/) [`libretro-samples`](../libretro-samples/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`nanosvg`](../nanosvg/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`openttd`](../openttd/) [`pcre2`](../pcre2/) [`pugixml`](../pugixml/) [`rapidjson`](../rapidjson/) [`retroarch`](../retroarch/) [`sdl2`](../sdl2/) [`vlc`](../vlc/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/retropie/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/retropie; B=build/$QNX_ARCH/retropie; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/share/retropie" && cp -a "$S/." "$STAGE$PREFIX/share/retropie/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- RetroPie for QNX: the retro gaming stack qnx-ports assembled -- RetroArch (frontend), EmulationStation (game browser), libretro cores and OpenTTD -- plus the configuration and start-up scripts from qnx-ports/build-files ports/RetroPie (files/). This component installs only those files; the programs are the components it depends on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH retropie`.

### 5. What it installs

19 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/retropie` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH retropie
ls repo/8.0/*/ | grep '^retropie-'
```

This writes `retropie-<ver>-r0.tar.gz` (run time) and `retropie-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install retropie
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/retropie-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/retropie-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# RetroPie for QNX: the retro gaming stack qnx-ports assembled -- RetroArch
# (frontend), EmulationStation (game browser), libretro cores and OpenTTD --
# plus the configuration and start-up scripts from qnx-ports/build-files
# ports/RetroPie (files/). This component installs only those files; the
# programs are the components it depends on.
VERSION=0.3
DEPS="retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/retropie/files/." "$SRCROOT/"; }
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/share/retropie" && cp -a "$S/." "$STAGE$PREFIX/share/retropie/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
