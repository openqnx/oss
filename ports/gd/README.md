# gd 2.3.3

> Library for the dynamic creation of images (PNG, JPEG, GIF, WebP, TIFF, AVIF, HEIF)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libgd/libgd.git (tag `gd-2.3.3`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`freetype`](../freetype/) [`fontconfig`](../fontconfig/) [`libavif`](../libavif/) [`libheif`](../libheif/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`libavif`](../libavif/) [`libheif`](../libheif/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gd`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gd` (programs, shared libraries, run-time data) and `gd-dev` (headers, pkg-config/CMake files, static libraries; needs `gd`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gd`](https://openqnx.github.io/oss/package/x86_64/gd.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gd.html)) · [`gd-dev`](https://openqnx.github.io/oss/package/x86_64/gd-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gd-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gd` (dependencies come along); one version: `apk add gd=<version>` | `apk del gd` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gd` (dependencies first) | `oss-get remove gd` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gd-dev` the same way, by its own name (`apk add gd-dev`, `oss-get install gd-dev`, by hand with its own file and list `gd-dev.list`); remove it before `gd`, which it needs.

```sh
# on the QNX target, as root
apk add gd gd-dev
apk add qnx-test-gd && /usr/local/bin/qnx-port-tests/test_gd    # its on-target test, from tests-8.0-<arch>
apk del gd gd-dev qnx-test-gd
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gd.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gd.list
rm var/lib/manual/gd.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gd` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (28 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib libpng libjpeg-turbo libtiff libwebp freetype fontconfig libavif libheif
```

<details><summary>Full build chain (28)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`dav1d`](../dav1d/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libde265`](../libde265/) [`libheif`](../libheif/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libyuv`](../libyuv/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openjpeg`](../openjpeg/) [`openjph`](../openjph/) [`x265`](../x265/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b gd-2.3.3 --recurse-submodules --shallow-submodules \
    https://github.com/libgd/libgd.git src/gd
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gd < ports/gd/patches/0001-libavif-1-cmake.patch
```

**[`0001-libavif-1-cmake.patch`](patches/0001-libavif-1-cmake.patch)** — Accept libavif 1.x in CMake (find_package without a 0.x version)

> libavif's CMake package config declares SameMajorVersion compatibility, so
> FIND_PACKAGE(libavif 0.8.2) rejects libavif 1.x ("version 1.1.1 ... not
> accepted") although gd works with it. Ask for any version; 0.8.2 is the
> minimum the comment keeps.

Touches: `CMakeLists.txt` · Upstream status: Pending (not QNX-specific: gd 2.3.3 predates libavif 1.0)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/gd -B build/$QNX_ARCH/gd -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_STATIC_LIBS=OFF \
      -DBUILD_TEST=OFF \
      -DENABLE_PNG=ON \
      -DENABLE_JPEG=ON \
      -DENABLE_TIFF=ON \
      -DENABLE_WEBP=ON \
      -DENABLE_FREETYPE=ON \
      -DENABLE_FONTCONFIG=ON \
      -DENABLE_AVIF=ON \
      -DENABLE_HEIF=ON \
      -DENABLE_XPM=OFF \
      -DENABLE_LIQ=OFF \
      -DENABLE_RAQM=OFF \
      -DENABLE_ICONV=OFF
cmake --build build/$QNX_ARCH/gd -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/gd
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- gd (libgd): a library for drawing images -- lines, polygons, text with FreeType fonts, fills, copies and scaling -- read and written as PNG, JPEG, GIF, WebP, TIFF, BMP, AVIF and HEIF. graphviz's gd renderer is built on it.
- Every format whose library is here; text with FreeType and fontconfig font names. Not: XPM (needs libXpm, X11), libimagequant and raqm (not ported).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gd`.

### 5. What it installs

47 files under `/usr/local`: 3 shared-library entries, 12 programs, 12 headers.

```
/usr/local/lib/libgd.so
/usr/local/lib/libgd.so.3
/usr/local/lib/libgd.so.3.0.11
/usr/local/bin/annotate
/usr/local/bin/bdftogd
/usr/local/bin/gd2copypal
/usr/local/bin/gd2togif
/usr/local/bin/gd2topng
/usr/local/bin/gdcmpgif
/usr/local/bin/gdparttopng
/usr/local/bin/gdtopng
/usr/local/bin/giftogd2
/usr/local/bin/pngtogd
/usr/local/bin/pngtogd2
/usr/local/bin/webpng
```

The complete list is `stage/<arch>/.manifest/gd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gd
ls dists/8.0/*/ | grep '^gd-'
```

This writes `gd-<ver>-r0.tar.gz` (run time) and `gd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gd`](../../tests/src/) links gd the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gd`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gd
/usr/local/bin/qnx-port-tests/test_gd
```

```
== gd ==
  ok   ...
PASS gd
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gd` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gd`](../../tests/src/) -- a program linked against gd the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - 100x80 true-colour image
   - filled rectangle: red inside, white outside
   - line along y=50
   - ellipse outline green, flood-filled blue inside
   - scaled to 50x40, the red square still red
   - rotated 90 degrees: 80x100
   - built with fontconfig
   - FreeType text 'Hello' at 14 pt: 30-70 px wide, drawn

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> gd`.

## Versions

This recipe builds **2.3.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gd=<version>`), sources in `ported/gd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gd`; port one with `scripts/port-update gd`.

<details><summary>recipe.sh</summary>

```bash
# gd (libgd): a library for drawing images -- lines, polygons, text with
# FreeType fonts, fills, copies and scaling -- read and written as PNG, JPEG,
# GIF, WebP, TIFF, BMP, AVIF and HEIF. graphviz's gd renderer is built on it.
VERSION=2.3.3; GIT=https://github.com/libgd/libgd.git; REF=gd-2.3.3
DEPS="zlib libpng libjpeg-turbo libtiff libwebp freetype fontconfig libavif libheif"
BUILDSYS=cmake
# Every format whose library is here; text with FreeType and fontconfig font
# names. Not: XPM (needs libXpm, X11), libimagequant and raqm (not ported).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DBUILD_TEST=OFF
                   -DENABLE_PNG=ON -DENABLE_JPEG=ON -DENABLE_TIFF=ON -DENABLE_WEBP=ON
                   -DENABLE_FREETYPE=ON -DENABLE_FONTCONFIG=ON -DENABLE_AVIF=ON -DENABLE_HEIF=ON
                   -DENABLE_XPM=OFF -DENABLE_LIQ=OFF -DENABLE_RAQM=OFF -DENABLE_ICONV=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
