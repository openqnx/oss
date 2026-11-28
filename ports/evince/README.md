# evince 48.5

> GNOME document viewer for PDF, TIFF and comic book archives (GTK 3, poppler)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/evince/48/evince-48.5.tar.xz |
| SHA-256 | `0c7aba9485d0b6fb9f3e0ee75be916544744dc2ed9d58a2f9936a2e0434ac4a6` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`gdk-pixbuf`](../gdk-pixbuf/) [`cairo`](../cairo/) [`libhandy`](../libhandy/) [`libxml2`](../libxml2/) [`poppler`](../poppler/) [`libtiff`](../libtiff/) [`libarchive`](../libarchive/) [`libsecret`](../libsecret/) [`dbus`](../dbus/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`zlib`](../zlib/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`libarchive`](../libarchive/) [`libhandy`](../libhandy/) [`libsecret`](../libsecret/) [`libtiff`](../libtiff/) [`libxml2`](../libxml2/) [`pango`](../pango/) [`poppler`](../poppler/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_evince`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `evince` and `evince-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild evince` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (71 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 gdk-pixbuf cairo libhandy libxml2 poppler libtiff libarchive libsecret dbus gsettings-desktop-schemas zlib
```

<details><summary>Full build chain (71)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gsettings-desktop-schemas`](../gsettings-desktop-schemas/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libarchive`](../libarchive/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libhandy`](../libhandy/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openjpeg`](../openjpeg/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`poppler`](../poppler/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/evince/48/evince-48.5.tar.xz
mkdir -p dist src/evince
curl -fL -o dist/$(basename $URL) $URL
echo "0c7aba9485d0b6fb9f3e0ee75be916544744dc2ed9d58a2f9936a2e0434ac4a6  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/evince --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/evince src/evince \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dplatform=gnome \
      -Dviewer=true \
      -Dpreviewer=true \
      -Dthumbnailer=true \
      -Dnautilus=false \
      -Dpdf=enabled \
      -Dtiff=enabled \
      -Dcomics=enabled \
      -Ddjvu=disabled \
      -Dxps=disabled \
      -Ddvi=disabled \
      -Dps=disabled \
      -Ddbus=true \
      -Dkeyring=enabled \
      -Dgtk_unix_print=enabled \
      -Dthumbnail_cache=disabled \
      -Dmultimedia=disabled \
      -Dgspell=disabled \
      -Dinternal_synctex=true \
      -Dintrospection=false \
      -Dgtk_doc=false \
      -Duser_doc=false \
      -Dsystemduserunitdir=no
ninja -C build/$QNX_ARCH/evince
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/evince install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Evince: the GNOME document viewer (GTK 3) -- PDF through poppler, TIFF, and comic book archives (cbz/cbr/cb7/cbt through libarchive); search, outline and thumbnails side pane, annotations, forms, presentation mode, printing through the GTK print dialog. Also the evince-thumbnailer the file manager uses for document thumbnails, the print previewer, and libevdocument/libevview for programs that embed the viewer. 48 is the last GTK 3 series.
- libsecret: passwords of encrypted documents kept in the keyring; dbus: the build reads session_bus_services_dir from dbus-1.pc (evinced, the daemon that keeps one window per document); gsettings-desktop-schemas: the lockdown and interface settings, looked up when present.
- The org.gnome.desktop.* schemas g_settings_new() aborts without; the MIME database (a document is opened by its content type, and without shared-mime-info every file is application/octet-stream); the symbolic icons of the toolbar and side pane.
- - formats: PDF, TIFF and comics. No DjVu (djvulibre), XPS (libgxps), DVI (kpathsea) or PostScript (libspectre): those libraries are not ported; - thumbnail_cache: gnome-desktop (the recent-documents thumbnails) is not ported; multimedia: video inside PDFs through GStreamer, which the viewer embeds with an X11 window handle -- off on a Wayland desktop; gspell is not ported (no spell checking in annotations); - internal_synctex: the bundled SyncTeX parser (no TeX Live here); - no Nautilus extension, GObject introspection, API reference, or user manual (it is shown by yelp, which is not ported); - systemduserunitdir=no: QNX has no systemd; evinced is started by the session bus (share/dbus-1/services).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH evince`.

### 5. What it installs

225 files under `/usr/local`: 6 shared-library entries, 3 programs, 50 headers.

```
/usr/local/lib/libevdocument3.so
/usr/local/lib/libevdocument3.so.4
/usr/local/lib/libevdocument3.so.4.0.0
/usr/local/lib/libevview3.so
/usr/local/lib/libevview3.so.3
/usr/local/lib/libevview3.so.3.0.0
/usr/local/bin/evince
/usr/local/bin/evince-previewer
/usr/local/bin/evince-thumbnailer
```

The complete list is `stage/<arch>/.manifest/evince` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa evince
ls dists/8.0/*/ | grep '^evince-'
```

This writes `evince-<ver>-r0.tar.gz` (run time) and `evince-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add evince
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/evince-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/evince-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_evince`](../../tests/src/) links evince the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-evince`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-evince
/usr/local/bin/qnx-port-tests/test_evince
```

