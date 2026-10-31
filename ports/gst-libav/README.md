# gst-libav 1.24.10

> GStreamer plugin for FFmpeg (libav) codecs and formats

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gst-plugins-base`](../gst-plugins-base/) [`ffmpeg`](../ffmpeg/) |
| Run-time dependencies | [`ffmpeg`](../ffmpeg/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gst_libav`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gst-libav
apk add gst-libav-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-gst-libav && /usr/local/bin/qnx-port-tests/test_gst_libav    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gst-libav`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gst-libav` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (25 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gst-plugins-base ffmpeg
```

<details><summary>Full build chain (25)</summary>

[`bzip2`](../bzip2/) [`dav1d`](../dav1d/) [`ffmpeg`](../ffmpeg/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`lame`](../lame/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`x264`](../x264/) [`x265`](../x265/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

The project lives in the `subprojects/gst-libav/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gst-libav src/gst-libav/subprojects/gst-libav \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddoc=disabled \
      -Dtests=disabled
ninja -C build/$QNX_ARCH/gst-libav
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gst-libav install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gst-libav`.

### 5. What it installs

3 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/gst-libav` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gst-libav
ls dists/8.0/*/ | grep '^gst-libav-'
```

This writes `gst-libav-<ver>-r0.tar.gz` (run time) and `gst-libav-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gst-libav
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gst-libav-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gst-libav-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gst_libav`](../../tests/src/) links gst-libav the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gst-libav`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gst-libav
/usr/local/bin/qnx-port-tests/test_gst_libav
```

```
== gst-libav ==
  ok   ...
PASS gst-libav
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gst_libav` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gst_libav`](../../tests/src/) -- a program linked against gst-libav the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - libav plugin registered
   - MPEG-4 Part 2 encoder and decoder
   - H.264 decoder, AAC encoder/decoder
   - a libavformat muxer element
   - 10 frames encoded, decoded, EOS reached
   - decoded caps 160x120
   - decoded picture is white (Y ~ 235)
   - AAC round trip keeps 44.1 kHz
   - about 44032 samples come back
   - decoded audio is a real signal, not silence

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gst-libav 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gst-libav-1.24.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gst-libav 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gst-libav-1.24.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gst-libav=<version>`), sources in `ported/gst-libav/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gst-libav`; port one with `scripts/port-update gst-libav`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-libav
DEPS="gst-plugins-base ffmpeg"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddoc=disabled -Dtests=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
