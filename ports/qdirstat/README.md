# qdirstat 2.0

> Qt-based directory statistics: disk usage with a treemap

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/shundhammer/qdirstat.git (tag `2.0`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`qt6`](../qt6/) [`zlib`](../zlib/) [`gettext`](../gettext/) |
| Run-time dependencies | [`gettext`](../gettext/) [`qt6`](../qt6/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qdirstat`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `qdirstat` (programs, shared libraries, run-time data) and `qdirstat-dev` (headers, pkg-config/CMake files, static libraries; needs `qdirstat`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`qdirstat`](https://openqnx.github.io/oss/package/x86_64/qdirstat.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qdirstat.html)) · [`qdirstat-dev`](https://openqnx.github.io/oss/package/x86_64/qdirstat-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qdirstat-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add qdirstat` (dependencies come along); one version: `apk add qdirstat=<version>` | `apk del qdirstat` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install qdirstat` (dependencies first) | `oss-get remove qdirstat` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`qdirstat-dev` the same way, by its own name (`apk add qdirstat-dev`, `oss-get install qdirstat-dev`, by hand with its own file and list `qdirstat-dev.list`); remove it before `qdirstat`, which it needs.

```sh
# on the QNX target, as root
apk add qdirstat qdirstat-dev
apk add qnx-test-qdirstat && /usr/local/bin/qnx-port-tests/test_qdirstat    # its on-target test, from tests-8.0-<arch>-m-z
apk del qdirstat qdirstat-dev qnx-test-qdirstat
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/qdirstat.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/qdirstat.list
rm var/lib/manual/qdirstat.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qdirstat` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (65 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild qt6 zlib gettext
```

<details><summary>Full build chain (65)</summary>

[`aom`](../aom/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dav1d`](../dav1d/) [`double-conversion`](../double-conversion/) [`expat`](../expat/) [`fdk-aac`](../fdk-aac/) [`ffmpeg`](../ffmpeg/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-bad`](../gst-plugins-bad/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lame`](../lame/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libvpx`](../libvpx/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openh264`](../openh264/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`qt6`](../qt6/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`svt-av1`](../svt-av1/) [`x264`](../x264/) [`x265`](../x265/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.0 --recurse-submodules --shallow-submodules \
    https://github.com/shundhammer/qdirstat.git src/qdirstat
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/qdirstat < ports/qdirstat/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/qdirstat < ports/qdirstat/patches/0002-qnx-clipboard-mount-cachewriter.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — QNX changes from qnx-ports: directory entry names, mount points, sizes

> Imported from github.com/qnx-ports/qdirstat, branch john_qnx_support
> (commits 56294b5, b5491de, 8d261bb, 31e51ba; on top of master 952addb,
> applied here to the 2.0 release):
>
>  - DirReadJob: QNX declares struct dirent's d_name as char[1] (the name is
>    stored past the end); Qt 6's QString::fromUtf8() takes a char array by
>    reference with its compile-time size, so every name was read as its
>    first character. Pass a const char * instead.
>  - MountPoints: QNX has no /proc/mounts, and QStorageInfo::mountedVolumes()
>    returns an empty device for most volumes there: parse the output of
>    mount(1) instead. QStorageInfo's byte counts are wrong on QNX: take the
>    sizes from statvfs().
>  - Cleanup: on QNX, %terminal is xfce4-terminal.
>
> The rest of that branch is left out: a QRegExp compatibility shim for Qt
> builds without Qt5Compat (this tree's Qt 6 has Qt5Compat), a qmake mkspec
> for native QNX builds, and an apk package-manager backend (a new feature).

Touches: `src/Cleanup.cpp`, `src/DirReadJob.cpp`, `src/MountPoints.cpp`, `src/MountPoints.h` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-clipboard-mount-cachewriter.patch`](patches/0002-qnx-clipboard-mount-cachewriter.patch)** — No clipboard on QNX Qt, mount(1) lookup, cache writer without URI::Escape

> Three changes for QNX on top of qnx-ports' (0001):
>
>  - MainWindow: the ported Qt 6 is built without the clipboard feature on
>    QNX (QT_FEATURE_clipboard -1 in qtgui-config.h), so QClipboard is an
>    incomplete type and "Copy path to clipboard" did not compile. Without a
>    clipboard, show the path in the status bar instead.
>  - MountPoints: qnx-ports runs /usr/bin/mount, which QNX images do not
>    necessarily have (it is in /bin or /proc/boot); look in the usual
>    places, and fall back to QStorageInfo when no mount command is found
>    rather than leaving the mount point list empty.
>  - qdirstat-cache-writer: it needs URI::Escape, which is not part of core
>    Perl (this tree's perl does not have it). Use it when installed, else an
>    equivalent of uri_escape() for the explicit character class the script
>    always passes.

Touches: `scripts/qdirstat-cache-writer`, `src/MainWindow.cpp`, `src/MountPoints.cpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/qdirstat; B=build/$QNX_ARCH/qdirstat; mkdir -p $B
```

```bash
do_configure() {
  local spec; case "$QNX_ARCH" in aarch64le) spec=qnx-aarch64le-qcc ;; x86_64) spec=qnx-x86-64-qcc ;; esac
  cat > "$B/qt.conf" <<Q
[Paths]
Prefix=$STAGE$PREFIX
HostPrefix=$WS/hosttools/qt6-$QT_VER
HostBinaries=bin
HostLibraries=lib
HostLibraryExecutables=libexec
HostData=$STAGE$PREFIX
TargetSpec=$spec
HostSpec=linux-g++
Q
  rm -rf "$B/tree" && cp -a "$S" "$B/tree" && rm -rf "$B/tree/.git" || return 1
  cd "$B/tree" && "$WS/hosttools/qt6-$QT_VER/bin/qmake6" -qtconf "$B/qt.conf" qdirstat.pro \
    INSTALL_PREFIX="$PREFIX" "QMAKE_LFLAGS+=-Wl,-rpath-link,$STAGE$PREFIX/lib" \
    "CONFIG+=no_qt_rpath" "LIBS+=-lintl"
}
```

```bash
do_build() { make -C "$B/tree" -j"$(nproc)"; }
```

```bash
do_install() {
  make -C "$B/tree" install INSTALL_ROOT="$STAGE" && \
  sed -i '1s|^#!/usr/bin/perl|#!/usr/local/bin/perl|' "$STAGE$PREFIX/bin/qdirstat-cache-writer"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- QDirStat: Qt 6 graphical disk usage statistics (directory tree with sizes, treemap, file-type and file-age statistics, cleanup actions). qmake project, built with the host Qt's qmake (hosttools/qt6-<version>) against the ported Qt 6 for the target, mkspec qnx-<arch>-qcc. The staged bin/target_qt.conf names the host Qt by a path relative to where the workspace once was, so the build writes a qt.conf of its own with absolute paths. no_qt_rpath: qmake would give the program a run path to Qt's library directory, which this qt.conf names in the staging tree. -lintl: the translations go through gettext, whose staged <libintl.h> maps dgettext() to libintl's libintl_dgettext() (as qnx-ports links it). Built in a copy of the source ($B/tree): the .pro files install the program and the man pages from the source directory (an in-source build). QNX changes from qnx-ports (patches/0001).
- qdirstat-cache-writer (scripts/, Perl) is installed too, with the target's perl in its #! line (upstream: /usr/bin/perl, which QNX does not have).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qdirstat`.

### 5. What it installs

32 files under `/usr/local`, 2 programs.

```
/usr/local/bin/qdirstat
/usr/local/bin/qdirstat-cache-writer
```

The complete list is `stage/<arch>/.manifest/qdirstat` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qdirstat
ls dists/8.0/*/ | grep '^qdirstat-'
```

This writes `qdirstat-<ver>-r0.tar.gz` (run time) and `qdirstat-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qdirstat
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qdirstat-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qdirstat-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qdirstat`](../../tests/src/) links qdirstat the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qdirstat`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qdirstat
/usr/local/bin/qnx-port-tests/test_qdirstat
```

```
== qdirstat ==
  ok   ...
PASS qdirstat
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qdirstat` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qdirstat`](../../tests/src/) -- a program linked against qdirstat the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qdirstat 2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/qdirstat-2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qdirstat 2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/qdirstat-2.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qdirstat=<version>`), sources in `ported/qdirstat/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qdirstat`; port one with `scripts/port-update qdirstat`.

<details><summary>recipe.sh</summary>

```bash
# QDirStat: Qt 6 graphical disk usage statistics (directory tree with sizes,
# treemap, file-type and file-age statistics, cleanup actions).
# qmake project, built with the host Qt's qmake (hosttools/qt6-<version>)
# against the ported Qt 6 for the target, mkspec qnx-<arch>-qcc. The staged
# bin/target_qt.conf names the host Qt by a path relative to where the
# workspace once was, so the build writes a qt.conf of its own with
# absolute paths. no_qt_rpath: qmake would give the program a run path to
# Qt's library directory, which this qt.conf names in the staging tree.
# -lintl: the translations go through gettext, whose staged <libintl.h>
# maps dgettext() to libintl's libintl_dgettext() (as qnx-ports links it).
# Built in a copy of the source ($B/tree): the .pro files install the
# program and the man pages from the source directory (an in-source build).
# QNX changes from qnx-ports (patches/0001).
VERSION=2.0; GIT=https://github.com/shundhammer/qdirstat.git; REF=2.0
DEPS="qt6 zlib gettext"
BUILDSYS=custom
# qdirstat-cache-writer (scripts/, Perl) is installed too, with the target's
# perl in its #! line (upstream: /usr/bin/perl, which QNX does not have).
RUNTIME_DEPS="perl"
QT_VER=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/qt6/recipe.sh")
do_configure() {
  local spec; case "$QNX_ARCH" in aarch64le) spec=qnx-aarch64le-qcc ;; x86_64) spec=qnx-x86-64-qcc ;; esac
  cat > "$B/qt.conf" <<Q
[Paths]
Prefix=$STAGE$PREFIX
HostPrefix=$WS/hosttools/qt6-$QT_VER
HostBinaries=bin
HostLibraries=lib
HostLibraryExecutables=libexec
HostData=$STAGE$PREFIX
TargetSpec=$spec
HostSpec=linux-g++
Q
  rm -rf "$B/tree" && cp -a "$S" "$B/tree" && rm -rf "$B/tree/.git" || return 1
  cd "$B/tree" && "$WS/hosttools/qt6-$QT_VER/bin/qmake6" -qtconf "$B/qt.conf" qdirstat.pro \
    INSTALL_PREFIX="$PREFIX" "QMAKE_LFLAGS+=-Wl,-rpath-link,$STAGE$PREFIX/lib" \
    "CONFIG+=no_qt_rpath" "LIBS+=-lintl"
}
do_build() { make -C "$B/tree" -j"$(nproc)"; }
do_install() {
  make -C "$B/tree" install INSTALL_ROOT="$STAGE" && \
  sed -i '1s|^#!/usr/bin/perl|#!/usr/local/bin/perl|' "$STAGE$PREFIX/bin/qdirstat-cache-writer"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