```
== evince ==
  ok   ...
PASS evince
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_evince` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_evince`](../../tests/src/) -- a program linked against evince the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> evince`.

## Versions

This recipe builds **48.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add evince=<version>`), sources in `ported/evince/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates evince`; port one with `scripts/port-update evince`.

<details><summary>recipe.sh</summary>

```bash
# Evince: the GNOME document viewer (GTK 3) -- PDF through poppler, TIFF, and
# comic book archives (cbz/cbr/cb7/cbt through libarchive); search, outline and
# thumbnails side pane, annotations, forms, presentation mode, printing through
# the GTK print dialog. Also the evince-thumbnailer the file manager uses for
# document thumbnails, the print previewer, and libevdocument/libevview for
# programs that embed the viewer. 48 is the last GTK 3 series.
VERSION=48.5
TARBALL=https://download.gnome.org/sources/evince/48/evince-48.5.tar.xz
SHA256=0c7aba9485d0b6fb9f3e0ee75be916544744dc2ed9d58a2f9936a2e0434ac4a6
# libsecret: passwords of encrypted documents kept in the keyring; dbus: the
# build reads session_bus_services_dir from dbus-1.pc (evinced, the daemon
# that keeps one window per document); gsettings-desktop-schemas: the
# lockdown and interface settings, looked up when present.
DEPS="glib gtk3 gdk-pixbuf cairo libhandy libxml2 poppler libtiff libarchive libsecret dbus gsettings-desktop-schemas zlib"
# The org.gnome.desktop.* schemas g_settings_new() aborts without; the MIME
# database (a document is opened by its content type, and without
# shared-mime-info every file is application/octet-stream); the symbolic icons
# of the toolbar and side pane.
RUNTIME_DEPS="gsettings-desktop-schemas shared-mime-info adwaita-icon-theme dbus"
BUILDSYS=meson
# - formats: PDF, TIFF and comics. No DjVu (djvulibre), XPS (libgxps), DVI
#   (kpathsea) or PostScript (libspectre): those libraries are not ported;
# - thumbnail_cache: gnome-desktop (the recent-documents thumbnails) is not
#   ported; multimedia: video inside PDFs through GStreamer, which the viewer
#   embeds with an X11 window handle -- off on a Wayland desktop; gspell is not
#   ported (no spell checking in annotations);
# - internal_synctex: the bundled SyncTeX parser (no TeX Live here);
# - no Nautilus extension, GObject introspection, API reference, or user
#   manual (it is shown by yelp, which is not ported);
# - systemduserunitdir=no: QNX has no systemd; evinced is started by the
#   session bus (share/dbus-1/services).
MESON_ARGS_EXTRA=( -Dplatform=gnome -Dviewer=true -Dpreviewer=true -Dthumbnailer=true -Dnautilus=false
                   -Dpdf=enabled -Dtiff=enabled -Dcomics=enabled
                   -Ddjvu=disabled -Dxps=disabled -Ddvi=disabled -Dps=disabled
                   -Ddbus=true -Dkeyring=enabled -Dgtk_unix_print=enabled
                   -Dthumbnail_cache=disabled -Dmultimedia=disabled -Dgspell=disabled
                   -Dinternal_synctex=true -Dintrospection=false -Dgtk_doc=false -Duser_doc=false
                   -Dsystemduserunitdir=no )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
