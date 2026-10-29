# gst-plugins-good 1.24.10

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gst-plugins-base`](../gst-plugins-base/) [`flac`](../flac/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libvpx`](../libvpx/) [`lame`](../lame/) [`zlib`](../zlib/) [`cairo`](../cairo/) [`libsoup`](../libsoup/) [`gdk-pixbuf`](../gdk-pixbuf/) [`bzip2`](../bzip2/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`flac`](../flac/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`lame`](../lame/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libsoup`](../libsoup/) [`libvpx`](../libvpx/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gst_plugins_good`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gst-plugins-good
oss-get install gst-plugins-good-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gst-plugins-good` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (47 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gst-plugins-base flac libpng libjpeg-turbo libvpx lame zlib cairo libsoup gdk-pixbuf bzip2
```

<details><summary>Full build chain (47)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`flac`](../flac/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`lame`](../lame/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libpsl`](../libpsl/) [`libsoup`](../libsoup/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`sqlite`](../sqlite/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

The project lives in the `subprojects/gst-plugins-good/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gst-plugins-good src/gst-plugins-good/subprojects/gst-plugins-good \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Dexamples=disabled \
      -Ddoc=disabled \
      -Dnls=enabled \
      # \
      ported \
      and \
      enabled \
      -Dflac=enabled \
      -Dpng=enabled \
      -Djpeg=enabled \
      -Dvpx=enabled \
      -Dlame=enabled \
      # \
      Linux-kernel \
      or \
      desktop-only \
      subsystems \
      QNX \
      does \
      not \
      have \
      -Dv4l2=disabled \
      -Doss=disabled \
      -Doss4=disabled \
      -Dpulse=disabled \
      -Djack=disabled \
      -Dximagesrc=disabled \
      -Dgtk3=disabled \
      -Dqt5=disabled \
      -Dqt6=disabled \
      -Dwaveform=disabled \
      -Ddirectsound=disabled \
      -Dosxaudio=disabled \
      -Dosxvideo=disabled \
      # \
      not \
      ported \
      (yet
ninja -C build/$QNX_ARCH/gst-plugins-good
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gst-plugins-good install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gst-plugins-good`.

### 5. What it installs

213 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/gst-plugins-good` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gst-plugins-good
ls repo/8.0/*/ | grep '^gst-plugins-good-'
```

This writes `gst-plugins-good-<ver>-r0.tar.gz` (run time) and `gst-plugins-good-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gst-plugins-good
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gst-plugins-good-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gst-plugins-good-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_gst_plugins_good`](../../tests/src/) links gst-plugins-good the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gst_plugins_good root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_gst_plugins_good'
```

```
== gst-plugins-good ==
  ok   ...
PASS gst-plugins-good
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-plugins-good
DEPS="gst-plugins-base flac libpng libjpeg-turbo libvpx lame zlib cairo libsoup gdk-pixbuf bzip2"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Ddoc=disabled -Dnls=enabled
  # ported and enabled
  -Dflac=enabled -Dpng=enabled -Djpeg=enabled -Dvpx=enabled -Dlame=enabled
  # Linux-kernel or desktop-only subsystems QNX does not have
  -Dv4l2=disabled -Doss=disabled -Doss4=disabled -Dpulse=disabled
  -Djack=disabled -Dximagesrc=disabled -Dgtk3=disabled -Dqt5=disabled -Dqt6=disabled
  -Dwaveform=disabled -Ddirectsound=disabled -Dosxaudio=disabled -Dosxvideo=disabled
  # not ported (yet)
  -Dcairo=enabled -Dsoup=enabled -Dwavpack=disabled
  -Dspeex=disabled -Dtaglib=disabled -Dtwolame=disabled
  -Dmpg123=disabled -Dshout2=disabled -Ddv=disabled -Ddv1394=disabled
  -Daalib=disabled -Dlibcaca=disabled -Dgdk-pixbuf=enabled -Dbz2=enabled
  -Dadaptivedemux2=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
