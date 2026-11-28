# qnx-desktop-core 1.0

> Desktop session set: labwc, Xfce panel/session/settings, file manager, terminal, D-Bus

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`oss-desktop`](../oss-desktop/) [`oss-greeter`](../oss-greeter/) [`labwc`](../labwc/) [`swaybg`](../swaybg/) [`xfce4-panel`](../xfce4-panel/) [`oss-network-plugin`](../oss-network-plugin/) [`xfce4-alsa-plugin`](../xfce4-alsa-plugin/) [`xfce4-session`](../xfce4-session/) [`xfce4-settings`](../xfce4-settings/) [`xfce4-terminal`](../xfce4-terminal/) [`thunar`](../thunar/) [`xfce4-appfinder`](../xfce4-appfinder/) [`xfconf`](../xfconf/) [`garcon`](../garcon/) [`exo`](../exo/) [`tumbler`](../tumbler/) [`dbus`](../dbus/) [`libnotify`](../libnotify/) [`xwayland`](../xwayland/) [`xkeyboard-config`](../xkeyboard-config/) [`xdg-utils`](../xdg-utils/) [`shared-mime-info`](../shared-mime-info/) [`desktop-file-utils`](../desktop-file-utils/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`glib-networking`](../glib-networking/) [`gnome-keyring`](../gnome-keyring/) [`dconf`](../dconf/) [`xfce4-notifyd`](../xfce4-notifyd/) [`xfdesktop`](../xfdesktop/) [`swayidle`](../swayidle/) [`swaylock`](../swaylock/) [`tzdata`](../tzdata/) [`librsvg`](../librsvg/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qnx_desktop`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `qnx-desktop-core` and `qnx-desktop-core-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qnx-desktop-core` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (153 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild oss-desktop oss-greeter labwc swaybg xfce4-panel oss-network-plugin xfce4-alsa-plugin xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder xfconf garcon exo tumbler dbus libnotify xwayland xkeyboard-config xdg-utils shared-mime-info desktop-file-utils gsettings-desktop-schemas glib-networking gnome-keyring dconf xfce4-notifyd xfdesktop swayidle swaylock tzdata librsvg
```

<details><summary>Full build chain (153)</summary>

[`adwaita-icon-theme`](../adwaita-icon-theme/) [`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`cairo`](../cairo/) [`curl`](../curl/) [`dav1d`](../dav1d/) [`dbus`](../dbus/) [`dconf`](../dconf/) [`desktop-file-utils`](../desktop-file-utils/) [`exo`](../exo/) [`expat`](../expat/) [`ffmpeg`](../ffmpeg/) [`ffmpegthumbnailer`](../ffmpegthumbnailer/) [`font-dejavu`](../font-dejavu/) [`font-util`](../font-util/) [`fontconfig`](../fontconfig/) [`fontenc`](../fontenc/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gcr`](../gcr/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`glib-networking`](../glib-networking/) [`gmp`](../gmp/) [`gnome-keyring`](../gnome-keyring/) [`gnutls`](../gnutls/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`harfbuzz`](../harfbuzz/) [`hicolor-icon-theme`](../hicolor-icon-theme/) [`icu`](../icu/) [`labwc`](../labwc/) [`lame`](../lame/) [`lcms2`](../lcms2/) [`libdrm-headers`](../libdrm-headers/) [`libepoxy`](../libepoxy/) [`libexif`](../libexif/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libgsf`](../libgsf/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libmd`](../libmd/) [`libnotify`](../libnotify/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libproxy`](../libproxy/) [`librsvg`](../librsvg/) [`libsecret`](../libsecret/) [`libsfdo`](../libsfdo/) [`libtasn1`](../libtasn1/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxcvt`](../libxcvt/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxfont2`](../libxfont2/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxkbfile`](../libxkbfile/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxshmfence`](../libxshmfence/) [`libxtst`](../libxtst/) [`libyaml`](../libyaml/) [`linux-headers-input-codes`](../linux-headers-input-codes/) [`lxde-icon-theme`](../lxde-icon-theme/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`mpg123`](../mpg123/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`nghttp2`](../nghttp2/) [`openh264`](../openh264/) [`openpam`](../openpam/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`oss-desktop`](../oss-desktop/) [`oss-greeter`](../oss-greeter/) [`oss-network-plugin`](../oss-network-plugin/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`rust`](../rust/) [`shared-mime-info`](../shared-mime-info/) [`sqlite`](../sqlite/) [`swaybg`](../swaybg/) [`swayidle`](../swayidle/) [`swaylock`](../swaylock/) [`thunar`](../thunar/) [`tumbler`](../tumbler/) [`tzdata`](../tzdata/) [`vte`](../vte/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`wlroots`](../wlroots/) [`x264`](../x264/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xdg-utils`](../xdg-utils/) [`xfce4-alsa-plugin`](../xfce4-alsa-plugin/) [`xfce4-appfinder`](../xfce4-appfinder/) [`xfce4-notifyd`](../xfce4-notifyd/) [`xfce4-panel`](../xfce4-panel/) [`xfce4-session`](../xfce4-session/) [`xfce4-settings`](../xfce4-settings/) [`xfce4-terminal`](../xfce4-terminal/) [`xfconf`](../xfconf/) [`xfdesktop`](../xfdesktop/) [`xkbcomp`](../xkbcomp/) [`xkeyboard-config`](../xkeyboard-config/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xwayland`](../xwayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-core/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qnx-desktop-core; B=build/$QNX_ARCH/qnx-desktop-core; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/core.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/core.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/core.list"
  install -Dm755 "$S/bin/qnx-desktop-start" "$STAGE$PREFIX/bin/qnx-desktop-start"
  install -Dm644 "$S/share/qnx-desktop/rc.local.sample" "$STAGE$PREFIX/share/qnx-desktop/rc.local.sample"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- qnx-desktop-core: the session of the desktop -- what Raspberry Pi OS puts in rpd-wayland-core. oss-desktop (the start script and the labwc/Xfce defaults) with every program the session runs: labwc and swaybg, the Xfce session, settings daemon, panel, application finder, Thunar and the terminal; the libraries' own services they talk to (xfconf, garcon's menus, exo's helpers, tumbler's thumbnails, D-Bus, notifications through libnotify); Xwayland for X11 programs; and the data a session needs at run time (keyboard layouts, MIME database, desktop-file tools, GSettings schemas, TLS for GIO, the secret store); the panel's network indicator (oss-network-plugin) and volume control (xfce4-alsa-plugin, on the QNX image's libasound -- "No sound card found" when io-snd is not running); the log-in screen (oss-greeter, shown when qnx-desktop.conf has DESKTOP_GREETER=yes). It also installs the boot entry of the session, bin/qnx-desktop-start, for a board image whose startup script runs /data/oss/etc/rc.local once Screen is up (files/; share/qnx-desktop/rc.local.sample is that rc.local). Its settings are in /data/oss/etc/qnx-desktop.conf (see the script's header), among them TZ, the time zone of the session: QNX's C library reads only POSIX strings (KST-9), so an area name (TZ=Asia/Seoul) is turned into one from tzdata; without a setting the system's (setconf _CS_TIMEZONE) is used, else UTC. The system D-Bus needs a "messagebus" account that the package does not add (/etc/passwd belongs to the BSP): qnx-desktop-start --check prints the two lines to add; the session runs without it. This component holds no program of its own: it is a meta package. Installing it installs the components below (DEPS builds them first, RUNTIME_DEPS is what the package depends on; keep the two lines the same). To add when ported (not in ports/ yet -- move a name into DEPS and RUNTIME_DEPS once ports/<name> exists and its package is verified): kanshi wlopm          output profiles and output power (rpd-wayland-core has them) a greeter / log-in screen, a first-boot wizard (not ported, not written yet)
- The package must hold a file to exist (qnxpkg writes no package for an empty file list): the member list, share/qnx-desktop/core.list -- the header from files/ and one member per line, written from RUNTIME_DEPS so that the list on the target is what the package depends on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qnx-desktop-core`.

### 5. What it installs

3 files under `/usr/local`, 1 programs.

```
/usr/local/bin/qnx-desktop-start
```

The complete list is `stage/<arch>/.manifest/qnx-desktop-core` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qnx-desktop-core
ls dists/8.0/*/ | grep '^qnx-desktop-core-'
```

This writes `qnx-desktop-core-<ver>-r0.tar.gz` (run time) and `qnx-desktop-core-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qnx-desktop-core
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qnx-desktop-core-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qnx-desktop-core-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qnx_desktop`](../../tests/src/) links qnx-desktop-core the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qnx-desktop`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qnx-desktop
/usr/local/bin/qnx-port-tests/test_qnx_desktop
```

```
== qnx-desktop-core ==
  ok   ...
PASS qnx-desktop-core
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qnx_desktop` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qnx_desktop`](../../tests/src/) -- a program linked against qnx-desktop-core the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> qnx-desktop-core`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qnx-desktop-core=<version>`), sources in `ported/qnx-desktop-core/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qnx-desktop-core`; port one with `scripts/port-update qnx-desktop-core`.

<details><summary>recipe.sh</summary>

```bash
# qnx-desktop-core: the session of the desktop -- what Raspberry Pi OS puts in
# rpd-wayland-core. oss-desktop (the start script and the labwc/Xfce defaults)
# with every program the session runs: labwc and swaybg, the Xfce session,
# settings daemon, panel, application finder, Thunar and the terminal; the
# libraries' own services they talk to (xfconf, garcon's menus, exo's helpers,
# tumbler's thumbnails, D-Bus, notifications through libnotify); Xwayland for
# X11 programs; and the data a session needs at run time (keyboard layouts,
# MIME database, desktop-file tools, GSettings schemas, TLS for GIO, the
# secret store); the panel's network indicator (oss-network-plugin) and
# volume control (xfce4-alsa-plugin, on the QNX image's libasound -- "No
# sound card found" when io-snd is not running); the log-in screen
# (oss-greeter, shown when qnx-desktop.conf has DESKTOP_GREETER=yes).
# It also installs the boot entry of the session, bin/qnx-desktop-start, for
# a board image whose startup script runs /data/oss/etc/rc.local once Screen
# is up (files/; share/qnx-desktop/rc.local.sample is that rc.local).
# Its settings are in /data/oss/etc/qnx-desktop.conf (see the script's
# header), among them TZ, the time zone of the session: QNX's C library reads
# only POSIX strings (KST-9), so an area name (TZ=Asia/Seoul) is turned into
# one from tzdata; without a setting the system's (setconf _CS_TIMEZONE) is
# used, else UTC. The system D-Bus needs a "messagebus" account that the
# package does not add (/etc/passwd belongs to the BSP): qnx-desktop-start
# --check prints the two lines to add; the session runs without it.
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   kanshi wlopm          output profiles and output power (rpd-wayland-core has them)
#   a greeter / log-in screen, a first-boot wizard (not ported, not written yet)
VERSION=1.0
DEPS="oss-desktop oss-greeter labwc swaybg xfce4-panel oss-network-plugin xfce4-alsa-plugin xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder xfconf garcon exo tumbler dbus libnotify xwayland xkeyboard-config xdg-utils shared-mime-info desktop-file-utils gsettings-desktop-schemas glib-networking gnome-keyring dconf xfce4-notifyd xfdesktop swayidle swaylock tzdata librsvg"
RUNTIME_DEPS="oss-desktop oss-greeter labwc swaybg xfce4-panel oss-network-plugin xfce4-alsa-plugin xfce4-session xfce4-settings xfce4-terminal thunar xfce4-appfinder xfconf garcon exo tumbler dbus libnotify xwayland xkeyboard-config xdg-utils shared-mime-info desktop-file-utils gsettings-desktop-schemas glib-networking gnome-keyring dconf xfce4-notifyd xfdesktop swayidle swaylock tzdata librsvg"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-core/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/core.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/core.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/core.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/core.list"
  install -Dm755 "$S/bin/qnx-desktop-start" "$STAGE$PREFIX/bin/qnx-desktop-start"
  install -Dm644 "$S/share/qnx-desktop/rc.local.sample" "$STAGE$PREFIX/share/qnx-desktop/rc.local.sample"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
