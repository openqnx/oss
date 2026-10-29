# xfce4-settings 4.20.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/xfce4-settings/4.20/xfce4-settings-4.20.1.tar.bz2 |
| SHA-256 | `fd0d602853ea75d94024e5baae2d2bf5ca8f8aa4dad7bfd5d08f9ff8afee77b2` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`garcon`](../garcon/) [`exo`](../exo/) [`libxfce4windowing`](../libxfce4windowing/) [`gtk-layer-shell`](../gtk-layer-shell/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`exo`](../exo/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`wayland`](../wayland/) [`xfconf`](../xfconf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xfce4-settings
apk add xfce4-settings-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xfce4-settings`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-settings` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell
```

<details><summary>Full build chain (59)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/xfce4-settings/4.20/xfce4-settings-4.20.1.tar.bz2
mkdir -p dist src/xfce4-settings
curl -fL -o dist/$(basename $URL) $URL
echo "fd0d602853ea75d94024e5baae2d2bf5ca8f8aa4dad7bfd5d08f9ff8afee77b2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-settings --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/xfce4-settings && cd build/$QNX_ARCH/xfce4-settings
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" \
$OLDPWD/src/xfce4-settings/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-x11 \
    --enable-wayland \
    --disable-xrandr \
    --disable-xcursor \
    --disable-libnotify \
    --disable-colord \
    --disable-upower-glib
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-settings: the settings manager and dialogs. The QNX Everywhere 8.0.4r1 image has the 4.21 development snapshot; the 4.20 release is used here to match the Xfce 4.20 libraries in this tree.
- Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-settings`.

### 5. What it installs

239 files under `/usr/local`, 7 programs.

```
/usr/local/bin/xfce4-appearance-settings
/usr/local/bin/xfce4-display-settings
/usr/local/bin/xfce4-mime-helper
/usr/local/bin/xfce4-mime-settings
/usr/local/bin/xfce4-settings-editor
/usr/local/bin/xfce4-settings-manager
/usr/local/bin/xfsettingsd
```

The complete list is `stage/<arch>/.manifest/xfce4-settings` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-settings
ls dists/8.0/*/ | grep '^xfce4-settings-'
```

This writes `xfce4-settings-<ver>-r0.tar.gz` (run time) and `xfce4-settings-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-settings
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-settings-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-settings-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **4.20.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-settings=<version>`), sources in `ported/xfce4-settings/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-settings`; port one with `scripts/port-update xfce4-settings`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-settings: the settings manager and dialogs.
# The QNX Everywhere 8.0.4r1 image has the 4.21 development snapshot; the
# 4.20 release is used here to match the Xfce 4.20 libraries in this tree.
VERSION=4.20.1
TARBALL=https://archive.xfce.org/src/xfce/xfce4-settings/4.20/xfce4-settings-4.20.1.tar.bz2
SHA256=fd0d602853ea75d94024e5baae2d2bf5ca8f8aa4dad7bfd5d08f9ff8afee77b2
# Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf garcon exo libxfce4windowing gtk-layer-shell"
BUILDSYS=autotools
CONF_ENV=( WAYLAND_SCANNER="$WS/hosttools/bin/wayland-scanner" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc --disable-x11 --enable-wayland --disable-xrandr --disable-xcursor --disable-libnotify --disable-colord --disable-upower-glib )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
