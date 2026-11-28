# qnx-desktop 1.0

> Complete desktop for QNX: session, applications, developer tools (meta package)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`qnx-desktop-core`](../qnx-desktop-core/) [`qnx-desktop-theme`](../qnx-desktop-theme/) [`qnx-desktop-preferences`](../qnx-desktop-preferences/) [`qnx-desktop-utilities`](../qnx-desktop-utilities/) [`qnx-desktop-applications`](../qnx-desktop-applications/) [`qnx-desktop-developer`](../qnx-desktop-developer/) [`qnx-desktop-graphics`](../qnx-desktop-graphics/) [`qnx-desktop-extras`](../qnx-desktop-extras/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qnx_desktop`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `qnx-desktop` and `qnx-desktop-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qnx-desktop` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (288 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild qnx-desktop-core qnx-desktop-theme qnx-desktop-preferences qnx-desktop-utilities qnx-desktop-applications qnx-desktop-developer qnx-desktop-graphics qnx-desktop-extras
```

<details><summary>Full build chain (288)</summary>

[`7zip`](../7zip/) [`abiword`](../abiword/) [`ada`](../ada/) [`adwaita-fonts`](../adwaita-fonts/) [`adwaita-icon-theme`](../adwaita-icon-theme/) [`aom`](../aom/) [`aspell`](../aspell/) [`at-spi2-core`](../at-spi2-core/) [`avahi`](../avahi/) [`binutils`](../binutils/) [`boost`](../boost/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`cairo`](../cairo/) [`chrony`](../chrony/) [`cmake`](../cmake/) [`cups`](../cups/) [`cups-filters`](../cups-filters/) [`curl`](../curl/) [`dav1d`](../dav1d/) [`dbus`](../dbus/) [`dbus-glib`](../dbus-glib/) [`dconf`](../dconf/) [`desktop-file-utils`](../desktop-file-utils/) [`enchant`](../enchant/) [`epiphany`](../epiphany/) [`evince`](../evince/) [`exiv2`](../exiv2/) [`exo`](../exo/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`ffmpeg`](../ffmpeg/) [`ffmpegthumbnailer`](../ffmpegthumbnailer/) [`file`](../file/) [`flac`](../flac/) [`font-cantarell`](../font-cantarell/) [`font-dejavu`](../font-dejavu/) [`font-hack`](../font-hack/) [`font-inconsolata`](../font-inconsolata/) [`font-inter`](../font-inter/) [`font-liberation`](../font-liberation/) [`font-noto-emoji`](../font-noto-emoji/) [`font-noto-sans-cjk`](../font-noto-sans-cjk/) [`font-util`](../font-util/) [`fontconfig`](../fontconfig/) [`fontenc`](../fontenc/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gcr`](../gcr/) [`gdb`](../gdb/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`geany`](../geany/) [`geany-plugins`](../geany-plugins/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`git`](../git/) [`glib`](../glib/) [`glib-networking`](../glib-networking/) [`gmp`](../gmp/) [`gnome-keyring`](../gnome-keyring/) [`gnumeric`](../gnumeric/) [`gnutls`](../gnutls/) [`gobject-introspection`](../gobject-introspection/) [`goffice`](../goffice/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`gst-libav`](../gst-libav/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gst-plugins-good`](../gst-plugins-good/) [`gst-plugins-ugly`](../gst-plugins-ugly/) [`gstreamer`](../gstreamer/) [`gthumb`](../gthumb/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk-vnc`](../gtk-vnc/) [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`gtksourceview4`](../gtksourceview4/) [`harfbuzz`](../harfbuzz/) [`hicolor-icon-theme`](../hicolor-icon-theme/) [`highway`](../highway/) [`htop`](../htop/) [`hyphen`](../hyphen/) [`icu`](../icu/) [`intltool`](../intltool/) [`ippusbxd`](../ippusbxd/) [`iso-codes`](../iso-codes/) [`json-glib`](../json-glib/) [`labwc`](../labwc/) [`lame`](../lame/) [`lcms2`](../lcms2/) [`libarchive`](../libarchive/) [`libavif`](../libavif/) [`libdaemon`](../libdaemon/) [`libdazzle`](../libdazzle/) [`libdrm-headers`](../libdrm-headers/) [`libepoxy`](../libepoxy/) [`libexif`](../libexif/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libgsf`](../libgsf/) [`libhandy`](../libhandy/) [`libice`](../libice/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libinih`](../libinih/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libmd`](../libmd/) [`libnotify`](../libnotify/) [`libogg`](../libogg/) [`libpaper`](../libpaper/) [`libpng`](../libpng/) [`libportal`](../libportal/) [`libproxy`](../libproxy/) [`libpsl`](../libpsl/) [`librsvg`](../librsvg/) [`libsecret`](../libsecret/) [`libsfdo`](../libsfdo/) [`libsm`](../libsm/) [`libsoup`](../libsoup/) [`libtasn1`](../libtasn1/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libusb`](../libusb/) [`libuv`](../libuv/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcrypt`](../libxcrypt/) [`libxcursor`](../libxcursor/) [`libxcvt`](../libxcvt/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxfont2`](../libxfont2/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxkbfile`](../libxkbfile/) [`libxml2`](../libxml2/) [`libxmu`](../libxmu/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxshmfence`](../libxshmfence/) [`libxslt`](../libxslt/) [`libxt`](../libxt/) [`libxtst`](../libxtst/) [`libyaml`](../libyaml/) [`libyuv`](../libyuv/) [`linux-headers-input-codes`](../linux-headers-input-codes/) [`llvm`](../llvm/) [`lxde-icon-theme`](../lxde-icon-theme/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`make`](../make/) [`mate-calc`](../mate-calc/) [`mesa`](../mesa/) [`meson`](../meson/) [`mousepad`](../mousepad/) [`mpc`](../mpc/) [`mpfr`](../mpfr/) [`mpg123`](../mpg123/) [`msgpack-c`](../msgpack-c/) [`nano`](../nano/) [`native-toolchain`](../native-toolchain/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`nghttp2`](../nghttp2/) [`ninja`](../ninja/) [`nodejs`](../nodejs/) [`npm`](../npm/) [`numpy`](../numpy/) [`openblas`](../openblas/) [`openh264`](../openh264/) [`openjpeg`](../openjpeg/) [`openpam`](../openpam/) [`openssh`](../openssh/) [`openssl`](../openssl/) [`opus`](../opus/) [`opusfile`](../opusfile/) [`orc`](../orc/) [`oss-desktop`](../oss-desktop/) [`oss-greeter`](../oss-greeter/) [`oss-network-plugin`](../oss-network-plugin/) [`oss-software`](../oss-software/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) [`parole`](../parole/) [`pcre2`](../pcre2/) [`perl`](../perl/) [`perl-error`](../perl-error/) [`pixman`](../pixman/) [`pkgconf`](../pkgconf/) [`poppler`](../poppler/) [`pthread-stubs`](../pthread-stubs/) [`pygame`](../pygame/) [`python3`](../python3/) [`python3-distlib`](../python3-distlib/) [`python3-filelock`](../python3-filelock/) [`python3-packaging`](../python3-packaging/) [`python3-pip`](../python3-pip/) [`python3-platformdirs`](../python3-platformdirs/) [`python3-setuptools`](../python3-setuptools/) [`python3-virtualenv`](../python3-virtualenv/) [`python3-wheel`](../python3-wheel/) [`qnx-desktop-applications`](../qnx-desktop-applications/) [`qnx-desktop-core`](../qnx-desktop-core/) [`qnx-desktop-developer`](../qnx-desktop-developer/) [`qnx-desktop-extras`](../qnx-desktop-extras/) [`qnx-desktop-graphics`](../qnx-desktop-graphics/) [`qnx-desktop-preferences`](../qnx-desktop-preferences/) [`qnx-desktop-theme`](../qnx-desktop-theme/) [`qnx-desktop-utilities`](../qnx-desktop-utilities/) [`qpdf`](../qpdf/) [`readline`](../readline/) [`rust`](../rust/) [`sdl2`](../sdl2/) [`sdl2-image`](../sdl2-image/) [`sdl2-mixer`](../sdl2-mixer/) [`sdl2-ttf`](../sdl2-ttf/) [`shared-mime-info`](../shared-mime-info/) [`simdjson`](../simdjson/) [`simdutf`](../simdutf/) [`sqlite`](../sqlite/) [`sudo`](../sudo/) [`svt-av1`](../svt-av1/) [`swaybg`](../swaybg/) [`swayidle`](../swayidle/) [`swaylock`](../swaylock/) [`taglib`](../taglib/) [`thunar`](../thunar/) [`tumbler`](../tumbler/) [`tzdata`](../tzdata/) [`vte`](../vte/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`webkitgtk`](../webkitgtk/) [`webrtc`](../webrtc/) [`webrtc-screencast`](../webrtc-screencast/) [`wlr-protocols`](../wlr-protocols/) [`wlroots`](../wlroots/) [`woff2`](../woff2/) [`wv`](../wv/) [`x264`](../x264/) [`x265`](../x265/) [`xarchiver`](../xarchiver/) [`xcb-proto`](../xcb-proto/) [`xdg-user-dirs`](../xdg-user-dirs/) [`xdg-utils`](../xdg-utils/) [`xfce4-alsa-plugin`](../xfce4-alsa-plugin/) [`xfce4-appfinder`](../xfce4-appfinder/) [`xfce4-cpugraph-plugin`](../xfce4-cpugraph-plugin/) [`xfce4-genmon-plugin`](../xfce4-genmon-plugin/) [`xfce4-notifyd`](../xfce4-notifyd/) [`xfce4-panel`](../xfce4-panel/) [`xfce4-screenshooter`](../xfce4-screenshooter/) [`xfce4-session`](../xfce4-session/) [`xfce4-settings`](../xfce4-settings/) [`xfce4-taskmanager`](../xfce4-taskmanager/) [`xfce4-terminal`](../xfce4-terminal/) [`xfconf`](../xfconf/) [`xfdesktop`](../xfdesktop/) [`xkbcomp`](../xkbcomp/) [`xkeyboard-config`](../xkeyboard-config/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xwayland`](../xwayland/) [`xxhash`](../xxhash/) [`xz`](../xz/) [`zip`](../zip/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qnx-desktop; B=build/$QNX_ARCH/qnx-desktop; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/qnx-desktop.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/qnx-desktop.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/qnx-desktop.list"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- qnx-desktop: the whole desktop with one command, "apk add qnx-desktop" -- a session like Raspberry Pi OS' on QNX Screen, from the ported components. Like rpd-wayland-all there, it is only the sum of the groups below, each a meta package of its own that can be installed alone (a board short of space takes qnx-desktop-core and qnx-desktop-theme: the session and its look): qnx-desktop-core          the session: labwc, Xfce panel/session/settings, Thunar, terminal, D-Bus; the boot entry qnx-desktop-start qnx-desktop-theme         icon themes, fonts (CJK, emoji) qnx-desktop-preferences   printing, service discovery, time qnx-desktop-utilities     editor, image viewer, calculator, archives qnx-desktop-applications  web browser, media players qnx-desktop-developer     compiler, CMake, Python, Git, GDB, Node.js qnx-desktop-graphics      GStreamer plugin sets, FFmpeg, Mesa qnx-desktop-extras        desktop in a browser, VNC viewer, SSH, sudo What QNX itself must provide (Screen with the board's graphics support, input, network, a writable /data) is not a package: the board image carries it. To add when written (not in ports/ yet): qnx-desktop-pi            board tools: vcgencmd/pinctrl counterparts, a configuration tool, panel temperature/voltage
- The package must hold a file to exist (qnxpkg writes no package for an empty file list): the member list, share/qnx-desktop/qnx-desktop.list -- the header from files/ and one member per line, written from RUNTIME_DEPS so that the list on the target is what the package depends on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qnx-desktop`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/qnx-desktop` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qnx-desktop
ls dists/8.0/*/ | grep '^qnx-desktop-'
```

This writes `qnx-desktop-<ver>-r0.tar.gz` (run time) and `qnx-desktop-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qnx-desktop
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qnx-desktop-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qnx-desktop-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qnx_desktop`](../../tests/src/) links qnx-desktop the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qnx-desktop`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qnx-desktop
/usr/local/bin/qnx-port-tests/test_qnx_desktop
```

```
== qnx-desktop ==
  ok   ...
PASS qnx-desktop
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qnx_desktop` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qnx_desktop`](../../tests/src/) -- a program linked against qnx-desktop the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> qnx-desktop`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qnx-desktop=<version>`), sources in `ported/qnx-desktop/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qnx-desktop`; port one with `scripts/port-update qnx-desktop`.

<details><summary>recipe.sh</summary>

```bash
# qnx-desktop: the whole desktop with one command, "apk add qnx-desktop" --
# a session like Raspberry Pi OS' on QNX Screen, from the ported components.
# Like rpd-wayland-all there, it is only the sum of the groups below, each a
# meta package of its own that can be installed alone (a board short of space
# takes qnx-desktop-core and qnx-desktop-theme: the session and its look):
#   qnx-desktop-core          the session: labwc, Xfce panel/session/settings,
#                             Thunar, terminal, D-Bus; the boot entry
#                             qnx-desktop-start
#   qnx-desktop-theme         icon themes, fonts (CJK, emoji)
#   qnx-desktop-preferences   printing, service discovery, time
#   qnx-desktop-utilities     editor, image viewer, calculator, archives
#   qnx-desktop-applications  web browser, media players
#   qnx-desktop-developer     compiler, CMake, Python, Git, GDB, Node.js
#   qnx-desktop-graphics      GStreamer plugin sets, FFmpeg, Mesa
#   qnx-desktop-extras        desktop in a browser, VNC viewer, SSH, sudo
# What QNX itself must provide (Screen with the board's graphics support,
# input, network, a writable /data) is not a package: the board image carries it.
#
# To add when written (not in ports/ yet):
#   qnx-desktop-pi            board tools: vcgencmd/pinctrl counterparts, a
#                             configuration tool, panel temperature/voltage
VERSION=1.0
DEPS="qnx-desktop-core qnx-desktop-theme qnx-desktop-preferences qnx-desktop-utilities qnx-desktop-applications qnx-desktop-developer qnx-desktop-graphics qnx-desktop-extras"
RUNTIME_DEPS="qnx-desktop-core qnx-desktop-theme qnx-desktop-preferences qnx-desktop-utilities qnx-desktop-applications qnx-desktop-developer qnx-desktop-graphics qnx-desktop-extras"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/qnx-desktop.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/qnx-desktop.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/qnx-desktop.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/qnx-desktop.list"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
