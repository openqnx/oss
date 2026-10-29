# gst-plugins-bad 1.24.10

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gst-plugins-base`](../gst-plugins-base/) [`x265`](../x265/) [`openh264`](../openh264/) [`aom`](../aom/) [`svt-av1`](../svt-av1/) [`fdk-aac`](../fdk-aac/) [`libwebp`](../libwebp/) [`openssl`](../openssl/) [`pango`](../pango/) [`cairo`](../cairo/) |
| Run-time dependencies | [`aom`](../aom/) [`cairo`](../cairo/) [`fdk-aac`](../fdk-aac/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`libwebp`](../libwebp/) [`openh264`](../openh264/) [`pango`](../pango/) [`svt-av1`](../svt-av1/) [`x265`](../x265/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gst-plugins-bad
oss-get install gst-plugins-bad-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gst-plugins-bad` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (51 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gst-plugins-base x265 openh264 aom svt-av1 fdk-aac libwebp openssl pango cairo
```

<details><summary>Full build chain (51)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`svt-av1`](../svt-av1/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

The project lives in the `subprojects/gst-plugins-bad/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gst-plugins-bad src/gst-plugins-bad/subprojects/gst-plugins-bad \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Dexamples=disabled \
      -Ddoc=disabled \
      -Dnls=enabled \
      -Dauto_features=disabled \
      -Dgpl=enabled \
      # \
      codecs \
      we \
      built \
      from \
      source \
      in \
      this \
      workspace \
      -Dx265=enabled \
      -Dopenh264=enabled \
      -Daom=enabled \
      -Dsvtav1=enabled \
      -Dfdkaac=enabled \
      -Dwebp=enabled \
      # \
      pure-C \
      element \
      sets \
      with \
      no \
      external \
      dependency \
      -Dvideoparsers=enabled \
      -Dmpegtsdemux=enabled \
      -Dmpegtsmux=enabled \
      -Dmpegpsmux=enabled \
      -Dclosedcaption=enabled \
      -Dcodecalpha=enabled \
      -Dcodectimestamper=enabled \
      -Dinterlace=enabled \
      -Divfparse=enabled \
      -Divtc=enabled \
      -Djpegformat=enabled \
      -Dmxf=enabled \
      -Dpcapparse=enabled \
      -Dsdp=enabled \
      -Dsegmentclip=enabled \
      -Dsiren=enabled \
      -Dspeed=enabled \
      -Dsubenc=enabled \
      -Dtimecode=enabled \
      -Dvideofilters=enabled \
      -Dvideoframe_audiolevel=enabled \
      -Dy4m=enabled \
      -Daudiobuffersplit=enabled \
      -Daudiolatency=enabled \
      -Daudiomixmatrix=enabled \
      -Dautoconvert=enabled \
      -Dbayer=enabled \
      -Dcamerabin2=enabled \
      -Ddebugutils=enabled \
      -Dfieldanalysis=enabled \
      -Dfreeverb=enabled \
      -Dgaudieffects=enabled \
      -Dgeometrictransform=enabled \
      -Dinter=enabled \
      -Dproxy=enabled \
      -Drawparse=enabled \
      -Dremovesilence=enabled \
      -Dsmooth=enabled \
      -Dspeed=enabled \
      -Dvideosignal=enabled \
      -Dwebrtcdsp=disabled
ninja -C build/$QNX_ARCH/gst-plugins-bad
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gst-plugins-bad install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- This plugin set probes for well over a hundred optional dependencies, most of them desktop-Linux only. Turn the auto-detection off and name what we ported.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gst-plugins-bad`.

### 5. What it installs

366 files under `/usr/local`: 50 shared-library entries, 95 headers.

```
/usr/local/lib/libgstadaptivedemux-1.0.so
/usr/local/lib/libgstadaptivedemux-1.0.so.0
/usr/local/lib/libgstadaptivedemux-1.0.so.0.2410.0
/usr/local/lib/libgstanalytics-1.0.so
/usr/local/lib/libgstanalytics-1.0.so.0
/usr/local/lib/libgstanalytics-1.0.so.0.2410.0
/usr/local/lib/libgstbadaudio-1.0.so
/usr/local/lib/libgstbadaudio-1.0.so.0
/usr/local/lib/libgstbadaudio-1.0.so.0.2410.0
/usr/local/lib/libgstbasecamerabinsrc-1.0.so
/usr/local/lib/libgstbasecamerabinsrc-1.0.so.0
/usr/local/lib/libgstbasecamerabinsrc-1.0.so.0.2410.0
/usr/local/lib/libgstcodecparsers-1.0.so
/usr/local/lib/libgstcodecparsers-1.0.so.0
/usr/local/lib/libgstcodecparsers-1.0.so.0.2410.0
/usr/local/lib/libgstcodecs-1.0.so
...
```

The complete list is `stage/<arch>/.manifest/gst-plugins-bad` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gst-plugins-bad
ls repo/8.0/*/ | grep '^gst-plugins-bad-'
```

This writes `gst-plugins-bad-<ver>-r0.tar.gz` (run time) and `gst-plugins-bad-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gst-plugins-bad
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gst-plugins-bad-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gst-plugins-bad-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-plugins-bad
DEPS="gst-plugins-base x265 openh264 aom svt-av1 fdk-aac libwebp openssl pango cairo"
BUILDSYS=meson
# This plugin set probes for well over a hundred optional dependencies, most of
# them desktop-Linux only. Turn the auto-detection off and name what we ported.
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Ddoc=disabled -Dnls=enabled
  -Dauto_features=disabled
  -Dgpl=enabled
  # codecs we built from source in this workspace
  -Dx265=enabled -Dopenh264=enabled -Daom=enabled
  -Dsvtav1=enabled -Dfdkaac=enabled -Dwebp=enabled
  # pure-C element sets with no external dependency
  -Dvideoparsers=enabled -Dmpegtsdemux=enabled -Dmpegtsmux=enabled
  -Dmpegpsmux=enabled -Dclosedcaption=enabled -Dcodecalpha=enabled
  -Dcodectimestamper=enabled -Dinterlace=enabled -Divfparse=enabled -Divtc=enabled
  -Djpegformat=enabled -Dmxf=enabled -Dpcapparse=enabled -Dsdp=enabled
  -Dsegmentclip=enabled -Dsiren=enabled -Dspeed=enabled -Dsubenc=enabled
  -Dtimecode=enabled -Dvideofilters=enabled -Dvideoframe_audiolevel=enabled
  -Dy4m=enabled -Daudiobuffersplit=enabled -Daudiolatency=enabled
  -Daudiomixmatrix=enabled -Dautoconvert=enabled -Dbayer=enabled
  -Dcamerabin2=enabled -Ddebugutils=enabled -Dfieldanalysis=enabled
  -Dfreeverb=enabled -Dgaudieffects=enabled -Dgeometrictransform=enabled
  -Dinter=enabled -Dproxy=enabled -Drawparse=enabled -Dremovesilence=enabled
  -Dsmooth=enabled -Dspeed=enabled -Dvideosignal=enabled -Dwebrtcdsp=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
