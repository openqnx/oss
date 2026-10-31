# graphicsmagick 1.3.46

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/graphicsmagick/graphicsmagick/1.3.46/GraphicsMagick-1.3.46.tar.xz |
| SHA-256 | `c7c706a505e9c6c3764156bb94a0c9644d79131785df15a89c9f8721d1abd061` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`libjxl`](../libjxl/) [`freetype`](../freetype/) [`libxml2`](../libxml2/) [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`zstd`](../zstd/) [`lcms2`](../lcms2/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`freetype`](../freetype/) [`lcms2`](../lcms2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`libxml2`](../libxml2/) [`libzip`](../libzip/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_graphicsmagick`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add graphicsmagick
apk add graphicsmagick-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-graphicsmagick && /usr/local/bin/qnx-port-tests/test_graphicsmagick    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install graphicsmagick`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild graphicsmagick` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libpng libjpeg-turbo libtiff libwebp libjxl freetype libxml2 zlib bzip2 xz zstd lcms2
```

<details><summary>Full build chain (19)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`highway`](../highway/) [`lcms2`](../lcms2/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://downloads.sourceforge.net/project/graphicsmagick/graphicsmagick/1.3.46/GraphicsMagick-1.3.46.tar.xz
mkdir -p dist src/graphicsmagick
curl -fL -o dist/$(basename $URL) $URL
echo "c7c706a505e9c6c3764156bb94a0c9644d79131785df15a89c9f8721d1abd061  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/graphicsmagick --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/graphicsmagick && cd build/$QNX_ARCH/graphicsmagick
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ac_cv_func_spawnvp=no \
$OLDPWD/src/graphicsmagick/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-modules=no \
    --without-x \
    --without-perl \
    --without-trio \
    --without-wmf \
    --without-gs \
    --disable-openmp \
    --with-quantum-depth=16
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- graphicsmagick: image processing (the gm command and the Magick++ / C libraries; PNG, JPEG, TIFF, WebP, JPEG XL, FreeType text).
- QNX has a spawnvp() (process.h), which configure finds; GraphicsMagick's spawnvp branch is the Windows one (_P_WAIT). The POSIX fork/exec one is right for QNX.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH graphicsmagick`.

### 5. What it installs

308 files under `/usr/local`: 9 shared-library entries, 4 programs, 83 headers.

```
/usr/local/lib/libGraphicsMagick++.so
/usr/local/lib/libGraphicsMagick++.so.12
/usr/local/lib/libGraphicsMagick++.so.12.8.2
/usr/local/lib/libGraphicsMagick.so
/usr/local/lib/libGraphicsMagick.so.3
/usr/local/lib/libGraphicsMagick.so.3.26.0
/usr/local/lib/libGraphicsMagickWand.so
/usr/local/lib/libGraphicsMagickWand.so.2
/usr/local/lib/libGraphicsMagickWand.so.2.10.2
/usr/local/bin/GraphicsMagick++-config
/usr/local/bin/GraphicsMagick-config
/usr/local/bin/GraphicsMagickWand-config
/usr/local/bin/gm
```

The complete list is `stage/<arch>/.manifest/graphicsmagick` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa graphicsmagick
ls dists/8.0/*/ | grep '^graphicsmagick-'
```

This writes `graphicsmagick-<ver>-r0.tar.gz` (run time) and `graphicsmagick-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add graphicsmagick
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/graphicsmagick-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/graphicsmagick-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_graphicsmagick`](../../tests/src/) links graphicsmagick the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-graphicsmagick`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-graphicsmagick
/usr/local/bin/qnx-port-tests/test_graphicsmagick
```

```
== graphicsmagick ==
  ok   ...
PASS graphicsmagick
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_graphicsmagick` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_graphicsmagick`](../../tests/src/) -- a program linked against graphicsmagick the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | graphicsmagick 1.3.46-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/graphicsmagick-1.3.46-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | graphicsmagick 1.3.46-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/graphicsmagick-1.3.46-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.46**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add graphicsmagick=<version>`), sources in `ported/graphicsmagick/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates graphicsmagick`; port one with `scripts/port-update graphicsmagick`.

<details><summary>recipe.sh</summary>

```bash
# graphicsmagick: image processing (the gm command and the Magick++ / C libraries;
# PNG, JPEG, TIFF, WebP, JPEG XL, FreeType text).
VERSION=1.3.46
TARBALL=https://downloads.sourceforge.net/project/graphicsmagick/graphicsmagick/1.3.46/GraphicsMagick-1.3.46.tar.xz
SHA256=c7c706a505e9c6c3764156bb94a0c9644d79131785df15a89c9f8721d1abd061
DEPS="libpng libjpeg-turbo libtiff libwebp libjxl freetype libxml2 zlib bzip2 xz zstd lcms2"
BUILDSYS=autotools
# QNX has a spawnvp() (process.h), which configure finds; GraphicsMagick's
# spawnvp branch is the Windows one (_P_WAIT). The POSIX fork/exec one is
# right for QNX.
CONF_ENV=( ac_cv_func_spawnvp=no )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-modules=no --without-x --without-perl --without-trio --without-wmf --without-gs --disable-openmp --with-quantum-depth=16 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
