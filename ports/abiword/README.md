# abiword 3.0.8

> AbiWord word processor: .abw, OpenDocument, Word .docx/.doc, RTF (GTK 3)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/World/AbiWord/-/archive/release-3.0.8/AbiWord-release-3.0.8.tar.gz |
| SHA-256 | `7c490e46f0a6143f84d5c7e3fed246f34486ad70cd55f162c792827607cd7e9a` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`librsvg`](../librsvg/) [`libgsf`](../libgsf/) [`wv`](../wv/) [`libxslt`](../libxslt/) [`libxml2`](../libxml2/) [`fribidi`](../fribidi/) [`boost`](../boost/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`cairo`](../cairo/) [`pango`](../pango/) [`enchant`](../enchant/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`enchant`](../enchant/) [`fontconfig`](../fontconfig/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`goffice`](../goffice/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libgcrypt`](../libgcrypt/) [`libgsf`](../libgsf/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`librsvg`](../librsvg/) [`libx11`](../libx11/) [`libxml2`](../libxml2/) [`libxslt`](../libxslt/) [`pango`](../pango/) [`wv`](../wv/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_abiword`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `abiword` and `abiword-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild abiword` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (68 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 librsvg libgsf wv libxslt libxml2 fribidi boost libpng libjpeg-turbo cairo pango enchant libgcrypt libgpg-error
```

<details><summary>Full build chain (68)</summary>

[`aspell`](../aspell/) [`at-spi2-core`](../at-spi2-core/) [`boost`](../boost/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`enchant`](../enchant/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libgsf`](../libgsf/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`librsvg`](../librsvg/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxslt`](../libxslt/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`rust`](../rust/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`wv`](../wv/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://gitlab.gnome.org/World/AbiWord/-/archive/release-3.0.8/AbiWord-release-3.0.8.tar.gz
mkdir -p dist src/abiword
curl -fL -o dist/$(basename $URL) $URL
echo "7c490e46f0a6143f84d5c7e3fed246f34486ad70cd55f162c792827607cd7e9a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/abiword --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/abiword < ports/abiword/patches/0001-qnx-autogen-without-autoconf-archive.patch
patch -p1 -d src/abiword < ports/abiword/patches/0002-enchant-2.patch
```

**[`0001-qnx-autogen-without-autoconf-archive.patch`](patches/0001-qnx-autogen-without-autoconf-archive.patch)** — abiword: configure.ac without autoconf-archive and the old libgcrypt.m4.

> The release tag has no generated configure; autogen.sh runs autoreconf, and
> that failed ("configure.ac:70: error: possibly undefined macro:
> AC_MSG_ERROR"): the --with-sanitizer block uses AX_CHECK_COMPILE_FLAG and
> AX_APPEND_COMPILE_FLAGS from autoconf-archive, which the build machine does
> not have, and the unexpanded macro's AC_MSG_ERROR argument is what autoconf
> reported (at a wrong line). The block (a debugging option) now adds the
> flags without checking them first. AX_CXX_COMPILE_STDCXX_11 (also
> autoconf-archive, left unexpanded it broke configure: "syntax error near
> unexpected token `noext,mandatory'") is replaced by what it did, -std=gnu++11
> in CXXFLAGS: the code does not build as C++17, GCC 12's default (dynamic
> exception specifications, std::pointer_to_unary_function). The bundled libgcrypt.m4 is deleted as
> Debian does (drop-outdated-libgcrypt.m4.diff): an outdated copy of macros
> autoreconf otherwise takes over the installed ones.

Touches: `abib/configure.ac`, `abib/libgcrypt.m4`

**[`0002-enchant-2.patch`](patches/0002-enchant-2.patch)** — abiword: enchant-2 (Debian's enchant2.patch, unchanged below).

> AbiWord 3.0.8 looks for "enchant" (version 1); the enchant port is 2.x
> (enchant-2.pc, libenchant-2). Without this the spell checker is left out.
> The patch is Debian's (debian/patches/enchant2.patch of abiword
> 3.0.8+ds-2, by Laurent Bigonville, https://bugs.debian.org/956710): the
> pkg-config name and enchant 2's renamed functions.

Touches: `configure.ac`, `src/af/xap/xp/enchant_checker.cpp`

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/abiword && cd build/$QNX_ARCH/abiword
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" \
$OLDPWD/src/abiword/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-gtk2=no \
    --without-gnomevfs \
    --without-redland \
    --without-evolution-data-server \
    --without-libical \
    --without-champlain \
    --disable-introspection \
    --enable-plugins="opendocument openxml"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- abiword: the AbiWord word processor (GTK 3) -- documents in its own .abw, OpenDocument (.odt), Office Open XML (.docx), Word 97-2003 (.doc, through wv), RTF, HTML and text; spelling with enchant. The word processor of the desktop (Raspberry Pi OS has LibreOffice Writer in this place; AbiWord is the small one).
- GNOME GitLab is where AbiWord is developed now (abisource.com is gone); the release tag's archive has no generated configure: do_patch makes it (below).
- - the default plugins (OpenDocument, Office Open XML) built in the program; - no GNOME VFS, Redland (RDF), Evolution Data Server, libical, Champlain; - no introspection (gobject-introspection cannot run QNX programs here).
- The OpenDocument plugin reads encrypted documents with libgcrypt: configure takes libgcrypt-config from PATH, the build machine's ("-L/usr/lib/<host multiarch>" in LIBGCRYPT_LIBS, and the plugin linked the host's libc.so.6). gpgrt-config of the staged libgpg-error reads the staged libgcrypt.pc (as ports/gpgme).
- The release tag has no configure: autogen.sh's two steps, run here one by one so that a failure stops the build (autogen.sh goes on and returns 0, and a failed autoreconf then showed up later as "cannot find required auxiliary files: compile missing install-sh config.guess config.sub"). AM_PATH_LIBGCRYPT comes from the staged libgcrypt's libgcrypt.m4 (ACLOCAL_PATH): patches/0001 deletes AbiWord's outdated copy, as Debian does, and a clean build machine has no system one (libgcrypt20-dev).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH abiword`.

### 5. What it installs

294 files under `/usr/local`: 1 shared-library entries, 1 programs, 3 headers.

```
/usr/local/lib/libabiword-3.0.so
/usr/local/bin/abiword
```

The complete list is `stage/<arch>/.manifest/abiword` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa abiword
ls dists/8.0/*/ | grep '^abiword-'
```

This writes `abiword-<ver>-r0.tar.gz` (run time) and `abiword-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add abiword
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/abiword-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/abiword-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_abiword`](../../tests/src/) links abiword the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-abiword`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-abiword
/usr/local/bin/qnx-port-tests/test_abiword
```

```
== abiword ==
  ok   ...
PASS abiword
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_abiword` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_abiword`](../../tests/src/) -- a program linked against abiword the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> abiword`.

## Versions

This recipe builds **3.0.8**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add abiword=<version>`), sources in `ported/abiword/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates abiword`; port one with `scripts/port-update abiword`.

<details><summary>recipe.sh</summary>

```bash
# abiword: the AbiWord word processor (GTK 3) -- documents in its own .abw,
# OpenDocument (.odt), Office Open XML (.docx), Word 97-2003 (.doc, through
# wv), RTF, HTML and text; spelling with enchant. The word processor of the
# desktop (Raspberry Pi OS has LibreOffice Writer in this place; AbiWord is
# the small one).

VERSION=3.0.8
# GNOME GitLab is where AbiWord is developed now (abisource.com is gone); the
# release tag's archive has no generated configure: do_patch makes it (below).
TARBALL=https://gitlab.gnome.org/World/AbiWord/-/archive/release-3.0.8/AbiWord-release-3.0.8.tar.gz
SHA256=7c490e46f0a6143f84d5c7e3fed246f34486ad70cd55f162c792827607cd7e9a
DEPS="glib gtk3 librsvg libgsf wv libxslt libxml2 fribidi boost libpng libjpeg-turbo cairo pango enchant libgcrypt libgpg-error"
RUNTIME_DEPS="librsvg"
BUILDSYS=autotools
# - the default plugins (OpenDocument, Office Open XML) built in the program;
# - no GNOME VFS, Redland (RDF), Evolution Data Server, libical, Champlain;
# - no introspection (gobject-introspection cannot run QNX programs here).
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gtk2=no --without-gnomevfs
                  --without-redland --without-evolution-data-server --without-libical
                  --without-champlain --disable-introspection --enable-plugins="opendocument openxml" )
# The OpenDocument plugin reads encrypted documents with libgcrypt: configure
# takes libgcrypt-config from PATH, the build machine's ("-L/usr/lib/<host
# multiarch>" in LIBGCRYPT_LIBS, and the plugin linked the host's libc.so.6).
# gpgrt-config of the staged libgpg-error reads the staged libgcrypt.pc
# (as ports/gpgme).
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
# The release tag has no configure: autogen.sh's two steps, run here one by
# one so that a failure stops the build (autogen.sh goes on and returns 0,
# and a failed autoreconf then showed up later as "cannot find required
# auxiliary files: compile missing install-sh config.guess config.sub").
# AM_PATH_LIBGCRYPT comes from the staged libgcrypt's libgcrypt.m4
# (ACLOCAL_PATH): patches/0001 deletes AbiWord's outdated copy, as Debian
# does, and a clean build machine has no system one (libgcrypt20-dev).
do_patch() {
  cd "$SRCROOT" && ./autogen-common.sh && \
  ACLOCAL_PATH="$STAGE$PREFIX/share/aclocal${ACLOCAL_PATH:+:$ACLOCAL_PATH}" autoreconf --force --install || return 1
  for f in compile missing install-sh config.guess config.sub configure; do
    [ -f "$f" ] || { echo "autoreconf left no $f"; return 1; }
  done
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
