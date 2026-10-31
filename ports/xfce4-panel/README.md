# xfce4-panel 4.20.0

> Xfce panel with launchers, menus and workspace switcher

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/xfce4-panel/4.20/xfce4-panel-4.20.0.tar.bz2 |
| SHA-256 | `ff33cd5f5d16c2193fe305f4878d82cd8d2feea92f2594bcd27b2b5c392d43b8` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`garcon`](../garcon/) [`exo`](../exo/) [`libxfce4windowing`](../libxfce4windowing/) [`gtk-layer-shell`](../gtk-layer-shell/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`exo`](../exo/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`pango`](../pango/) [`wayland`](../wayland/) [`xfconf`](../xfconf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfce4_panel`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xfce4-panel
apk add xfce4-panel-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xfce4-panel && /usr/local/bin/qnx-port-tests/test_xfce4_panel    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xfce4-panel`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-panel` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (59 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols
```

<details><summary>Full build chain (59)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/xfce4-panel/4.20/xfce4-panel-4.20.0.tar.bz2
mkdir -p dist src/xfce4-panel
curl -fL -o dist/$(basename $URL) $URL
echo "ff33cd5f5d16c2193fe305f4878d82cd8d2feea92f2594bcd27b2b5c392d43b8  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-panel --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/xfce4-panel && cd build/$QNX_ARCH/xfce4-panel
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" \
$OLDPWD/src/xfce4-panel/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-introspection \
    --disable-vala \
    --disable-x11 \
    --enable-wayland \
    --enable-gtk-layer-shell
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Release tarball, as for every Xfce package here -- see README hazard 74.
- Wayland only, matching libxfce4windowing, which was built --disable-x11: the panel's X11 half needs libwnck, and libwnck's window tracking is X11 through and through.  gtk-layer-shell is what lets the panel anchor itself to a screen edge under a wlroots-style compositor.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-panel`.

### 5. What it installs

204 files under `/usr/local`: 3 shared-library entries, 4 programs, 10 headers.

```
/usr/local/lib/libxfce4panel-2.0.so
/usr/local/lib/libxfce4panel-2.0.so.4
/usr/local/lib/libxfce4panel-2.0.so.4.0.0
/usr/local/bin/xfce4-panel
/usr/local/bin/xfce4-popup-applicationsmenu
/usr/local/bin/xfce4-popup-directorymenu
/usr/local/bin/xfce4-popup-windowmenu
```

The complete list is `stage/<arch>/.manifest/xfce4-panel` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-panel
ls dists/8.0/*/ | grep '^xfce4-panel-'
```

This writes `xfce4-panel-<ver>-r0.tar.gz` (run time) and `xfce4-panel-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-panel
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-panel-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-panel-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfce4_panel`](../../tests/src/) links xfce4-panel the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfce4-panel`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfce4-panel
/usr/local/bin/qnx-port-tests/test_xfce4_panel
```

```
== xfce4-panel ==
  ok   ...
PASS xfce4-panel
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfce4_panel` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfce4_panel`](../../tests/src/) -- a program linked against xfce4-panel the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libxfce4panel 4.20.0
   - libxfce4panel_check_version
   - xfconf channel name xfce4-panel
   - XfcePanelPluginMode enum: deskbar
   - XfceScreenPosition enum: floating-v
   - XfcePanelPlugin has its 14 documented properties
   - all 11 plugins: .desktop -> module -> xfce_panel_module_init registers an XfcePanelPlugin type
   - default panel layout (xfconf XML) installed
   - scratch directory
   - xfce4-panel --version: 4.20.0
   - xfce4-panel --help lists --preferences, --add
   - XfceArrowButton: arrow up
   - XfceArrowButton: set arrow left, blinking
   - gtk_init_check on the headless display
   - without a display xfce4-panel stops at the display probe

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xfce4-panel 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xfce4-panel-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xfce4-panel 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xfce4-panel-4.20.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-panel=<version>`), sources in `ported/xfce4-panel/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-panel`; port one with `scripts/port-update xfce4-panel`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
# Release tarball, as for every Xfce package here -- see README hazard 74.
TARBALL=https://archive.xfce.org/src/xfce/xfce4-panel/4.20/xfce4-panel-4.20.0.tar.bz2
SHA256=ff33cd5f5d16c2193fe305f4878d82cd8d2feea92f2594bcd27b2b5c392d43b8
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell wayland wayland-protocols"
BUILDSYS=autotools
# Wayland only, matching libxfce4windowing, which was built --disable-x11: the
# panel's X11 half needs libwnck, and libwnck's window tracking is X11 through
# and through.  gtk-layer-shell is what lets the panel anchor itself to a screen
# edge under a wlroots-style compositor.
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-vala --disable-x11
                  --enable-wayland --enable-gtk-layer-shell )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
