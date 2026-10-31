# gst-plugins-base 1.24.10

> Essential GStreamer plugins and helper libraries (base set)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gstreamer`](../gstreamer/) [`orc`](../orc/) [`libogg`](../libogg/) [`libvorbis`](../libvorbis/) [`opus`](../opus/) [`libtheora`](../libtheora/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`gstreamer`](../gstreamer/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libvorbis`](../libvorbis/) [`opus`](../opus/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gst_plugins_base`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gst-plugins-base
apk add gst-plugins-base-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-gst-plugins-base && /usr/local/bin/qnx-port-tests/test_gst_plugins_base    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gst-plugins-base`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gst-plugins-base` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (17 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gstreamer orc libogg libvorbis opus libtheora zlib
```

<details><summary>Full build chain (17)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`gstreamer`](../gstreamer/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

The project lives in the `subprojects/gst-plugins-base/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gst-plugins-base src/gst-plugins-base/subprojects/gst-plugins-base \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Dexamples=disabled \
      -Dtools=enabled \
      -Dintrospection=disabled \
      -Ddoc=disabled \
      -Dnls=enabled \
      # \
      what \
      we \
      have \
      ported \
      -Dogg=enabled \
      -Dvorbis=enabled \
      -Dopus=enabled \
      -Dtheora=enabled \
      # \
      desktop/Linux-only \
      or \
      not \
      ported \
      yet \
      -Dalsa=disabled \
      -Dcdparanoia=disabled \
      -Dlibvisual=disabled \
      -Dpango=disabled \
      -Dtremor=disabled \
      -Dx11=disabled \
      -Dxvideo=disabled \
      -Dxshm=disabled \
      -Dgl=disabled \
      -Dgl-graphene=disabled
ninja -C build/$QNX_ARCH/gst-plugins-base
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gst-plugins-base install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gst-plugins-base`.

### 5. What it installs

320 files under `/usr/local`: 33 shared-library entries, 3 programs, 136 headers.

```
/usr/local/lib/libgstallocators-1.0.so
/usr/local/lib/libgstallocators-1.0.so.0
/usr/local/lib/libgstallocators-1.0.so.0.2410.0
/usr/local/lib/libgstapp-1.0.so
/usr/local/lib/libgstapp-1.0.so.0
/usr/local/lib/libgstapp-1.0.so.0.2410.0
/usr/local/lib/libgstaudio-1.0.so
/usr/local/lib/libgstaudio-1.0.so.0
/usr/local/lib/libgstaudio-1.0.so.0.2410.0
/usr/local/lib/libgstfft-1.0.so
/usr/local/lib/libgstfft-1.0.so.0
/usr/local/lib/libgstfft-1.0.so.0.2410.0
/usr/local/lib/libgstpbutils-1.0.so
/usr/local/lib/libgstpbutils-1.0.so.0
/usr/local/lib/libgstpbutils-1.0.so.0.2410.0
/usr/local/lib/libgstriff-1.0.so
...
```

The complete list is `stage/<arch>/.manifest/gst-plugins-base` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gst-plugins-base
ls dists/8.0/*/ | grep '^gst-plugins-base-'
```

This writes `gst-plugins-base-<ver>-r0.tar.gz` (run time) and `gst-plugins-base-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gst-plugins-base
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gst-plugins-base-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gst-plugins-base-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gst_plugins_base`](../../tests/src/) links gst-plugins-base the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gst-plugins-base`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gst-plugins-base
/usr/local/bin/qnx-port-tests/test_gst_plugins_base
```

```
== gst-plugins-base ==
  ok   ...
PASS gst-plugins-base
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gst_plugins_base` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gst_plugins_base`](../../tests/src/) -- a program linked against gst-plugins-base the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - all base elements used here are registered
   - video pipeline built
   - exactly num-buffers=10 frames arrive
   - negotiated RGB 64x48
   - frame size 64*3*48 = 9216 bytes
   - pattern=red converts to RGB red
   - appsink reached EOS
   - audio pipeline built
   - 8000 samples @ 8 kHz resample to ~16000 @ 16 kHz
   - volume 0.8 * 0.5 -> peak ~0.4
   - mono upmixed to identical left/right
   - scratch directory
   - encoded 1 s of audio to Ogg/Vorbis
   - file starts with OggS
   - decodebin (oggdemux + vorbisdec) returns ~44100 samples (1 s)
   - decode pipeline built
   - GstDiscoverer: duration 1.0 s
   - GstDiscoverer: one 44100 Hz audio stream
   - GstDiscoverer analysed the file
   - gst-discoverer-1.0 prints a 1 s duration
   - gst-discoverer-1.0 names the Vorbis stream
   - pbutils codec description

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gst-plugins-base 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gst-plugins-base-1.24.10-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gst-plugins-base 1.24.10-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gst-plugins-base-1.24.10-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gst-plugins-base 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gst-plugins-base-1.24.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gst-plugins-base 1.24.10-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gst-plugins-base-1.24.10-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gst-plugins-base=<version>`), sources in `ported/gst-plugins-base/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gst-plugins-base`; port one with `scripts/port-update gst-plugins-base`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"          # same monorepo as the core
SUBDIR=subprojects/gst-plugins-base
DEPS="gstreamer orc libogg libvorbis opus libtheora zlib"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Dtools=enabled
  -Dintrospection=disabled -Ddoc=disabled -Dnls=enabled
  # what we have ported
  -Dogg=enabled -Dvorbis=enabled -Dopus=enabled -Dtheora=enabled
  # desktop/Linux-only or not ported yet
  -Dalsa=disabled -Dcdparanoia=disabled -Dlibvisual=disabled
  -Dpango=disabled -Dtremor=disabled
  -Dx11=disabled -Dxvideo=disabled -Dxshm=disabled
  -Dgl=disabled -Dgl-graphene=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
