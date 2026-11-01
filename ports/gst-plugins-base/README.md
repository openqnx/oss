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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gst-plugins-base` (programs, shared libraries, run-time data) and `gst-plugins-base-dev` (headers, pkg-config/CMake files, static libraries; needs `gst-plugins-base`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gst-plugins-base`](https://openqnx.github.io/oss/package/x86_64/gst-plugins-base.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gst-plugins-base.html)) · [`gst-plugins-base-dev`](https://openqnx.github.io/oss/package/x86_64/gst-plugins-base-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gst-plugins-base-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gst-plugins-base` (dependencies come along); one version: `apk add gst-plugins-base=<version>` | `apk del gst-plugins-base` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gst-plugins-base` (dependencies first) | `oss-get remove gst-plugins-base` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gst-plugins-base-dev` the same way, by its own name (`apk add gst-plugins-base-dev`, `oss-get install gst-plugins-base-dev`, by hand with its own file and list `gst-plugins-base-dev.list`); remove it before `gst-plugins-base`, which it needs.

```sh
# on the QNX target, as root
apk add gst-plugins-base gst-plugins-base-dev
apk add qnx-test-gst-plugins-base && /usr/local/bin/qnx-port-tests/test_gst_plugins_base    # its on-target test, from tests-8.0-<arch>
apk del gst-plugins-base gst-plugins-base-dev qnx-test-gst-plugins-base
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gst-plugins-base.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gst-plugins-base.list
rm var/lib/manual/gst-plugins-base.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

**Build the dependencies first** (24 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gstreamer orc libogg libvorbis opus libtheora zlib
```

<details><summary>Full build chain (24)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`gstreamer`](../gstreamer/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
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

[`test_gst_plugins_base`](../../tests/src/) links gst-plugins-base the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gst-plugins-base`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

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
