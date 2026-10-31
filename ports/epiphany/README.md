# epiphany 43.1

> GNOME Web, a WebKitGTK-based web browser

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/epiphany/43/epiphany-43.1.tar.xz |
| SHA-256 | `e86ead27cb9982815150664de3bf20faf375f77b8065b02b31180c65b6bbebb4` |
| Build system | Meson |
| QNX patches | 5 |
| Build dependencies | [`webkitgtk`](../webkitgtk/) [`gtk3`](../gtk3/) [`libhandy`](../libhandy/) [`libportal`](../libportal/) [`gcr`](../gcr/) [`libsecret`](../libsecret/) [`json-glib`](../json-glib/) [`sqlite`](../sqlite/) [`nettle`](../nettle/) [`libarchive`](../libarchive/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`iso-codes`](../iso-codes/) [`libdazzle`](../libdazzle/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gcr`](../gcr/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gmp`](../gmp/) [`gtk3`](../gtk3/) [`json-glib`](../json-glib/) [`libarchive`](../libarchive/) [`libdazzle`](../libdazzle/) [`libhandy`](../libhandy/) [`libportal`](../libportal/) [`libsecret`](../libsecret/) [`libsoup`](../libsoup/) [`libxml2`](../libxml2/) [`nettle`](../nettle/) [`pango`](../pango/) [`sqlite`](../sqlite/) [`webkitgtk`](../webkitgtk/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_epiphany`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add epiphany
apk add epiphany-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-epiphany && /usr/local/bin/qnx-port-tests/test_epiphany    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install epiphany`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild epiphany` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (100 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild webkitgtk gtk3 libhandy libportal gcr libsecret json-glib sqlite nettle libarchive gsettings-desktop-schemas iso-codes libdazzle
```

<details><summary>Full build chain (100)</summary>

[`aom`](../aom/) [`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`dbus`](../dbus/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gcr`](../gcr/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`gmp`](../gmp/) [`graphite2`](../graphite2/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`highway`](../highway/) [`hyphen`](../hyphen/) [`icu`](../icu/) [`iso-codes`](../iso-codes/) [`json-glib`](../json-glib/) [`lcms2`](../lcms2/) [`libarchive`](../libarchive/) [`libavif`](../libavif/) [`libdazzle`](../libdazzle/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libhandy`](../libhandy/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libjxl`](../libjxl/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libportal`](../libportal/) [`libpsl`](../libpsl/) [`libsecret`](../libsecret/) [`libsoup`](../libsoup/) [`libtasn1`](../libtasn1/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxslt`](../libxslt/) [`libxtst`](../libxtst/) [`libyuv`](../libyuv/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`nghttp2`](../nghttp2/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`sqlite`](../sqlite/) [`svt-av1`](../svt-av1/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`webkitgtk`](../webkitgtk/) [`woff2`](../woff2/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/epiphany/43/epiphany-43.1.tar.xz
mkdir -p dist src/epiphany
curl -fL -o dist/$(basename $URL) $URL
echo "e86ead27cb9982815150664de3bf20faf375f77b8065b02b31180c65b6bbebb4  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/epiphany --strip-components=1
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/epiphany < ports/epiphany/patches/0001-aports-001-fix-missing-function-getrandom.patch
patch -p1 -d src/epiphany < ports/epiphany/patches/0002-aports-002-fix-navigation-button-timeout-crash.patch
patch -p1 -d src/epiphany < ports/epiphany/patches/0003-aports-003-use-posix-spawn-for-fork-safety.patch
patch -p1 -d src/epiphany < ports/epiphany/patches/0004-help-only-with-itstool.patch
patch -p1 -d src/epiphany < ports/epiphany/patches/0005-qnx-locale-objects.patch
```

**[`0001-aports-001-fix-missing-function-getrandom.patch`](patches/0001-aports-001-fix-missing-function-getrandom.patch)** — QNX change from QNX's aports (extra/epiphany/001-fix-missing-function-getrandom.patch).

> Taken from qnx-ports/aports extra/epiphany -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> fix missing function getrandom
>
>
>  100.0% lib/

Touches: `lib/ephy-sync-utils.c` · Upstream status: QNX-specific

**[`0002-aports-002-fix-navigation-button-timeout-crash.patch`](patches/0002-aports-002-fix-navigation-button-timeout-crash.patch)** — QNX change from QNX's aports (extra/epiphany/002-fix-navigation-button-timeout-crash.patch).

> Taken from qnx-ports/aports extra/epiphany -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `src/ephy-action-bar-start.c` · Upstream status: QNX-specific

**[`0003-aports-003-use-posix-spawn-for-fork-safety.patch`](patches/0003-aports-003-use-posix-spawn-for-fork-safety.patch)** — QNX change from QNX's aports (extra/epiphany/003-use-posix-spawn-for-fork-safety.patch).

> Taken from qnx-ports/aports extra/epiphany -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `lib/ephy-file-helpers.c` · Upstream status: QNX-specific

**[`0004-help-only-with-itstool.patch`](patches/0004-help-only-with-itstool.patch)** — Build the help pages only when itstool is available.

> The user manual is translated with itstool at build time; without it on the
> build machine, configuration stops. Skip the help in that case.

Touches: `meson.build` · Upstream status: QNX-specific

**[`0005-qnx-locale-objects.patch`](patches/0005-qnx-locale-objects.patch)** — Emulate newlocale()/uselocale() on QNX with setlocale().

> QNX has no POSIX.1-2008 locale objects, so gnome-languages.c (copied from
> gnome-desktop) fails to compile: unknown type name locale_t, implicit
> declaration of newlocale. It uses them only to check a locale name and to
> switch locale around one translation lookup; a process-wide setlocale()
> does the same job there.

Touches: `lib/contrib/gnome-languages.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/epiphany src/epiphany \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dunit_tests=disabled \
      -Dtech_preview=false
ninja -C build/$QNX_ARCH/epiphany
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/epiphany install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNOME Web (Epiphany), the WebKitGTK browser. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libdazzle

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH epiphany`.

### 5. What it installs

126 files under `/usr/local`, 1 programs.

```
/usr/local/bin/epiphany
```

The complete list is `stage/<arch>/.manifest/epiphany` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa epiphany
ls dists/8.0/*/ | grep '^epiphany-'
```

This writes `epiphany-<ver>-r0.tar.gz` (run time) and `epiphany-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add epiphany
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/epiphany-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/epiphany-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_epiphany`](../../tests/src/) links epiphany the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-epiphany`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-epiphany
/usr/local/bin/qnx-port-tests/test_epiphany
```

```
== epiphany ==
  ok   ...
PASS epiphany
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_epiphany` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_epiphany`](../../tests/src/) -- a program linked against epiphany the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - epiphany --version: Web 43.1
   - epiphany --help lists its options
   - conflicting --private-instance/--application-mode rejected
   - --automation-mode with --incognito-mode rejected
   - --application-mode with a missing --profile directory rejected
   - unknown option rejected by name
   - ephy-profile-migrator: migration version 37
   - default bookmarks (RDF) installed
   - GSettings schema org.gnome.Epiphany.web installed
   - desktop entry runs epiphany

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | epiphany 43.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/epiphany-43.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | epiphany 43.1-r1 | ❌ FAIL | 2026-11-01 | [log](../../tests/logs/aarch64le/epiphany-43.1-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | epiphany 43.1-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/epiphany-43.1-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | epiphany 43.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/epiphany-43.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | epiphany 43.1-r1 | ❌ FAIL | 2026-11-01 | [log](../../tests/logs/x86_64/epiphany-43.1-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | epiphany 43.1-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/epiphany-43.1-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **43.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add epiphany=<version>`), sources in `ported/epiphany/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates epiphany`; port one with `scripts/port-update epiphany`.

<details><summary>recipe.sh</summary>

```bash
# GNOME Web (Epiphany), the WebKitGTK browser.
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=43.1
TARBALL=https://download.gnome.org/sources/epiphany/43/epiphany-43.1.tar.xz
SHA256=e86ead27cb9982815150664de3bf20faf375f77b8065b02b31180c65b6bbebb4
# Also linked at run time (found by scripts/gen-dependency-index): libdazzle
DEPS="webkitgtk gtk3 libhandy libportal gcr libsecret json-glib sqlite nettle libarchive gsettings-desktop-schemas iso-codes libdazzle"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dunit_tests=disabled -Dtech_preview=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
