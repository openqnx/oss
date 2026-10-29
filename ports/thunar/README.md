# thunar 4.20.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.0.tar.bz2 |
| SHA-256 | `27731a76f3aecf3752b1ca35afad89e264c52244f70083d933507dd4a17548b0` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`exo`](../exo/) [`pango`](../pango/) [`gdk-pixbuf`](../gdk-pixbuf/) [`libnotify`](../libnotify/) [`libexif`](../libexif/) [`pcre2`](../pcre2/) [`xfce4-panel`](../xfce4-panel/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`exo`](../exo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libexif`](../libexif/) [`libnotify`](../libnotify/) [`libx11`](../libx11/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install thunar
oss-get install thunar-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild thunar` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (62 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel
```

<details><summary>Full build chain (62)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libexif`](../libexif/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libnotify`](../libnotify/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.0.tar.bz2
mkdir -p dist src/thunar
curl -fL -o dist/$(basename $URL) $URL
echo "27731a76f3aecf3752b1ca35afad89e264c52244f70083d933507dd4a17548b0  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/thunar --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/thunar && cd build/$QNX_ARCH/thunar
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/thunar/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-introspection \
    --disable-gudev \
    --disable-polkit \
    --enable-notifications \
    --enable-exif \
    --enable-pcre2
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Xfce file manager, and the thunarx extension library that its plugins (and tumbler's consumers) link against.  gudev is Linux's udev, which QNX does not have, so volume management is off; polkit likewise.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH thunar`.

### 5. What it installs

112 files under `/usr/local`: 2 shared-library entries, 3 programs, 14 headers.

```
/usr/local/lib/libthunarx-3.so
/usr/local/lib/libthunarx-3.so.0
/usr/local/bin/Thunar
/usr/local/bin/thunar
/usr/local/bin/thunar-settings
```

The complete list is `stage/<arch>/.manifest/thunar` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH thunar
ls repo/8.0/*/ | grep '^thunar-'
```

This writes `thunar-<ver>-r0.tar.gz` (run time) and `thunar-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install thunar
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/thunar-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/thunar-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
TARBALL=https://archive.xfce.org/src/xfce/thunar/4.20/thunar-4.20.0.tar.bz2
SHA256=27731a76f3aecf3752b1ca35afad89e264c52244f70083d933507dd4a17548b0
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf exo pango gdk-pixbuf libnotify libexif pcre2 xfce4-panel"
BUILDSYS=autotools
# The Xfce file manager, and the thunarx extension library that its plugins
# (and tumbler's consumers) link against.  gudev is Linux's udev, which QNX
# does not have, so volume management is off; polkit likewise.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-gudev --disable-polkit
                  --enable-notifications --enable-exif --enable-pcre2 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
