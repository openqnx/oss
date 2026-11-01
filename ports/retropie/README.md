# retropie 0.3

> RetroPie retro gaming stack: RetroArch, EmulationStation, cores, configs

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`retroarch`](../retroarch/) [`emulationstation`](../emulationstation/) [`openttd`](../openttd/) [`libretro-2048`](../libretro-2048/) [`libretro-samples`](../libretro-samples/) [`libretro-mrboom`](../libretro-mrboom/) [`libretro-retro8`](../libretro-retro8/) [`libretro-fake08`](../libretro-fake08/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_retropie`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `retropie`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`retropie`](https://openqnx.github.io/oss/package/x86_64/retropie.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/retropie.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add retropie` (dependencies come along); one version: `apk add retropie=<version>` | `apk del retropie` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install retropie` (dependencies first) | `oss-get remove retropie` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-retropie && /usr/local/bin/qnx-port-tests/test_retropie    # its on-target test, from tests-8.0-<arch>-m-z
apk del retropie qnx-test-retropie
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/retropie.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/retropie.list
rm var/lib/manual/retropie.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

**Build the dependencies first** (48 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild retroarch emulationstation openttd libretro-2048 libretro-samples libretro-mrboom libretro-retro8 libretro-fake08
```

<details><summary>Full build chain (48)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`emulationstation`](../emulationstation/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freeimage`](../freeimage/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libass`](../libass/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libretro-2048`](../libretro-2048/) [`libretro-fake08`](../libretro-fake08/) [`libretro-mrboom`](../libretro-mrboom/) [`libretro-retro8`](../libretro-retro8/) [`libretro-samples`](../libretro-samples/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`nanosvg`](../nanosvg/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`openttd`](../openttd/) [`pcre2`](../pcre2/) [`pugixml`](../pugixml/) [`python3`](../python3/) [`rapidjson`](../rapidjson/) [`readline`](../readline/) [`retroarch`](../retroarch/) [`sdl2`](../sdl2/) [`sqlite`](../sqlite/) [`vlc`](../vlc/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa retropie
ls dists/8.0/*/ | grep '^retropie-'
```

This writes `retropie-<ver>-r0.tar.gz` (run time) and `retropie-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add retropie
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/retropie-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/retropie-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_retropie`](../../tests/src/) links retropie the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-retropie`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-retropie
/usr/local/bin/qnx-port-tests/test_retropie
```

```
== retropie ==
  ok   ...
PASS retropie
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_retropie` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_retropie`](../../tests/src/) -- a program linked against retropie the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - share/retropie installed
   - es_systems.cfg parses: <systemList>
   - 4 systems: retroarch, picoeight, openttd, testsuite
   - each has a name, full name, extension and a %ROM% command
   - 4 gamelists parse; every game has a path element and a name
   - every referenced game cover is installed
   - every PNG cover decodes (libpng)
   - 2048.png is 250x250
   - openttd.svg is an SVG document
   - es_settings.cfg: named settings parse
   - es_input.cfg: a keyboard input configuration
   - retroarch.cfg: 3000+ key = \"value\" settings, none malformed
   - video_driver vulkan, input_driver qnx_input, menu_driver ozone

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | retropie 0.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/retropie-0.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | retropie 0.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/retropie-0.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | retropie 0.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/retropie-0.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | retropie 0.3-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/retropie-0.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add retropie=<version>`), sources in `ported/retropie/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates retropie`; port one with `scripts/port-update retropie`.

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
