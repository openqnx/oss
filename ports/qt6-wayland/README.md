# qt6-wayland 6.8.1

> Qt 6 Wayland: the wayland platform plugin, so Qt programs run on Wayland compositors (labwc)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwayland-everywhere-src-6.8.1.tar.xz |
| SHA-256 | `2226fbde4e2ddd12f8bf4b239c8f38fd706a54e789e63467dfddc77129eca203` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`qt6`](../qt6/) [`wayland`](../wayland/) |
| Run-time dependencies | [`libxkbcommon`](../libxkbcommon/) [`qt6`](../qt6/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qt6_wayland`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `qt6-wayland` (programs, shared libraries, run-time data) and `qt6-wayland-dev` (headers, pkg-config/CMake files, static libraries; needs `qt6-wayland`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`qt6-wayland`](https://openqnx.github.io/oss/package/x86_64/qt6-wayland.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qt6-wayland.html)) · [`qt6-wayland-dev`](https://openqnx.github.io/oss/package/x86_64/qt6-wayland-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qt6-wayland-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add qt6-wayland` (dependencies come along); one version: `apk add qt6-wayland=<version>` | `apk del qt6-wayland` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install qt6-wayland` (dependencies first) | `oss-get remove qt6-wayland` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`qt6-wayland-dev` the same way, by its own name (`apk add qt6-wayland-dev`, `oss-get install qt6-wayland-dev`, by hand with its own file and list `qt6-wayland-dev.list`); remove it before `qt6-wayland`, which it needs.

```sh
# on the QNX target, as root
apk add qt6-wayland qt6-wayland-dev
apk add qnx-test-qt6-wayland && /usr/local/bin/qnx-port-tests/test_qt6_wayland    # its on-target test, from tests-8.0-<arch>-m-z
apk del qt6-wayland qt6-wayland-dev qnx-test-qt6-wayland
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/qt6-wayland.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/qt6-wayland.list
rm var/lib/manual/qt6-wayland.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qt6-wayland` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (67 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild qt6 wayland
```

<details><summary>Full build chain (67)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`double-conversion`](../double-conversion/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`ffmpeg`](../ffmpeg/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lame`](../lame/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`qt6`](../qt6/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`svt-av1`](../svt-av1/) [`wayland`](../wayland/) [`x264`](../x264/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwayland-everywhere-src-6.8.1.tar.xz
mkdir -p dist src/qt6-wayland
curl -fL -o dist/$(basename $URL) $URL
echo "2226fbde4e2ddd12f8bf4b239c8f38fd706a54e789e63467dfddc77129eca203  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/qt6-wayland --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/qt6-wayland < ports/qt6-wayland/patches/0001-qnx-egl-platform-wayland-test.patch
```

**[`0001-qnx-egl-platform-wayland-test.patch`](patches/0001-qnx-egl-platform-wayland-test.patch)** — Decide Wayland EGL on QNX with qtwayland's own EGL test, not a qtbase feature 6.8 lacks

> On QNX, the wayland-egl client integration and the drm-egl server buffer
> are made conditional on the qtbase feature egl_extension_platform_wayland,
> which Qt 6.8's qtbase does not define (it arrived in a later qtbase):
> configure stops with "Attempting to evaluate feature
> egl_extension_platform_wayland but its definition is missing". What the
> condition asks -- does EGL know the Wayland platform
> (eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_EXT, ...)) -- is exactly
> qtwayland's own compile test egl_1_5_wayland: use that on QNX. SDP 8's
> EGL headers define EGL_PLATFORM_WAYLAND_EXT, and QNX's EGL on a
> graphics target accepts Wayland displays (GTK 4 and wlroots use it).

