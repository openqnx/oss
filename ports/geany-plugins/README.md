# geany-plugins 2.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://plugins.geany.org/geany-plugins/geany-plugins-2.1.tar.bz2 |
| SHA-256 | `813f24570ca7590a9ae6e8e9ca4e5bd6c9f09d61fe36ad6feca89b7c2feaae70` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`geany`](../geany/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`geany`](../geany/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install geany-plugins
oss-get install geany-plugins-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild geany-plugins` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (53 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild geany
```

<details><summary>Full build chain (53)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`geany`](../geany/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://plugins.geany.org/geany-plugins/geany-plugins-2.1.tar.bz2
mkdir -p dist src/geany-plugins
curl -fL -o dist/$(basename $URL) $URL
echo "813f24570ca7590a9ae6e8e9ca4e5bd6c9f09d61fe36ad6feca89b7c2feaae70  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/geany-plugins --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/geany-plugins < ports/geany-plugins/patches/0001-aports-fix-qnx.patch
```

**[`0001-aports-fix-qnx.patch`](patches/0001-aports-fix-qnx.patch)** — QNX change from QNX's aports (extra/geany-plugins/fix-qnx.patch).

> Taken from qnx-ports/aports extra/geany-plugins -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `geany-plugins-2.1/build//scope.m4`, `geany-plugins-2.1/scope/src/conterm.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/geany-plugins && cd build/$QNX_ARCH/geany-plugins
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/geany-plugins/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-all-plugins \
    --enable-addons \
    --enable-autoclose \
    --enable-automark \
    --enable-codenav \
    --enable-commander \
    --enable-defineformat \
    --enable-filebrowser \
    --enable-keyrecord \
    --enable-lineoperations \
    --enable-overview \
    --enable-pairtaghighlighter \
    --enable-projectorganizer \
    --enable-sendmail \
    --enable-shiftcolumn \
    --enable-treebrowser \
    --enable-xmlsnippets
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Geany plugin collection (the plugins without extra library dependencies). Version as in the QNX Everywhere 8.0.4r1 image.
- The QNX patch touches the autotools inputs; regenerate the build files.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH geany-plugins`.

### 5. What it installs

116 files under `/usr/local`: 2 shared-library entries.

```
/usr/local/lib/libgeanypluginutils.so
/usr/local/lib/libgeanypluginutils.so.0
```

The complete list is `stage/<arch>/.manifest/geany-plugins` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH geany-plugins
ls repo/8.0/*/ | grep '^geany-plugins-'
```

This writes `geany-plugins-<ver>-r0.tar.gz` (run time) and `geany-plugins-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install geany-plugins
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/geany-plugins-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/geany-plugins-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# The Geany plugin collection (the plugins without extra library dependencies).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.1
TARBALL=https://plugins.geany.org/geany-plugins/geany-plugins-2.1.tar.bz2
SHA256=813f24570ca7590a9ae6e8e9ca4e5bd6c9f09d61fe36ad6feca89b7c2feaae70
DEPS="geany"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-all-plugins --enable-addons --enable-autoclose --enable-automark --enable-codenav --enable-commander --enable-defineformat --enable-filebrowser --enable-keyrecord --enable-lineoperations --enable-overview --enable-pairtaghighlighter --enable-projectorganizer --enable-sendmail --enable-shiftcolumn --enable-treebrowser --enable-xmlsnippets )
# The QNX patch touches the autotools inputs; regenerate the build files.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
