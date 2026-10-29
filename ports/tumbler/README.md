# tumbler 4.20.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/tumbler/4.20/tumbler-4.20.0.tar.bz2 |
| SHA-256 | `74b1647d55926547e98bfac70838ff63c5a84299a5e10c81c38d1fab90e25880` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`dbus`](../dbus/) [`gdk-pixbuf`](../gdk-pixbuf/) [`libgsf`](../libgsf/) [`ffmpegthumbnailer`](../ffmpegthumbnailer/) [`libpng`](../libpng/) [`freetype`](../freetype/) [`libxfce4util`](../libxfce4util/) [`curl`](../curl/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) |
| Run-time dependencies | [`curl`](../curl/) [`ffmpegthumbnailer`](../ffmpegthumbnailer/) [`freetype`](../freetype/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`libgsf`](../libgsf/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libxfce4util`](../libxfce4util/) [`libxml2`](../libxml2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add tumbler
apk add tumbler-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install tumbler`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tumbler` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (40 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib dbus gdk-pixbuf libgsf ffmpegthumbnailer libpng freetype libxfce4util curl gst-plugins-base gstreamer
```

<details><summary>Full build chain (40)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`dav1d`](../dav1d/) [`dbus`](../dbus/) [`expat`](../expat/) [`ffmpeg`](../ffmpeg/) [`ffmpegthumbnailer`](../ffmpegthumbnailer/) [`freetype`](../freetype/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`lame`](../lame/) [`libffi`](../libffi/) [`libgsf`](../libgsf/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libxfce4util`](../libxfce4util/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`x264`](../x264/) [`x265`](../x265/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/tumbler/4.20/tumbler-4.20.0.tar.bz2
mkdir -p dist src/tumbler
curl -fL -o dist/$(basename $URL) $URL
echo "74b1647d55926547e98bfac70838ff63c5a84299a5e10c81c38d1fab90e25880  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tumbler --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/tumbler && cd build/$QNX_ARCH/tumbler
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/tumbler/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-libgepub \
    --disable-poppler \
    --enable-ffmpeg \
    --enable-gsf \
    --enable-pixbuf-thumbnailer \
    --enable-font-thumbnailer
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Release tarball, as with the rest of Xfce -- see README hazard 74.
- Also linked at run time (found by scripts/gen-dependency-index): curl gst-plugins-base gstreamer
- The Xfce thumbnailing service.  Its plugins are what make it worth porting: the ffmpeg thumbnailer and the GSF (OLE2/ZIP document) one both have their backing libraries staged already.  No poppler (PDF) or libgepub here -- not ported -- and no systemd login handling.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tumbler`.

### 5. What it installs

97 files under `/usr/local`: 3 shared-library entries, 16 headers.

```
/usr/local/lib/libtumbler-1.so
/usr/local/lib/libtumbler-1.so.0
/usr/local/lib/libtumbler-1.so.0.0.0
```

The complete list is `stage/<arch>/.manifest/tumbler` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tumbler
ls dists/8.0/*/ | grep '^tumbler-'
```

This writes `tumbler-<ver>-r0.tar.gz` (run time) and `tumbler-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tumbler
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tumbler-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tumbler-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tumbler=<version>`), sources in `ported/tumbler/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tumbler`; port one with `scripts/port-update tumbler`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
# Release tarball, as with the rest of Xfce -- see README hazard 74.
TARBALL=https://archive.xfce.org/src/xfce/tumbler/4.20/tumbler-4.20.0.tar.bz2
SHA256=74b1647d55926547e98bfac70838ff63c5a84299a5e10c81c38d1fab90e25880
# Also linked at run time (found by scripts/gen-dependency-index): curl gst-plugins-base gstreamer
DEPS="glib dbus gdk-pixbuf libgsf ffmpegthumbnailer libpng freetype libxfce4util curl gst-plugins-base gstreamer"
BUILDSYS=autotools
# The Xfce thumbnailing service.  Its plugins are what make it worth porting:
# the ffmpeg thumbnailer and the GSF (OLE2/ZIP document) one both have their
# backing libraries staged already.  No poppler (PDF) or libgepub here -- not
# ported -- and no systemd login handling.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-libgepub --disable-poppler
                  --enable-ffmpeg --enable-gsf --enable-pixbuf-thumbnailer
                  --enable-font-thumbnailer )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
