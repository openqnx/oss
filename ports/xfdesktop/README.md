# xfdesktop 4.20.2

> Xfce desktop manager: background, desktop icons, applications menu and window list

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/xfdesktop/4.20/xfdesktop-4.20.2.tar.bz2 |
| SHA-256 | `1d9bd76015fb6e9aca05e73cd998c7c66ed4fc8c10b626e08fc2eb7c39df3f7b` |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`exo`](../exo/) [`garcon`](../garcon/) [`libxfce4windowing`](../libxfce4windowing/) [`thunar`](../thunar/) [`libnotify`](../libnotify/) [`libyaml`](../libyaml/) [`gtk-layer-shell`](../gtk-layer-shell/) [`wayland`](../wayland/) |
| Run-time dependencies | [`cairo`](../cairo/) [`exo`](../exo/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`libnotify`](../libnotify/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libyaml`](../libyaml/) [`pango`](../pango/) [`thunar`](../thunar/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfdesktop`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xfdesktop` and `xfdesktop-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfdesktop` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (70 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf exo garcon libxfce4windowing thunar libnotify libyaml gtk-layer-shell wayland
```

<details><summary>Full build chain (70)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libexif`](../libexif/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libnotify`](../libnotify/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`libyaml`](../libyaml/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`thunar`](../thunar/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/xfdesktop/4.20/xfdesktop-4.20.2.tar.bz2
mkdir -p dist src/xfdesktop
curl -fL -o dist/$(basename $URL) $URL
echo "1d9bd76015fb6e9aca05e73cd998c7c66ed4fc8c10b626e08fc2eb7c39df3f7b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfdesktop --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xfdesktop < ports/xfdesktop/patches/0001-find-libyaml-without-pkg-config.patch
```

**[`0001-find-libyaml-without-pkg-config.patch`](patches/0001-find-libyaml-without-pkg-config.patch)** — build: find libyaml without its pkg-config file

> The saved icon positions are YAML, read and written with libyaml, which
> meson looks up as the pkg-config module yaml-0.1. Only libyaml's Autotools
> build installs yaml-0.1.pc; a libyaml built with its CMake files (as the
> one in this tree is) installs the library, yaml.h and CMake package files,
> and no .pc. Fall back to looking for the library and its header, so file
> icons are not silently built out (or the configuration refused) because of
> the missing metadata.

Touches: `meson.build` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfdesktop src/xfdesktop \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dx11=disabled \
      -Dwayland=enabled \
      -Ddesktop-menu=enabled \
      -Ddesktop-icons=true \
      -Dfile-icons=true \
      -Dthunarx=enabled \
      -Dnotifications=enabled \
      -Dtests=false
ninja -C build/$QNX_ARCH/xfdesktop
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfdesktop install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfdesktop: the Xfce desktop manager -- draws the desktop background (image or colour, per monitor, optionally cycling through a folder), the icons on it (the files of ~/Desktop, home/file system/trash, minimized windows) and the right-click applications menu and the middle-click window list. On Wayland the desktop is a wlr-layer-shell surface in the background layer, so it needs a compositor with that protocol (labwc). xfdesktop-settings is its dialog.
- Release tarball, as for every Xfce package here: it carries the generated sources a git checkout makes with xfce4-dev-tools.
- garcon: the applications menu; thunar (libthunarx): the file manager's extensions in the icons' context menu; libnotify: mount/unmount messages; libyaml: the saved icon positions (~/.config/xfce4/desktop/icons.screen0.yaml).
- xfconf: every setting (channel xfce4-desktop) comes from xfconfd on the session bus; thunar: opening, renaming and deleting a file icon is done by the file manager over D-Bus (org.xfce.FileManager, with "Thunar" as the fallback). librsvg: the default background (xfce-x.svg) and most of the shipped ones are SVG, which gdk-pixbuf reads through librsvg's loader.
- - Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11); - no test programs.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfdesktop`.

### 5. What it installs

99 files under `/usr/local`, 2 programs.

```
/usr/local/bin/xfdesktop
/usr/local/bin/xfdesktop-settings
```

The complete list is `stage/<arch>/.manifest/xfdesktop` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfdesktop
ls dists/8.0/*/ | grep '^xfdesktop-'
```

This writes `xfdesktop-<ver>-r0.tar.gz` (run time) and `xfdesktop-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfdesktop
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfdesktop-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfdesktop-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfdesktop`](../../tests/src/) links xfdesktop the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfdesktop`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfdesktop
/usr/local/bin/qnx-port-tests/test_xfdesktop
```

```
== xfdesktop ==
  ok   ...
PASS xfdesktop
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfdesktop` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfdesktop`](../../tests/src/) -- a program linked against xfdesktop the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xfdesktop`.

## Versions

This recipe builds **4.20.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfdesktop=<version>`), sources in `ported/xfdesktop/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfdesktop`; port one with `scripts/port-update xfdesktop`.

<details><summary>recipe.sh</summary>

```bash
# xfdesktop: the Xfce desktop manager -- draws the desktop background (image
# or colour, per monitor, optionally cycling through a folder), the icons on it
# (the files of ~/Desktop, home/file system/trash, minimized windows) and the
# right-click applications menu and the middle-click window list. On Wayland
# the desktop is a wlr-layer-shell surface in the background layer, so it needs
# a compositor with that protocol (labwc). xfdesktop-settings is its dialog.
VERSION=4.20.2
# Release tarball, as for every Xfce package here: it carries the generated
# sources a git checkout makes with xfce4-dev-tools.
TARBALL=https://archive.xfce.org/src/xfce/xfdesktop/4.20/xfdesktop-4.20.2.tar.bz2
SHA256=1d9bd76015fb6e9aca05e73cd998c7c66ed4fc8c10b626e08fc2eb7c39df3f7b
# garcon: the applications menu; thunar (libthunarx): the file manager's
# extensions in the icons' context menu; libnotify: mount/unmount messages;
# libyaml: the saved icon positions (~/.config/xfce4/desktop/icons.screen0.yaml).
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf exo garcon libxfce4windowing thunar libnotify libyaml gtk-layer-shell wayland"
# xfconf: every setting (channel xfce4-desktop) comes from xfconfd on the
# session bus; thunar: opening, renaming and deleting a file icon is done by the
# file manager over D-Bus (org.xfce.FileManager, with "Thunar" as the fallback).
# librsvg: the default background (xfce-x.svg) and most of the shipped ones
# are SVG, which gdk-pixbuf reads through librsvg's loader.
RUNTIME_DEPS="dbus xfconf thunar librsvg"
BUILDSYS=meson
# - Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11);
# - no test programs.
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Ddesktop-menu=enabled -Ddesktop-icons=true
                   -Dfile-icons=true -Dthunarx=enabled -Dnotifications=enabled
                   -Dtests=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
