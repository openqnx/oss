# sdl2-ttf 2.24.0

> TrueType font rendering library for SDL2 using FreeType and HarfBuzz

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libsdl-org/SDL_ttf.git (tag `release-2.24.0`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`sdl2`](../sdl2/) [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) |
| Run-time dependencies | [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`sdl2`](../sdl2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sdl2_ttf`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `sdl2-ttf` (programs, shared libraries, run-time data) and `sdl2-ttf-dev` (headers, pkg-config/CMake files, static libraries; needs `sdl2-ttf`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`sdl2-ttf`](https://openqnx.github.io/oss/package/x86_64/sdl2-ttf.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sdl2-ttf.html)) · [`sdl2-ttf-dev`](https://openqnx.github.io/oss/package/x86_64/sdl2-ttf-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sdl2-ttf-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add sdl2-ttf` (dependencies come along); one version: `apk add sdl2-ttf=<version>` | `apk del sdl2-ttf` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install sdl2-ttf` (dependencies first) | `oss-get remove sdl2-ttf` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`sdl2-ttf-dev` the same way, by its own name (`apk add sdl2-ttf-dev`, `oss-get install sdl2-ttf-dev`, by hand with its own file and list `sdl2-ttf-dev.list`); remove it before `sdl2-ttf`, which it needs.

```sh
# on the QNX target, as root
apk add sdl2-ttf sdl2-ttf-dev
apk add qnx-test-sdl2-ttf && /usr/local/bin/qnx-port-tests/test_sdl2_ttf    # its on-target test
apk del sdl2-ttf sdl2-ttf-dev qnx-test-sdl2-ttf
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/sdl2-ttf.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/sdl2-ttf.list
rm var/lib/manual/sdl2-ttf.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sdl2-ttf
ls dists/8.0/*/ | grep '^sdl2-ttf-'
```

This writes `sdl2-ttf-<ver>-r0.tar.gz` (run time) and `sdl2-ttf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sdl2-ttf
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sdl2-ttf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sdl2-ttf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sdl2_ttf`](../../tests/src/) links sdl2-ttf the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sdl2-ttf`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sdl2-ttf
/usr/local/bin/qnx-port-tests/test_sdl2_ttf
```

```
== sdl2-ttf ==
  ok   ...
PASS sdl2-ttf
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sdl2_ttf` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sdl2_ttf`](../../tests/src/) -- a program linked against sdl2-ttf the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - TTF_Init
   - TTF_OpenFont DejaVuSansMono.ttf at 24 pt
   - family name DejaVu Sans Mono
   - style name Book
   - face is fixed width
   - plausible vertical metrics for 24 pt
   - glyph metrics for 'M'
   - monospace: equal advances
   - TTF_SizeText
   - width = 10 advances (+-2 px), height = font height
   - coverage: 'A' and Greek Omega yes, CJK no
   - Solid render has the measured size
   - Solid render has ink
   - Blended render
   - more ink for WWW than for iii
   - UTF-8 text (accents, Greek) renders: 5 advances wide
   - style set to bold

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sdl2-ttf 2.24.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/sdl2-ttf-2.24.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sdl2-ttf 2.24.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/sdl2-ttf-2.24.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sdl2-ttf 2.24.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/sdl2-ttf-2.24.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sdl2-ttf 2.24.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/sdl2-ttf-2.24.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.24.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sdl2-ttf=<version>`), sources in `ported/sdl2-ttf/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sdl2-ttf`; port one with `scripts/port-update sdl2-ttf`.

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
