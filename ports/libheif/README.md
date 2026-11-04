# libheif 1.20.2

> HEIF and AVIF image file format decoder and encoder

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/strukturag/libheif.git (tag `v1.20.2`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libde265`](../libde265/) [`x265`](../x265/) [`aom`](../aom/) [`dav1d`](../dav1d/) [`openh264`](../openh264/) [`libjpeg-turbo`](../libjpeg-turbo/) [`openjpeg`](../openjpeg/) [`openjph`](../openjph/) [`libwebp`](../libwebp/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`zlib`](../zlib/) |
| Run-time dependencies | [`aom`](../aom/) [`dav1d`](../dav1d/) [`libde265`](../libde265/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`openh264`](../openh264/) [`openjpeg`](../openjpeg/) [`openjph`](../openjph/) [`x265`](../x265/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libheif`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libheif` (programs, shared libraries, run-time data) and `libheif-dev` (headers, pkg-config/CMake files, static libraries; needs `libheif`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libheif`](https://openqnx.github.io/oss/package/x86_64/libheif.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libheif.html)) · [`libheif-dev`](https://openqnx.github.io/oss/package/x86_64/libheif-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libheif-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libheif` (dependencies come along); one version: `apk add libheif=<version>` | `apk del libheif` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libheif` (dependencies first) | `oss-get remove libheif` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libheif-dev` the same way, by its own name (`apk add libheif-dev`, `oss-get install libheif-dev`, by hand with its own file and list `libheif-dev.list`); remove it before `libheif`, which it needs.

```sh
# on the QNX target, as root
apk add libheif libheif-dev
apk add qnx-test-libheif && /usr/local/bin/qnx-port-tests/test_libheif    # its on-target test, from tests-8.0-<arch>
apk del libheif libheif-dev qnx-test-libheif
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libheif.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libheif.list
rm var/lib/manual/libheif.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libheif` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (20 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libde265 x265 aom dav1d openh264 libjpeg-turbo openjpeg openjph libwebp libpng libtiff zlib
```

<details><summary>Full build chain (20)</summary>

[`aom`](../aom/) [`dav1d`](../dav1d/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`lcms2`](../lcms2/) [`libde265`](../libde265/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openjpeg`](../openjpeg/) [`openjph`](../openjph/) [`x265`](../x265/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.20.2 --recurse-submodules --shallow-submodules \
    https://github.com/strukturag/libheif.git src/libheif
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/libheif -B build/$QNX_ARCH/libheif -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DWITH_FUZZERS=OFF \
      -DENABLE_PLUGIN_LOADING=OFF \
      -DWITH_LIBDE265=ON \
      -DWITH_X265=ON \
      -DWITH_AOM_DECODER=ON \
      -DWITH_AOM_ENCODER=ON \
      -DWITH_DAV1D=ON \
      -DWITH_DAV1D_PLUGIN=OFF \
      -DWITH_OpenH264_DECODER=ON \
      -DWITH_JPEG_DECODER=ON \
      -DWITH_JPEG_ENCODER=ON \
      -DWITH_OpenJPEG_DECODER=ON \
      -DWITH_OpenJPEG_ENCODER=ON \
      -DWITH_OpenJPEG_DECODER_PLUGIN=OFF \
      -DWITH_OpenJPEG_ENCODER_PLUGIN=OFF \
      -DWITH_OPENJPH_ENCODER=ON \
      -DWITH_OPENJPH_ENCODER_PLUGIN=OFF \
      -DWITH_SvtEnc=OFF \
      -DWITH_RAV1E=OFF \
      -DWITH_KVAZAAR=OFF \
      -DWITH_FFMPEG_DECODER=OFF \
      -DWITH_EXAMPLES=ON \
      -DWITH_EXAMPLE_HEIF_VIEW=OFF \
      -DWITH_GDK_PIXBUF=OFF \
      -DCMAKE_DISABLE_FIND_PACKAGE_SDL2=ON \
      -DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON
cmake --build build/$QNX_ARCH/libheif -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/libheif
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libheif: HEIF and AVIF image file format decoder and encoder (ISO/IEC 23008-12), with heif-dec, heif-enc, heif-info, heif-thumbnailer.
- Codecs, all built into libheif (no plugin directory to install and search): HEVC: libde265 decodes, x265 encodes; AV1: aom both ways, dav1d decodes; H.264 (openh264) decodes; JPEG (libjpeg-turbo) and JPEG 2000 (openjpeg) both ways; HT-J2K (openjph) encodes. libsharpyuv (from libwebp) for RGB->YUV. Left out: heif-view (an SDL viewer; on-screen use is not what this port is for), the gdk-pixbuf loader (it would need the loader cache updated on install), the experimental VVC codecs, kvazaar/svt/rav1e (not ported), tests.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libheif`.

### 5. What it installs

41 files under `/usr/local`: 3 shared-library entries, 5 programs, 24 headers.

```
/usr/local/lib/libheif.so
/usr/local/lib/libheif.so.1
/usr/local/lib/libheif.so.1.20.2
/usr/local/bin/heif-convert
/usr/local/bin/heif-dec
/usr/local/bin/heif-enc
/usr/local/bin/heif-info
/usr/local/bin/heif-thumbnailer
```

The complete list is `stage/<arch>/.manifest/libheif` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libheif
ls dists/8.0/*/ | grep '^libheif-'
```

This writes `libheif-<ver>-r0.tar.gz` (run time) and `libheif-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libheif
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libheif-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libheif-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libheif`](../../tests/src/) links libheif the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libheif`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libheif
/usr/local/bin/qnx-port-tests/test_libheif
```

```
== libheif ==
  ok   ...
PASS libheif
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libheif` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libheif`](../../tests/src/) -- a program linked against libheif the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libheif 1.x
   - HEVC decoder (libde265)
   - HEVC encoder (x265)
   - AV1 decoder (aom/dav1d)
   - AV1 encoder (aom)
   - JPEG 2000 decoder (openjpeg)
   - temp dir
   - HEIF (x265 -> libde265): 64x48, Exif kept, pixels within 3/255
   - AVIF (aom): 64x48, pixels within 3/255
   - HEIF with JPEG 2000 (openjpeg): pixels within 3/255

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> libheif`.

## Versions

This recipe builds **1.20.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libheif=<version>`), sources in `ported/libheif/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libheif`; port one with `scripts/port-update libheif`.

<details><summary>recipe.sh</summary>

```bash
# libheif: HEIF and AVIF image file format decoder and encoder (ISO/IEC
# 23008-12), with heif-dec, heif-enc, heif-info, heif-thumbnailer.
VERSION=1.20.2; GIT=https://github.com/strukturag/libheif.git; REF=v1.20.2
DEPS="libde265 x265 aom dav1d openh264 libjpeg-turbo openjpeg openjph libwebp libpng libtiff zlib"
BUILDSYS=cmake
# Codecs, all built into libheif (no plugin directory to install and search):
#   HEVC: libde265 decodes, x265 encodes; AV1: aom both ways, dav1d decodes;
#   H.264 (openh264) decodes; JPEG (libjpeg-turbo) and JPEG 2000 (openjpeg)
#   both ways; HT-J2K (openjph) encodes. libsharpyuv (from libwebp) for RGB->YUV.
# Left out: heif-view (an SDL viewer; on-screen use is not what this port is
# for), the gdk-pixbuf loader (it would need the loader cache updated on
# install), the experimental VVC codecs, kvazaar/svt/rav1e (not ported), tests.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DWITH_FUZZERS=OFF
                   -DENABLE_PLUGIN_LOADING=OFF
                   -DWITH_LIBDE265=ON -DWITH_X265=ON -DWITH_AOM_DECODER=ON -DWITH_AOM_ENCODER=ON
                   -DWITH_DAV1D=ON -DWITH_DAV1D_PLUGIN=OFF -DWITH_OpenH264_DECODER=ON
                   -DWITH_JPEG_DECODER=ON -DWITH_JPEG_ENCODER=ON
                   -DWITH_OpenJPEG_DECODER=ON -DWITH_OpenJPEG_ENCODER=ON
                   -DWITH_OpenJPEG_DECODER_PLUGIN=OFF -DWITH_OpenJPEG_ENCODER_PLUGIN=OFF
                   -DWITH_OPENJPH_ENCODER=ON -DWITH_OPENJPH_ENCODER_PLUGIN=OFF
                   -DWITH_SvtEnc=OFF -DWITH_RAV1E=OFF -DWITH_KVAZAAR=OFF -DWITH_FFMPEG_DECODER=OFF
                   -DWITH_EXAMPLES=ON -DWITH_EXAMPLE_HEIF_VIEW=OFF -DWITH_GDK_PIXBUF=OFF
                   -DCMAKE_DISABLE_FIND_PACKAGE_SDL2=ON -DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
