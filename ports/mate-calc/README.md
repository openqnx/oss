# mate-calc 1.28.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://pub.mate-desktop.org/releases/1.28/mate-calc-1.28.0.tar.xz |
| SHA-256 | `804b125d1e2864b1e74af816da9b2ab8b19472b9af974437ee7355ada5e628f5` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxml2`](../libxml2/) [`gmp`](../gmp/) [`mpfr`](../mpfr/) [`mpc`](../mpc/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gmp`](../gmp/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libxml2`](../libxml2/) [`mpc`](../mpc/) [`mpfr`](../mpfr/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install mate-calc
oss-get install mate-calc-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mate-calc` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (55 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxml2 gmp mpfr mpc
```

<details><summary>Full build chain (55)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gmp`](../gmp/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`mpc`](../mpc/) [`mpfr`](../mpfr/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://pub.mate-desktop.org/releases/1.28/mate-calc-1.28.0.tar.xz
mkdir -p dist src/mate-calc
curl -fL -o dist/$(basename $URL) $URL
echo "804b125d1e2864b1e74af816da9b2ab8b19472b9af974437ee7355ada5e628f5  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mate-calc --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/mate-calc < ports/mate-calc/patches/0001-aports-fix-angle-parsing-qnx.patch
patch -p1 -d src/mate-calc < ports/mate-calc/patches/0002-aports-fix-radix-qnx.patch
patch -p1 -d src/mate-calc < ports/mate-calc/patches/0003-aports-fix-test-mp-equation-qnx.patch
```

**[`0001-aports-fix-angle-parsing-qnx.patch`](patches/0001-aports-fix-angle-parsing-qnx.patch)** — QNX change from QNX's aports (extra/mate-calc/fix-angle-parsing-qnx.patch).

> Taken from qnx-ports/aports extra/mate-calc -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/mp-convert.c` · Upstream status: QNX-specific

**[`0002-aports-fix-radix-qnx.patch`](patches/0002-aports-fix-radix-qnx.patch)** — QNX change from QNX's aports (extra/mate-calc/fix-radix-qnx.patch).

> Taken from qnx-ports/aports extra/mate-calc -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/mp-serializer.c` · Upstream status: QNX-specific

**[`0003-aports-fix-test-mp-equation-qnx.patch`](patches/0003-aports-fix-test-mp-equation-qnx.patch)** — QNX change from QNX's aports (extra/mate-calc/fix-test-mp-equation-qnx.patch).

> Taken from qnx-ports/aports extra/mate-calc -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/test-mp-equation.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/mate-calc && cd build/$QNX_ARCH/mate-calc
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
ITSTOOL=/bin/true \
$OLDPWD/src/mate-calc/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- MATE Calculator (GTK3). Version as in the QNX Everywhere 8.0.4r1 image.
- itstool only builds the help pages (it would have to run on the build host).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mate-calc`.

### 5. What it installs

36 files under `/usr/local`, 3 programs.

```
/usr/local/bin/mate-calc
/usr/local/bin/mate-calc-cmd
/usr/local/bin/mate-calculator
```

The complete list is `stage/<arch>/.manifest/mate-calc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH mate-calc
ls repo/8.0/*/ | grep '^mate-calc-'
```

This writes `mate-calc-<ver>-r0.tar.gz` (run time) and `mate-calc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install mate-calc
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/mate-calc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/mate-calc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# MATE Calculator (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.28.0
TARBALL=https://pub.mate-desktop.org/releases/1.28/mate-calc-1.28.0.tar.xz
SHA256=804b125d1e2864b1e74af816da9b2ab8b19472b9af974437ee7355ada5e628f5
DEPS="glib gtk3 libxml2 gmp mpfr mpc"
BUILDSYS=autotools
# itstool only builds the help pages (it would have to run on the build host).
CONF_ENV=( ITSTOOL=/bin/true )
CONF_ARGS_EXTRA=( --disable-nls )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