Touches: `src/configure.cmake` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qt6-wayland; B=build/$QNX_ARCH/qt6-wayland; mkdir -p $B
```

```bash
do_configure() {
  qt6_host_module libexec/qtwaylandscanner -DFEATURE_wayland_client=OFF -DFEATURE_wayland_server=OFF &&
  qt6_module_configure -DFEATURE_wayland_server=OFF -DFEATURE_wayland_client=ON
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Qt 6 qtwayland, the Qt version ports/qt6 builds, from Qt's own submodule tarball, cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
- Keyboard input without xkbcommon: Qt reads xkbcommon only through qtbase's xkbcommon feature, which ports/qt6 does not enable; the protocol XML Qt needs is in its own source (src/3rdparty/protocol).
- The client side only: the wayland platform plugin (QT_QPA_PLATFORM=wayland, what labwc runs Qt programs with), its shell integrations (xdg-shell, ...) and the EGL client buffer integration; not the compositor library. qtwaylandscanner generates C++ from the protocol XML at build time: the host build makes only that tool (no client, no compositor) for the host Qt. The protocol XML is turned into C by wayland-scanner from hosttools/.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qt6-wayland`.

### 5. What it installs

282 files under `/usr/local`: 9 shared-library entries, 128 headers.

```
/usr/local/lib/libQt6WaylandClient.so
/usr/local/lib/libQt6WaylandClient.so.6
/usr/local/lib/libQt6WaylandClient.so.6.8.1
/usr/local/lib/libQt6WaylandEglClientHwIntegration.so
/usr/local/lib/libQt6WaylandEglClientHwIntegration.so.6
/usr/local/lib/libQt6WaylandEglClientHwIntegration.so.6.8.1
/usr/local/lib/libQt6WlShellIntegration.so
/usr/local/lib/libQt6WlShellIntegration.so.6
/usr/local/lib/libQt6WlShellIntegration.so.6.8.1
```

The complete list is `stage/<arch>/.manifest/qt6-wayland` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qt6-wayland
ls dists/8.0/*/ | grep '^qt6-wayland-'
```

This writes `qt6-wayland-<ver>-r0.tar.gz` (run time) and `qt6-wayland-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qt6-wayland
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qt6-wayland-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qt6-wayland-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qt6_wayland`](../../tests/src/) links qt6-wayland the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qt6-wayland`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qt6-wayland
/usr/local/bin/qnx-port-tests/test_qt6_wayland
```

```
== qt6-wayland ==
  ok   ...
PASS qt6-wayland
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qt6_wayland` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qt6_wayland`](../../tests/src/) -- a program linked against qt6-wayland the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - platforms/libqwayland-generic.so loads (Qt6WaylandClient, libwayland-client, -cursor)
   - QPA plugin, key wayland
   - platforms/libqwayland-egl.so loads (EGL on Wayland), key wayland-egl
   - xdg-shell integration plugin loads
   - XDG_RUNTIME_DIR
   - labwc (headless) is up: wayland-0
   - QGuiApplication on the wayland platform
   - xdg-shell window configured and exposed by labwc
   - window painted (wl_shm buffer)
   - window size 320 x 200

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qt6-wayland 6.8.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/qt6-wayland-6.8.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qt6-wayland 6.8.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/qt6-wayland-6.8.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.8.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qt6-wayland=<version>`), sources in `ported/qt6-wayland/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qt6-wayland`; port one with `scripts/port-update qt6-wayland`.

<details><summary>recipe.sh</summary>

```bash
# Qt 6 qtwayland, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwayland-everywhere-src-6.8.1.tar.xz
SHA256=2226fbde4e2ddd12f8bf4b239c8f38fd706a54e789e63467dfddc77129eca203
# Keyboard input without xkbcommon: Qt reads xkbcommon only through qtbase's
# xkbcommon feature, which ports/qt6 does not enable; the protocol XML Qt
# needs is in its own source (src/3rdparty/protocol).
DEPS="qt6 wayland"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
# The client side only: the wayland platform plugin (QT_QPA_PLATFORM=wayland,
# what labwc runs Qt programs with), its shell integrations (xdg-shell, ...)
# and the EGL client buffer integration; not the compositor library.
# qtwaylandscanner generates C++ from the protocol XML at build time: the host
# build makes only that tool (no client, no compositor) for the host Qt.
# The protocol XML is turned into C by wayland-scanner from hosttools/.
do_configure() {
  qt6_host_module libexec/qtwaylandscanner -DFEATURE_wayland_client=OFF -DFEATURE_wayland_server=OFF &&
  qt6_module_configure -DFEATURE_wayland_server=OFF -DFEATURE_wayland_client=ON
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
