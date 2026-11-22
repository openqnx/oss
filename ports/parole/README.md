# parole 4.20.0

> Xfce media player (GTK 3, GStreamer)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/apps/parole/4.20/parole-4.20.0.tar.xz |
| SHA-256 | `5cf753e670d6518701133eb860d8bceb3a08a496af6a2b7cc67b93320230c983` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`dbus`](../dbus/) [`dbus-glib`](../dbus-glib/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`gstreamer`](../gstreamer/) [`gst-plugins-base`](../gst-plugins-base/) [`taglib`](../taglib/) [`libnotify`](../libnotify/) |
| Run-time dependencies | [`cairo`](../cairo/) [`dbus`](../dbus/) [`dbus-glib`](../dbus-glib/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk3`](../gtk3/) [`libnotify`](../libnotify/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`taglib`](../taglib/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_parole`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `parole` and `parole-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild parole` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gtk3 dbus dbus-glib libxfce4util libxfce4ui xfconf gstreamer gst-plugins-base taglib libnotify
```

<details><summary>Full build chain (71)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`dbus-glib`](../dbus-glib/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libnotify`](../libnotify/) [`libogg`](../libogg/) [`libpng`](../libpng/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`taglib`](../taglib/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/apps/parole/4.20/parole-4.20.0.tar.xz
mkdir -p dist src/parole
curl -fL -o dist/$(basename $URL) $URL
echo "5cf753e670d6518701133eb860d8bceb3a08a496af6a2b7cc67b93320230c983  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/parole --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/parole src/parole \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dx11=disabled \
      -Dwayland=enabled
ninja -C build/$QNX_ARCH/parole
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/parole install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Parole: the Xfce media player (GTK 3, GStreamer). Plays local files and network streams (HTTP, HLS/DASH through GStreamer) on the Wayland desktop. No DRM: Widevine-protected services (Netflix and the like) cannot play -- their decryption module is closed and not available for QNX.
- Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH parole`.

### 5. What it installs

84 files under `/usr/local`, 1 programs, 8 headers.

```
/usr/local/bin/parole
```

The complete list is `stage/<arch>/.manifest/parole` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa parole
ls dists/8.0/*/ | grep '^parole-'
```

This writes `parole-<ver>-r0.tar.gz` (run time) and `parole-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add parole
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/parole-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/parole-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_parole`](../../tests/src/) links parole the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-parole`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-parole
/usr/local/bin/qnx-port-tests/test_parole
```

```
== parole ==
  ok   ...
PASS parole
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_parole` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_parole`](../../tests/src/) -- a program linked against parole the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> parole`.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add parole=<version>`), sources in `ported/parole/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates parole`; port one with `scripts/port-update parole`.

<details><summary>recipe.sh</summary>

```bash
# Parole: the Xfce media player (GTK 3, GStreamer). Plays local files and
# network streams (HTTP, HLS/DASH through GStreamer) on the Wayland desktop.
# No DRM: Widevine-protected services (Netflix and the like) cannot play --
# their decryption module is closed and not available for QNX.
VERSION=4.20.0
TARBALL=https://archive.xfce.org/src/apps/parole/4.20/parole-4.20.0.tar.xz
SHA256=5cf753e670d6518701133eb860d8bceb3a08a496af6a2b7cc67b93320230c983
DEPS="glib gtk3 dbus dbus-glib libxfce4util libxfce4ui xfconf gstreamer gst-plugins-base taglib libnotify"
RUNTIME_DEPS="gst-plugins-good gst-plugins-bad gst-libav"
BUILDSYS=meson
# Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11).
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
