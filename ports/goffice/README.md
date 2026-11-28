# goffice 0.10.62

> GNOME office library: charts and graphs, canvas, number formats (Gnumeric's base)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/goffice/0.10/goffice-0.10.62.tar.xz |
| SHA-256 | `500eaff50628faa75adb3cd560236a4db498d9e7c52c036b78fa9a57f21805b3` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libgsf`](../libgsf/) [`libxml2`](../libxml2/) [`pango`](../pango/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`libxslt`](../libxslt/) [`gtk3`](../gtk3/) [`librsvg`](../librsvg/) [`intltool`](../intltool/) [`gettext`](../gettext/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libgsf`](../libgsf/) [`librsvg`](../librsvg/) [`libxml2`](../libxml2/) [`libxslt`](../libxslt/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_goffice`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `goffice` and `goffice-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild goffice` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (63 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libgsf libxml2 pango cairo gdk-pixbuf libxslt gtk3 librsvg intltool gettext
```

<details><summary>Full build chain (63)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`intltool`](../intltool/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgsf`](../libgsf/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`librsvg`](../librsvg/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxslt`](../libxslt/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`rust`](../rust/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/goffice/0.10/goffice-0.10.62.tar.xz
mkdir -p dist src/goffice
curl -fL -o dist/$(basename $URL) $URL
echo "500eaff50628faa75adb3cd560236a4db498d9e7c52c036b78fa9a57f21805b3  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/goffice --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/goffice; B=build/$QNX_ARCH/goffice; mkdir -p $B
```

```bash
do_configure() {
  _go_hostbin || return 1
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
```

```bash
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- goffice: the GNOME office library under Gnumeric -- charts and graphs (plot engines, the graph editor), the canvas, formats, number formatting, component plugins and the GUI helpers office programs share.
- - SVG through librsvg (pictures in charts and documents); no lasem (MathML and SVG equations: not ported); - settings in GSettings; no introspection (it cannot run QNX programs here).
- intltool (configure checks it; po/ is made with it): the staged programs are the target's Perl scripts (#!/usr/local/bin/perl); wrappers in the build directory run them with the build machine's perl, first on PATH for configure, make and the install -- as ports/xarchiver, whose comments say why setting INTLTOOL_UPDATE alone does not do. The cross environment is spelled out because a hook replaces the whole step.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH goffice`.

### 5. What it installs

265 files under `/usr/local`: 3 shared-library entries, 168 headers.

```
/usr/local/lib/libgoffice-0.10.so
/usr/local/lib/libgoffice-0.10.so.10
/usr/local/lib/libgoffice-0.10.so.10.0.62
```

The complete list is `stage/<arch>/.manifest/goffice` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa goffice
ls dists/8.0/*/ | grep '^goffice-'
```

This writes `goffice-<ver>-r0.tar.gz` (run time) and `goffice-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add goffice
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/goffice-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/goffice-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_goffice`](../../tests/src/) links goffice the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-goffice`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-goffice
/usr/local/bin/qnx-port-tests/test_goffice
```

```
== goffice ==
  ok   ...
PASS goffice
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_goffice` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_goffice`](../../tests/src/) -- a program linked against goffice the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> goffice`.

## Versions

This recipe builds **0.10.62**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add goffice=<version>`), sources in `ported/goffice/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates goffice`; port one with `scripts/port-update goffice`.

<details><summary>recipe.sh</summary>

```bash
# goffice: the GNOME office library under Gnumeric -- charts and graphs
# (plot engines, the graph editor), the canvas, formats, number formatting,
# component plugins and the GUI helpers office programs share.
VERSION=0.10.62
TARBALL=https://download.gnome.org/sources/goffice/0.10/goffice-0.10.62.tar.xz
SHA256=500eaff50628faa75adb3cd560236a4db498d9e7c52c036b78fa9a57f21805b3
DEPS="glib libgsf libxml2 pango cairo gdk-pixbuf libxslt gtk3 librsvg intltool gettext"
RUNTIME_DEPS="librsvg gsettings-desktop-schemas"
BUILDSYS=autotools
# - SVG through librsvg (pictures in charts and documents); no lasem (MathML
#   and SVG equations: not ported);
# - settings in GSettings; no introspection (it cannot run QNX programs here).
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gtk --with-librsvg --without-lasem
                  --with-config-backend=gsettings --disable-introspection )
# intltool (configure checks it; po/ is made with it): the staged programs are
# the target's Perl scripts (#!/usr/local/bin/perl); wrappers in the build
# directory run them with the build machine's perl, first on PATH for
# configure, make and the install -- as ports/xarchiver, whose comments say
# why setting INTLTOOL_UPDATE alone does not do. The cross environment is
# spelled out because a hook replaces the whole step.
_go_hostbin() {
  local t
  mkdir -p "$B/hostbin" || return 1
  for t in intltool-update intltool-merge intltool-extract; do
    printf '#!/bin/sh\nexec /usr/bin/perl "%s" "$@"\n' "$STAGE$PREFIX/bin/$t" > "$B/hostbin/$t" && chmod +x "$B/hostbin/$t" || return 1
  done
}
do_configure() {
  _go_hostbin || return 1
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)"; }
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
