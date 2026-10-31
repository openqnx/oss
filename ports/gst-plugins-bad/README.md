# gst-plugins-bad 1.24.10

> GStreamer plugins of lesser quality or maturity (bad set)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gst-plugins-base`](../gst-plugins-base/) [`x265`](../x265/) [`openh264`](../openh264/) [`aom`](../aom/) [`svt-av1`](../svt-av1/) [`fdk-aac`](../fdk-aac/) [`libwebp`](../libwebp/) [`openssl`](../openssl/) [`pango`](../pango/) [`cairo`](../cairo/) |
| Run-time dependencies | [`aom`](../aom/) [`cairo`](../cairo/) [`fdk-aac`](../fdk-aac/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`libwebp`](../libwebp/) [`openh264`](../openh264/) [`pango`](../pango/) [`svt-av1`](../svt-av1/) [`x265`](../x265/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gst_plugins_bad`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gst-plugins-bad` (programs, shared libraries, run-time data) and `gst-plugins-bad-dev` (headers, pkg-config/CMake files, static libraries; needs `gst-plugins-bad`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gst-plugins-bad`](https://openqnx.github.io/oss/package/x86_64/gst-plugins-bad.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gst-plugins-bad.html)) · [`gst-plugins-bad-dev`](https://openqnx.github.io/oss/package/x86_64/gst-plugins-bad-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gst-plugins-bad-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gst-plugins-bad` (dependencies come along); one version: `apk add gst-plugins-bad=<version>` | `apk del gst-plugins-bad` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gst-plugins-bad` (dependencies first) | `oss-get remove gst-plugins-bad` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gst-plugins-bad-dev` the same way, by its own name (`apk add gst-plugins-bad-dev`, `oss-get install gst-plugins-bad-dev`, by hand with its own file and list `gst-plugins-bad-dev.list`); remove it before `gst-plugins-bad`, which it needs.

```sh
# on the QNX target, as root
apk add gst-plugins-bad gst-plugins-bad-dev
apk add qnx-test-gst-plugins-bad && /usr/local/bin/qnx-port-tests/test_gst_plugins_bad    # its on-target test
apk del gst-plugins-bad gst-plugins-bad-dev qnx-test-gst-plugins-bad
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gst-plugins-bad.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gst-plugins-bad.list
rm var/lib/manual/gst-plugins-bad.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gst-plugins-bad
ls dists/8.0/*/ | grep '^gst-plugins-bad-'
```

This writes `gst-plugins-bad-<ver>-r0.tar.gz` (run time) and `gst-plugins-bad-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gst-plugins-bad
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gst-plugins-bad-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gst-plugins-bad-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gst_plugins_bad`](../../tests/src/) links gst-plugins-bad the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gst-plugins-bad`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gst-plugins-bad
/usr/local/bin/qnx-port-tests/test_gst_plugins_bad
```

```
== gst-plugins-bad ==
  ok   ...
PASS gst-plugins-bad
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gst_plugins_bad` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gst_plugins_bad`](../../tests/src/) -- a program linked against gst-plugins-bad the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - openh264 elements
   - videoparsersbad and MPEG-TS elements
   - freeverb, y4mdec, x265enc
   - 10 H.264 access units
   - codecparsers reads the SPS: 160x120
   - baseline/main/high profile
   - first access unit holds an IDR slice
   - first access unit captured
   - 10 frames through mpegtsmux/tsdemux/openh264dec
   - decoded 160x120
   - freeverb: mono in, stereo out

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gst-plugins-bad 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gst-plugins-bad-1.24.10-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gst-plugins-bad 1.24.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gst-plugins-bad-1.24.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gst-plugins-bad 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gst-plugins-bad-1.24.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gst-plugins-bad 1.24.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gst-plugins-bad-1.24.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gst-plugins-bad=<version>`), sources in `ported/gst-plugins-bad/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gst-plugins-bad`; port one with `scripts/port-update gst-plugins-bad`.

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
