# qt6 6.8.1

> Qt 6 cross-platform application and UI framework

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.qt.io/archive/qt/6.8/6.8.1/single/qt-everywhere-src-6.8.1.tar.xz |
| SHA-256 | `45e3a9f6d33c92ffe65a1fde1a8eba5b228112df675f7f9026eaa332b2e2edff` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`freetype`](../freetype/) [`harfbuzz`](../harfbuzz/) [`pcre2`](../pcre2/) [`openssl`](../openssl/) [`icu`](../icu/) [`double-conversion`](../double-conversion/) [`ffmpeg`](../ffmpeg/) [`fontconfig`](../fontconfig/) [`gstreamer`](../gstreamer/) [`gst-plugins-base`](../gst-plugins-base/) [`gst-plugins-bad`](../gst-plugins-bad/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`zstd`](../zstd/) |
| Run-time dependencies | [`brotli`](../brotli/) [`double-conversion`](../double-conversion/) [`ffmpeg`](../ffmpeg/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`glib`](../glib/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libwebp`](../libwebp/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qt6`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add qt6
apk add qt6-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-qt6 && /usr/local/bin/qnx-port-tests/test_qt6    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install qt6`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qt6` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (60 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd
```

<details><summary>Full build chain (60)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`double-conversion`](../double-conversion/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`ffmpeg`](../ffmpeg/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lame`](../lame/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`svt-av1`](../svt-av1/) [`x264`](../x264/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.qt.io/archive/qt/6.8/6.8.1/single/qt-everywhere-src-6.8.1.tar.xz
mkdir -p dist src/qt6
curl -fL -o dist/$(basename $URL) $URL
echo "45e3a9f6d33c92ffe65a1fde1a8eba5b228112df675f7f9026eaa332b2e2edff  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/qt6 --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/qt6 < ports/qt6/patches/0001-qtmultimedia-alsa-backend-on-qnx-8.patch
```

**[`0001-qtmultimedia-alsa-backend-on-qnx-8.patch`](patches/0001-qtmultimedia-alsa-backend-on-qnx-8.patch)** — qtmultimedia: use the ALSA backend on QNX SDP 8

> Qt's QNX audio backend (src/multimedia/qnx) is written against io-snd's
> <sys/asoundlib.h> and its snd_pcm_plugin_* API, which SDP 8 no longer ships
> ("sys/asoundlib.h: No such file or directory"). SDP 8's libasound offers the
> ALSA API under <alsa/asoundlib.h> instead, which is what Qt's ALSA backend
> uses. Let the alsa feature be enabled on QNX, and build the QNX backend only
> when it is not; qplatformmediadevices.cpp already prefers ALSA to Q_OS_QNX.
>
> ---

Touches: `qtmultimedia/src/multimedia/configure.cmake`, `qtmultimedia/src/multimedia/CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qt6; B=build/$QNX_ARCH/qt6; mkdir -p $B
```

```bash
do_configure() {
  if [ ! -x "$QT_HOST/bin/qmake" ]; then
    local hb="$BUILD_DIR/host/qt6-$VERSION"; mkdir -p "$hb"
    ( cd "$hb" && env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR "$S/configure" \
        -prefix "$QT_HOST" -submodules "$QT_SUBMODULES" -release -nomake examples -nomake tests \
        -no-opengl -no-feature-vulkan -- -G Ninja && cmake --build . --parallel && cmake --install . ) || return 1
  fi
  # Qt derives QNX itself, from qt_set01(QNX CMAKE_SYSTEM_NAME STREQUAL "QNX"),
  # whose unquoted ${ARGN} reads the literal QNX as a variable: with the
  # shared toolchain's set(QNX TRUE) in force it compares against "TRUE" and
  # turns QNX off, so neither the qnx platform plugin nor qtmultimedia's QNX
  # audio backend is built ("qqnxmediadevices_p.h: No such file or
  # directory"). Wrap the toolchain and drop QNX for Qt to set.
  printf 'include("%s")\nunset(QNX)\n' "$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" > "$B/qt-toolchain.cmake"
  ( cd "$B" && "$S/configure" -prefix "$PREFIX" -qt-host-path "$QT_HOST" \
      -submodules "$QT_SUBMODULES" -release -nomake examples -nomake tests \
      -opengl es2 -qpa qnx -system-zlib -system-libpng -system-libjpeg -system-freetype -system-harfbuzz \
      -system-pcre -openssl-linked -icu -no-feature-qdoc -no-feature-clang -no-feature-backtrace -no-eglfs -no-kms -feature-alsa -no-sbom \
      -- -G Ninja -DCMAKE_TOOLCHAIN_FILE="$B/qt-toolchain.cmake" -DQT_QMAKE_TARGET_MKSPEC="$(_qt_mkspec)" )
}
```

```bash
do_build() { cmake --build "$B" --parallel; }
```

```bash
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Qt 6.8.1, the release qnx-ports builds. Qt supports QNX as a target itself (QPA plugin "qnx" on Screen + EGL/GLES2 from the SDP); the one source patch moves Qt Multimedia's audio onto SDP 8's ALSA API (patches/). Modules: the commonly used set rather than all 40 (QtWebEngine, for one, does not support QNX); add more to QT_SUBMODULES. A Qt cross build needs a host Qt of the same version for its tools (moc, rcc, uic, qsb, ...); it is built from the same source into hosttools/qt6-$VERSION, once for both architectures.
- Qt's QNX mkspecs: qnx-aarch64le-qcc and qnx-x86-64-qcc (dashes, not the x86_64 of the architecture name: "Unknown platform qnx-x86_64-qcc").
- EGLFS and KMS are off: they are Linux's DRM/KMS display integration; on QNX Qt draws through its qnx platform plugin (Screen). backtrace is off: QNX has no glibc backtrace() ("'::backtrace' has not been declared" in qlogging.cpp). qdoc (and the libclang it needs) is off for the target as it is for the host build: the target configure otherwise looks for the host's qdoc ("Failed to find the host tool Qt6::qdoc").
- STAGE is the component's pkgroot while installing, so install there. SBOM generation is off (-no-sbom): at install time it checksums files at the configured prefix, ignoring --prefix ("Cannot find 'lib/libQt6Core.so.6.8.1' to compute its checksum"), and with DESTDIR it then could not find the other modules' SBOM documents ("Could not find external SBOM document").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qt6`.

### 5. What it installs

10860 files under `/usr/local`: 264 shared-library entries, 16 programs, 5194 headers.

```
/usr/local/lib/libQt6Charts.so
/usr/local/lib/libQt6Charts.so.6
/usr/local/lib/libQt6Charts.so.6.8.1
/usr/local/lib/libQt6ChartsQml.so
/usr/local/lib/libQt6ChartsQml.so.6
/usr/local/lib/libQt6ChartsQml.so.6.8.1
/usr/local/lib/libQt6Concurrent.so
/usr/local/lib/libQt6Concurrent.so.6
/usr/local/lib/libQt6Concurrent.so.6.8.1
/usr/local/lib/libQt6Core.so
/usr/local/lib/libQt6Core.so.6
/usr/local/lib/libQt6Core.so.6.8.1
/usr/local/lib/libQt6Core5Compat.so
/usr/local/lib/libQt6Core5Compat.so.6
/usr/local/lib/libQt6Core5Compat.so.6.8.1
/usr/local/lib/libQt6DBus.so
...
```

The complete list is `stage/<arch>/.manifest/qt6` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qt6
ls dists/8.0/*/ | grep '^qt6-'
```

This writes `qt6-<ver>-r0.tar.gz` (run time) and `qt6-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qt6
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qt6-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qt6-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qt6`](../../tests/src/) links qt6 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qt6`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qt6
/usr/local/bin/qnx-port-tests/test_qt6
```

```
== qt6 ==
  ok   ...
PASS qt6
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qt6` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qt6`](../../tests/src/) -- a program linked against qt6 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - event loop ran a timer
   - JSON round trip
   - regular expression (PCRE2)
   - Unicode case mapping
   - raster painting into QImage

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64 virt, vanilla mkqnximage image | apk:local-dists | qt6 6.8.1-r0 | ✅ PASS | 2026-10-30 | [log](../../tests/logs/aarch64le/qt6-6.8.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qt6 6.8.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/qt6-6.8.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qt6 6.8.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/qt6-6.8.1-r1.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qt6 6.8.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/qt6-6.8.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qt6 6.8.1-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/qt6-6.8.1-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.8.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qt6=<version>`), sources in `ported/qt6/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qt6`; port one with `scripts/port-update qt6`.

