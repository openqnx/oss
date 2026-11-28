# qnx-desktop-utilities 1.0

> Desktop utilities: text editor, image viewer, calculator, archives, task monitor

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`geany`](../geany/) [`gthumb`](../gthumb/) [`mate-calc`](../mate-calc/) [`htop`](../htop/) [`ffmpegthumbnailer`](../ffmpegthumbnailer/) [`7zip`](../7zip/) [`zip`](../zip/) [`nano`](../nano/) [`evince`](../evince/) [`xdg-user-dirs`](../xdg-user-dirs/) [`xfce4-genmon-plugin`](../xfce4-genmon-plugin/) [`xfce4-cpugraph-plugin`](../xfce4-cpugraph-plugin/) [`xarchiver`](../xarchiver/) [`mousepad`](../mousepad/) [`xfce4-taskmanager`](../xfce4-taskmanager/) [`xfce4-screenshooter`](../xfce4-screenshooter/) [`oss-software`](../oss-software/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qnx_desktop`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `qnx-desktop-utilities` and `qnx-desktop-utilities-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qnx-desktop-utilities` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (121 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild geany gthumb mate-calc htop ffmpegthumbnailer 7zip zip nano evince xdg-user-dirs xfce4-genmon-plugin xfce4-cpugraph-plugin xarchiver mousepad xfce4-taskmanager xfce4-screenshooter oss-software
```

<details><summary>Full build chain (121)</summary>

[`7zip`](../7zip/) [`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`dbus`](../dbus/) [`evince`](../evince/) [`exiv2`](../exiv2/) [`exo`](../exo/) [`expat`](../expat/) [`ffmpeg`](../ffmpeg/) [`ffmpegthumbnailer`](../ffmpegthumbnailer/) [`file`](../file/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`geany`](../geany/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`gmp`](../gmp/) [`graphite2`](../graphite2/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`gthumb`](../gthumb/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`gtksourceview4`](../gtksourceview4/) [`harfbuzz`](../harfbuzz/) [`htop`](../htop/) [`icu`](../icu/) [`intltool`](../intltool/) [`json-glib`](../json-glib/) [`lame`](../lame/) [`lcms2`](../lcms2/) [`libarchive`](../libarchive/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libhandy`](../libhandy/) [`libice`](../libice/) [`libiconv`](../libiconv/) [`libinih`](../libinih/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libsm`](../libsm/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxmu`](../libxmu/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxt`](../libxt/) [`libxtst`](../libxtst/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`mate-calc`](../mate-calc/) [`mousepad`](../mousepad/) [`mpc`](../mpc/) [`mpfr`](../mpfr/) [`mpg123`](../mpg123/) [`nano`](../nano/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openjpeg`](../openjpeg/) [`openssl`](../openssl/) [`opus`](../opus/) [`oss-software`](../oss-software/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`poppler`](../poppler/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`x264`](../x264/) [`x265`](../x265/) [`xarchiver`](../xarchiver/) [`xcb-proto`](../xcb-proto/) [`xdg-user-dirs`](../xdg-user-dirs/) [`xfce4-cpugraph-plugin`](../xfce4-cpugraph-plugin/) [`xfce4-genmon-plugin`](../xfce4-genmon-plugin/) [`xfce4-panel`](../xfce4-panel/) [`xfce4-screenshooter`](../xfce4-screenshooter/) [`xfce4-taskmanager`](../xfce4-taskmanager/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zip`](../zip/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-utilities/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qnx-desktop-utilities; B=build/$QNX_ARCH/qnx-desktop-utilities; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/utilities.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/utilities.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/utilities.list"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- qnx-desktop-utilities: the small programs of a desktop -- Raspberry Pi OS' rpd-utilities (mousepad, eom, galculator, xarchiver, lxtask). With what is ported: Mousepad as the text editor (Geany for programming), gThumb as the image viewer, MATE Calculator, the Xfce Task Manager (htop in a terminal too), the Xfce Screenshooter (screen, window or region; through wlr-screencopy), Xarchiver, 7-Zip and zip for archives, Evince for PDF, nano, "Software" (oss-software: add, remove and update programs with apk), and ffmpegthumbnailer for the video thumbnails Thunar shows through tumbler. This component holds no program of its own: it is a meta package. Installing it installs the components below (DEPS builds them first, RUNTIME_DEPS is what the package depends on; keep the two lines the same). To add when ported (not in ports/ yet -- move a name into DEPS and RUNTIME_DEPS once ports/<name> exists and its package is verified): xfce4-genmon-plugin xfce4-cpugraph-plugin   panel monitors Left out on purpose (ported; install by name when wanted): qdirstat              brings Qt 6 (166 MB installed) for one program pqiv btop             a second image viewer / a second task monitor
- The package must hold a file to exist (qnxpkg writes no package for an empty file list): the member list, share/qnx-desktop/utilities.list -- the header from files/ and one member per line, written from RUNTIME_DEPS so that the list on the target is what the package depends on.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qnx-desktop-utilities`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/qnx-desktop-utilities` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qnx-desktop-utilities
ls dists/8.0/*/ | grep '^qnx-desktop-utilities-'
```

This writes `qnx-desktop-utilities-<ver>-r0.tar.gz` (run time) and `qnx-desktop-utilities-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qnx-desktop-utilities
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qnx-desktop-utilities-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qnx-desktop-utilities-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qnx_desktop`](../../tests/src/) links qnx-desktop-utilities the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qnx-desktop`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qnx-desktop
/usr/local/bin/qnx-port-tests/test_qnx_desktop
```

```
== qnx-desktop-utilities ==
  ok   ...
PASS qnx-desktop-utilities
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qnx_desktop` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qnx_desktop`](../../tests/src/) -- a program linked against qnx-desktop-utilities the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> qnx-desktop-utilities`.

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qnx-desktop-utilities=<version>`), sources in `ported/qnx-desktop-utilities/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qnx-desktop-utilities`; port one with `scripts/port-update qnx-desktop-utilities`.

<details><summary>recipe.sh</summary>

```bash
# qnx-desktop-utilities: the small programs of a desktop -- Raspberry Pi OS'
# rpd-utilities (mousepad, eom, galculator, xarchiver, lxtask). With what is
# ported: Mousepad as the text editor (Geany for programming), gThumb as the
# image viewer, MATE Calculator, the Xfce Task Manager (htop in a terminal too),
# the Xfce Screenshooter (screen, window or region; through wlr-screencopy),
# Xarchiver, 7-Zip and zip for archives, Evince for PDF, nano, "Software"
# (oss-software: add, remove and update programs with apk), and
# ffmpegthumbnailer for the video thumbnails Thunar shows through tumbler.
# This component holds no program of its own: it is a meta package. Installing
# it installs the components below (DEPS builds them first, RUNTIME_DEPS is
# what the package depends on; keep the two lines the same).
#
# To add when ported (not in ports/ yet -- move a name into DEPS and
# RUNTIME_DEPS once ports/<name> exists and its package is verified):
#   xfce4-genmon-plugin xfce4-cpugraph-plugin   panel monitors
# Left out on purpose (ported; install by name when wanted):
#   qdirstat              brings Qt 6 (166 MB installed) for one program
#   pqiv btop             a second image viewer / a second task monitor
VERSION=1.0
DEPS="geany gthumb mate-calc htop ffmpegthumbnailer 7zip zip nano evince xdg-user-dirs xfce4-genmon-plugin xfce4-cpugraph-plugin xarchiver mousepad xfce4-taskmanager xfce4-screenshooter oss-software"
RUNTIME_DEPS="geany gthumb mate-calc htop ffmpegthumbnailer 7zip zip nano evince xdg-user-dirs xfce4-genmon-plugin xfce4-cpugraph-plugin xarchiver mousepad xfce4-taskmanager xfce4-screenshooter oss-software"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/qnx-desktop-utilities/files/." "$SRCROOT/"; }
do_build() { :; }
# The package must hold a file to exist (qnxpkg writes no package for an empty
# file list): the member list, share/qnx-desktop/utilities.list -- the header
# from files/ and one member per line, written from RUNTIME_DEPS so that the
# list on the target is what the package depends on.
do_install() {
  install -d "$STAGE$PREFIX/share/qnx-desktop"
  { cat "$S/share/qnx-desktop/utilities.list.in"; printf '%s\n' $RUNTIME_DEPS; } > "$STAGE$PREFIX/share/qnx-desktop/utilities.list"
  chmod 644 "$STAGE$PREFIX/share/qnx-desktop/utilities.list"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
