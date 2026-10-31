# sdl2-image 2.8.8

> Image loading library for SDL2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libsdl-org/SDL_image.git (tag `release-2.8.8`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`sdl2`](../sdl2/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`libavif`](../libavif/) [`libjxl`](../libjxl/) |
| Run-time dependencies | [`libavif`](../libavif/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`sdl2`](../sdl2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sdl2_image`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add sdl2-image
apk add sdl2-image-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-sdl2-image && /usr/local/bin/qnx-port-tests/test_sdl2_image    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install sdl2-image`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sdl2-image` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (26 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl
```

<details><summary>Full build chain (26)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`dav1d`](../dav1d/) [`expat`](../expat/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`highway`](../highway/) [`lcms2`](../lcms2/) [`libavif`](../libavif/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libyuv`](../libyuv/) [`ncurses`](../ncurses/) [`sdl2`](../sdl2/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-2.8.8 --recurse-submodules --shallow-submodules \
    https://github.com/libsdl-org/SDL_image.git src/sdl2-image
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/sdl2-image -B build/$QNX_ARCH/sdl2-image -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DSDL2IMAGE_SAMPLES=OFF \
      -DSDL2IMAGE_TESTS=OFF \
      -DSDL2IMAGE_DEPS_SHARED=OFF \
      -DSDL2IMAGE_VENDORED=OFF \
      -DSDL2IMAGE_BACKEND_IMAGEIO=OFF
cmake --build build/$QNX_ARCH/sdl2-image -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/sdl2-image
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sdl2-image`.

### 5. What it installs

13 files under `/usr/local`: 4 shared-library entries, 1 headers.

```
/usr/local/lib/libSDL2_image-2.0.so
/usr/local/lib/libSDL2_image-2.0.so.0
/usr/local/lib/libSDL2_image-2.0.so.0.800.8
/usr/local/lib/libSDL2_image.so
```

The complete list is `stage/<arch>/.manifest/sdl2-image` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sdl2-image
ls dists/8.0/*/ | grep '^sdl2-image-'
```

This writes `sdl2-image-<ver>-r0.tar.gz` (run time) and `sdl2-image-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sdl2-image
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sdl2-image-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sdl2-image-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sdl2_image`](../../tests/src/) links sdl2-image the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sdl2-image`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sdl2-image
/usr/local/bin/qnx-port-tests/test_sdl2_image
```

```
== sdl2-image ==
  ok   ...
PASS sdl2-image
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sdl2_image` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sdl2_image`](../../tests/src/) -- a program linked against sdl2-image the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - PNG and JPEG codecs available
   - format sniffing: PNG
   - IMG_Load_RW decodes the 1x1 PNG
   - decoded pixel is pure red
   - create 64x48 RGBA surface
   - IMG_SavePNG to /tmp
   - IMG_Load reads it back (64x48)
   - PNG round trip is lossless (RGBA)
   - IMG_SaveJPG quality 95
   - format sniffing: JPEG
   - JPEG round trip within tolerance
   - TGA decoded (2x1)
   - TGA pixels: blue, green
   - missing file -> NULL with an error

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sdl2-image 2.8.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/sdl2-image-2.8.8-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sdl2-image 2.8.8-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/sdl2-image-2.8.8-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.8.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sdl2-image=<version>`), sources in `ported/sdl2-image/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sdl2-image`; port one with `scripts/port-update sdl2-image`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.8.8; GIT=https://github.com/libsdl-org/SDL_image.git; REF=release-2.8.8
SRCROOT="$SRC_DIR/sdl2-image"
DEPS="sdl2 libpng libjpeg-turbo libtiff libwebp libavif libjxl"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DSDL2IMAGE_SAMPLES=OFF -DSDL2IMAGE_TESTS=OFF
                   -DSDL2IMAGE_DEPS_SHARED=OFF -DSDL2IMAGE_VENDORED=OFF
                   -DSDL2IMAGE_BACKEND_IMAGEIO=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
