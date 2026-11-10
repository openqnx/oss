# qt6-webchannel 6.8.1

> Qt 6 WebChannel: expose QObjects to JavaScript clients (web pages, QML) over a transport

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwebchannel-everywhere-src-6.8.1.tar.xz |
| SHA-256 | `6b139f7f9128309e924da145023fa8ea87922bd79693392b3b2e4ba568a5763a` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`qt6`](../qt6/) |
| Run-time dependencies | [`qt6`](../qt6/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qt6_webchannel`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `qt6-webchannel` (programs, shared libraries, run-time data) and `qt6-webchannel-dev` (headers, pkg-config/CMake files, static libraries; needs `qt6-webchannel`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`qt6-webchannel`](https://openqnx.github.io/oss/package/x86_64/qt6-webchannel.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qt6-webchannel.html)) · [`qt6-webchannel-dev`](https://openqnx.github.io/oss/package/x86_64/qt6-webchannel-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qt6-webchannel-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add qt6-webchannel` (dependencies come along); one version: `apk add qt6-webchannel=<version>` | `apk del qt6-webchannel` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install qt6-webchannel` (dependencies first) | `oss-get remove qt6-webchannel` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`qt6-webchannel-dev` the same way, by its own name (`apk add qt6-webchannel-dev`, `oss-get install qt6-webchannel-dev`, by hand with its own file and list `qt6-webchannel-dev.list`); remove it before `qt6-webchannel`, which it needs.

```sh
# on the QNX target, as root
apk add qt6-webchannel qt6-webchannel-dev
apk add qnx-test-qt6-webchannel && /usr/local/bin/qnx-port-tests/test_qt6_webchannel    # its on-target test, from tests-8.0-<arch>-m-z
apk del qt6-webchannel qt6-webchannel-dev qnx-test-qt6-webchannel
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/qt6-webchannel.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/qt6-webchannel.list
rm var/lib/manual/qt6-webchannel.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qt6-webchannel` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (67 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild qt6
```

<details><summary>Full build chain (67)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`double-conversion`](../double-conversion/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`ffmpeg`](../ffmpeg/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lame`](../lame/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`qt6`](../qt6/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`svt-av1`](../svt-av1/) [`wayland`](../wayland/) [`x264`](../x264/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwebchannel-everywhere-src-6.8.1.tar.xz
mkdir -p dist src/qt6-webchannel
curl -fL -o dist/$(basename $URL) $URL
echo "6b139f7f9128309e924da145023fa8ea87922bd79693392b3b2e4ba568a5763a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/qt6-webchannel --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qt6-webchannel; B=build/$QNX_ARCH/qt6-webchannel; mkdir -p $B
```

```bash
do_configure() { qt6_module_configure; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Qt 6 qtwebchannel, the Qt version ports/qt6 builds, from Qt's own submodule tarball, cross-built against the staged qt6 (scripts/lib/qt6-module.sh).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qt6-webchannel`.

### 5. What it installs

68 files under `/usr/local`: 6 shared-library entries, 22 headers.

```
/usr/local/lib/libQt6WebChannel.so
/usr/local/lib/libQt6WebChannel.so.6
/usr/local/lib/libQt6WebChannel.so.6.8.1
/usr/local/lib/libQt6WebChannelQuick.so
/usr/local/lib/libQt6WebChannelQuick.so.6
/usr/local/lib/libQt6WebChannelQuick.so.6.8.1
```

The complete list is `stage/<arch>/.manifest/qt6-webchannel` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qt6-webchannel
ls dists/8.0/*/ | grep '^qt6-webchannel-'
```

This writes `qt6-webchannel-<ver>-r0.tar.gz` (run time) and `qt6-webchannel-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qt6-webchannel
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qt6-webchannel-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qt6-webchannel-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qt6_webchannel`](../../tests/src/) links qt6-webchannel the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qt6-webchannel`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qt6-webchannel
/usr/local/bin/qnx-port-tests/test_qt6_webchannel
```

```
== qt6-webchannel ==
  ok   ...
PASS qt6-webchannel
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qt6_webchannel` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qt6_webchannel`](../../tests/src/) -- a program linked against qt6-webchannel the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - init: thermo described
   - init: property celsius = 21
   - init: method toFahrenheit listed
   - invoke toFahrenheit(100) -> 212
   - set property celsius = 30 on the C++ object
   - property update (celsius = 30) sent to the idle client

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qt6-webchannel 6.8.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/qt6-webchannel-6.8.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qt6-webchannel 6.8.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/qt6-webchannel-6.8.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **6.8.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qt6-webchannel=<version>`), sources in `ported/qt6-webchannel/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qt6-webchannel`; port one with `scripts/port-update qt6-webchannel`.

<details><summary>recipe.sh</summary>

```bash
# Qt 6 qtwebchannel, the Qt version ports/qt6 builds, from Qt's own submodule tarball,
# cross-built against the staged qt6 (scripts/lib/qt6-module.sh).
VERSION=6.8.1
TARBALL=https://download.qt.io/archive/qt/6.8/6.8.1/submodules/qtwebchannel-everywhere-src-6.8.1.tar.xz
SHA256=6b139f7f9128309e924da145023fa8ea87922bd79693392b3b2e4ba568a5763a
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
