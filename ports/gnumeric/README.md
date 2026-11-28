# gnumeric 1.12.62

> GNOME spreadsheet: formulas, charts, Excel/OpenDocument/CSV files, ssconvert

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/gnumeric/1.12/gnumeric-1.12.62.tar.xz |
| SHA-256 | `89331121321b9bad72d37af5a6d13a0a636f0fbb0880f0ea1f2cf8b7ab9ae631` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`goffice`](../goffice/) [`libgsf`](../libgsf/) [`libxml2`](../libxml2/) [`pango`](../pango/) [`cairo`](../cairo/) [`zlib`](../zlib/) [`intltool`](../intltool/) [`gettext`](../gettext/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`goffice`](../goffice/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libgsf`](../libgsf/) [`libxml2`](../libxml2/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gnumeric`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `gnumeric` and `gnumeric-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gnumeric` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (64 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 goffice libgsf libxml2 pango cairo zlib intltool gettext
```

<details><summary>Full build chain (64)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`goffice`](../goffice/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`intltool`](../intltool/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgsf`](../libgsf/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`librsvg`](../librsvg/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxslt`](../libxslt/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`rust`](../rust/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/gnumeric/1.12/gnumeric-1.12.62.tar.xz
mkdir -p dist src/gnumeric
curl -fL -o dist/$(basename $URL) $URL
echo "89331121321b9bad72d37af5a6d13a0a636f0fbb0880f0ea1f2cf8b7ab9ae631  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gnumeric --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/gnumeric; B=build/$QNX_ARCH/gnumeric; mkdir -p $B
```

```bash
do_configure() {
  _gn_hostbin || return 1
  # ITSTOOL=/bin/true: itstool only translates the help pages and would have to
  # run on the build host (as ports/mate-calc); the English manual is installed.
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl ITSTOOL=/bin/true \
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
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)" "${_GN_MAKE[@]}"; }
```

```bash
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" "${_GN_MAKE[@]}" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- gnumeric: the GNOME spreadsheet -- workbooks with formulas (the function library, solver, goal seek), charts (goffice), and the formats of other spreadsheets: Excel (.xls, .xlsx), OpenDocument (.ods), CSV/text, Lotus, Quattro Pro and more; ssconvert converts between them from the command line.
- - no Perl or Python plugin loaders (scripting in the workbook), no GDA (databases), Guile, Psiconv (Psion), Paradox, Mono, PDF manual; - no introspection (it cannot run QNX programs here).
- intltool as in ports/goffice and ports/xarchiver: wrappers run the staged Perl scripts with the build machine's perl, first on PATH throughout.
- glib-compile-resources runs at build time: the build host's, not the staged QNX one that gio-2.0.pc names (as ports/xfce4-panel). configure.ac sets it from pkg-config whatever the environment says, so it is given to make. HELP_LINGUAS=: the translated help pages are made by itstool (not run, see ITSTOOL above); only the English manual is installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gnumeric`.

### 5. What it installs

955 files under `/usr/local`: 2 shared-library entries, 6 programs, 183 headers.

```
/usr/local/lib/libspreadsheet-1.12.62.so
/usr/local/lib/libspreadsheet.so
/usr/local/bin/gnumeric
/usr/local/bin/gnumeric-1.12.62
/usr/local/bin/ssconvert
/usr/local/bin/ssdiff
/usr/local/bin/ssgrep
/usr/local/bin/ssindex
```

The complete list is `stage/<arch>/.manifest/gnumeric` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gnumeric
ls dists/8.0/*/ | grep '^gnumeric-'
```

This writes `gnumeric-<ver>-r0.tar.gz` (run time) and `gnumeric-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gnumeric
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gnumeric-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gnumeric-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gnumeric`](../../tests/src/) links gnumeric the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gnumeric`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gnumeric
/usr/local/bin/qnx-port-tests/test_gnumeric
```

```
== gnumeric ==
  ok   ...
PASS gnumeric
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gnumeric` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gnumeric`](../../tests/src/) -- a program linked against gnumeric the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> gnumeric`.

## Versions

This recipe builds **1.12.62**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gnumeric=<version>`), sources in `ported/gnumeric/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gnumeric`; port one with `scripts/port-update gnumeric`.

<details><summary>recipe.sh</summary>

```bash
# gnumeric: the GNOME spreadsheet -- workbooks with formulas (the function
# library, solver, goal seek), charts (goffice), and the formats of other
# spreadsheets: Excel (.xls, .xlsx), OpenDocument (.ods), CSV/text, Lotus,
# Quattro Pro and more; ssconvert converts between them from the command line.
VERSION=1.12.62
TARBALL=https://download.gnome.org/sources/gnumeric/1.12/gnumeric-1.12.62.tar.xz
SHA256=89331121321b9bad72d37af5a6d13a0a636f0fbb0880f0ea1f2cf8b7ab9ae631
DEPS="glib gtk3 goffice libgsf libxml2 pango cairo zlib intltool gettext"
RUNTIME_DEPS="goffice gsettings-desktop-schemas"
BUILDSYS=autotools
# - no Perl or Python plugin loaders (scripting in the workbook), no GDA
#   (databases), Guile, Psiconv (Psion), Paradox, Mono, PDF manual;
# - no introspection (it cannot run QNX programs here).
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-perl --without-python --without-gda
                  --without-guile --without-psiconv --without-paradox --without-mono
                  --disable-pdfdocs --disable-introspection --disable-component )
# intltool as in ports/goffice and ports/xarchiver: wrappers run the staged
# Perl scripts with the build machine's perl, first on PATH throughout.
_gn_hostbin() {
  local t
  mkdir -p "$B/hostbin" || return 1
  for t in intltool-update intltool-merge intltool-extract; do
    printf '#!/bin/sh\nexec /usr/bin/perl "%s" "$@"\n' "$STAGE$PREFIX/bin/$t" > "$B/hostbin/$t" && chmod +x "$B/hostbin/$t" || return 1
  done
}
do_configure() {
  _gn_hostbin || return 1
  # ITSTOOL=/bin/true: itstool only translates the help pages and would have to
  # run on the build host (as ports/mate-calc); the English manual is installed.
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl ITSTOOL=/bin/true \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
# glib-compile-resources runs at build time: the build host's, not the staged QNX
# one that gio-2.0.pc names (as ports/xfce4-panel). configure.ac sets it from
# pkg-config whatever the environment says, so it is given to make.
# HELP_LINGUAS=: the translated help pages are made by itstool (not run, see
# ITSTOOL above); only the English manual is installed.
_GN_MAKE=( GLIB_COMPILE_RESOURCES=/usr/bin/glib-compile-resources HELP_LINGUAS= )
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)" "${_GN_MAKE[@]}"; }
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" "${_GN_MAKE[@]}" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