<details><summary>recipe.sh</summary>

```bash
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/single/qt-everywhere-src-6.8.1.tar.xz
SHA256=45e3a9f6d33c92ffe65a1fde1a8eba5b228112df675f7f9026eaa332b2e2edff
DEPS="zlib libpng libjpeg-turbo freetype harfbuzz pcre2 openssl icu double-conversion ffmpeg fontconfig gstreamer gst-plugins-base gst-plugins-bad libtiff libwebp zstd"
BUILDSYS=custom
# Qt 6.8.1, the release qnx-ports builds. Qt supports QNX as a target itself
# (QPA plugin "qnx" on Screen + EGL/GLES2 from the SDP); the one source patch
# moves Qt Multimedia's audio onto SDP 8's ALSA API (patches/).
# Modules: the commonly used set rather than all 40 (QtWebEngine, for one, does
# not support QNX); add more to QT_SUBMODULES.
#
# A Qt cross build needs a host Qt of the same version for its tools (moc, rcc,
# uic, qsb, ...); it is built from the same source into hosttools/qt6-$VERSION,
# once for both architectures.
QT_SUBMODULES="qtbase,qtshadertools,qtdeclarative,qtsvg,qtimageformats,qt5compat,qtserialport,qtserialbus,qtwebsockets,qtcharts,qtvirtualkeyboard,qttools,qtmultimedia"
QT_HOST="$WS/hosttools/qt6-$VERSION"
# Qt's QNX mkspecs: qnx-aarch64le-qcc and qnx-x86-64-qcc (dashes, not the
# x86_64 of the architecture name: "Unknown platform qnx-x86_64-qcc").
_qt_mkspec() { case "$QNX_ARCH" in aarch64le) echo qnx-aarch64le-qcc ;; x86_64) echo qnx-x86-64-qcc ;; esac; }
# EGLFS and KMS are off: they are Linux's DRM/KMS display integration; on QNX Qt
# draws through its qnx platform plugin (Screen).
# backtrace is off: QNX has no glibc backtrace() ("'::backtrace' has not been
# declared" in qlogging.cpp).
# qdoc (and the libclang it needs) is off for the target as it is for the
# host build: the target configure otherwise looks for the host's qdoc
# ("Failed to find the host tool Qt6::qdoc").
do_configure() {
  if [ ! -x "$QT_HOST/bin/qmake" ]; then
    local hb="$BUILD_DIR/host/qt6-$VERSION"; mkdir -p "$hb"
    ( cd "$hb" && env -u CC -u CXX -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR "$S/configure" \
        -prefix "$QT_HOST" -submodules "$QT_SUBMODULES" -release -nomake examples -nomake tests \
        -no-opengl -no-feature-vulkan -- -G Ninja && cmake --build . --parallel && cmake --install . ) || return 1
  fi
  # Qt derives QNX itself, from qt_set01(QNX CMAKE_SYSTEM_NAME STREQUAL "QNX"),
  # whose unquoted ${ARGN} reads the literal QNX as a variable: with the
  # shared toolchain's set(QNX TRUE) in force it compares against "TRUE" and
  # turns QNX off, so neither the qnx platform plugin nor qtmultimedia's QNX
  # audio backend is built ("qqnxmediadevices_p.h: No such file or
  # directory"). Wrap the toolchain and drop QNX for Qt to set.
  printf 'include("%s")\nunset(QNX)\n' "$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" > "$B/qt-toolchain.cmake"
  ( cd "$B" && "$S/configure" -prefix "$PREFIX" -qt-host-path "$QT_HOST" \
      -submodules "$QT_SUBMODULES" -release -nomake examples -nomake tests \
      -opengl es2 -qpa qnx -system-zlib -system-libpng -system-libjpeg -system-freetype -system-harfbuzz \
      -system-pcre -openssl-linked -icu -no-feature-qdoc -no-feature-clang -no-feature-backtrace -no-eglfs -no-kms -feature-alsa -no-sbom \
      -- -G Ninja -DCMAKE_TOOLCHAIN_FILE="$B/qt-toolchain.cmake" -DQT_QMAKE_TARGET_MKSPEC="$(_qt_mkspec)" )
}
do_build() { cmake --build "$B" --parallel; }
# STAGE is the component's pkgroot while installing, so install there.
# SBOM generation is off (-no-sbom): at install time it checksums files at the
# configured prefix, ignoring --prefix ("Cannot find 'lib/libQt6Core.so.6.8.1'
# to compute its checksum"), and with DESTDIR it then could not find the
# other modules' SBOM documents ("Could not find external SBOM document").
do_install() { DESTDIR="$STAGE" cmake --install "$B"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
