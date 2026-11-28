# xfce4-notifyd 0.9.7

> Xfce notification daemon (org.freedesktop.Notifications), log, settings and panel plugin

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/apps/xfce4-notifyd/0.9/xfce4-notifyd-0.9.7.tar.bz2 |
| SHA-256 | `170d18fd5f40cce823ffc7ae3d7e21644007c3f45808ab4835f0401d21b3516a` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`xfce4-panel`](../xfce4-panel/) [`libnotify`](../libnotify/) [`sqlite`](../sqlite/) [`gtk-layer-shell`](../gtk-layer-shell/) [`wayland`](../wayland/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`libnotify`](../libnotify/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`sqlite`](../sqlite/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfce4_notifyd`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xfce4-notifyd` and `xfce4-notifyd-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-notifyd` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel libnotify sqlite gtk-layer-shell wayland
```

<details><summary>Full build chain (67)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libnotify`](../libnotify/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/apps/xfce4-notifyd/0.9/xfce4-notifyd-0.9.7.tar.bz2
mkdir -p dist src/xfce4-notifyd
curl -fL -o dist/$(basename $URL) $URL
echo "170d18fd5f40cce823ffc7ae3d7e21644007c3f45808ab4835f0401d21b3516a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-notifyd --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfce4-notifyd src/xfce4-notifyd \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dx11=disabled \
      -Dwayland=enabled \
      -Dsound=disabled \
      -Dsystemd=disabled \
      -Ddbus-start-daemon=true \
      -Dxdg-autostart=true \
      -Dtests=false
ninja -C build/$QNX_ARCH/xfce4-notifyd
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfce4-notifyd install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-notifyd: the Xfce notification daemon -- the org.freedesktop.Notifications service that shows the pop-ups programs send with libnotify/notify-send, keeps a log of them (SQLite, ~/.cache/xfce4/notifyd/log.sqlite), a settings dialog (xfce4-notifyd-config) and a panel plugin with the recent notifications and a "do not disturb" switch. The session bus starts it on the first notification (share/dbus-1/services), and etc/xdg/autostart starts it with the session.
- Release tarball, as for every Xfce package here: it carries the generated sources a git checkout makes with xfce4-dev-tools.
- dbus: the daemon is a session-bus service; xfconf: its settings (xfconfd).
- - Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11): the pop-ups are wlr-layer-shell surfaces (gtk-layer-shell) on labwc; - no notification sounds: they need libcanberra, which is not ported; - no systemd user unit (QNX has no systemd): the D-Bus activation files and the XDG autostart entry start the daemon.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-notifyd`.

### 5. What it installs

81 files under `/usr/local`, 1 programs.

```
/usr/local/bin/xfce4-notifyd-config
```

The complete list is `stage/<arch>/.manifest/xfce4-notifyd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-notifyd
ls dists/8.0/*/ | grep '^xfce4-notifyd-'
```

This writes `xfce4-notifyd-<ver>-r0.tar.gz` (run time) and `xfce4-notifyd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-notifyd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-notifyd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-notifyd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfce4_notifyd`](../../tests/src/) links xfce4-notifyd the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfce4-notifyd`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfce4-notifyd
/usr/local/bin/qnx-port-tests/test_xfce4_notifyd
```

```
== xfce4-notifyd ==
  ok   ...
PASS xfce4-notifyd
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfce4_notifyd` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfce4_notifyd`](../../tests/src/) -- a program linked against xfce4-notifyd the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xfce4-notifyd`.

## Versions

This recipe builds **0.9.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-notifyd=<version>`), sources in `ported/xfce4-notifyd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-notifyd`; port one with `scripts/port-update xfce4-notifyd`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-notifyd: the Xfce notification daemon -- the org.freedesktop.Notifications
# service that shows the pop-ups programs send with libnotify/notify-send, keeps
# a log of them (SQLite, ~/.cache/xfce4/notifyd/log.sqlite), a settings dialog
# (xfce4-notifyd-config) and a panel plugin with the recent notifications and a
# "do not disturb" switch. The session bus starts it on the first notification
# (share/dbus-1/services), and etc/xdg/autostart starts it with the session.
VERSION=0.9.7
# Release tarball, as for every Xfce package here: it carries the generated
# sources a git checkout makes with xfce4-dev-tools.
TARBALL=https://archive.xfce.org/src/apps/xfce4-notifyd/0.9/xfce4-notifyd-0.9.7.tar.bz2
SHA256=170d18fd5f40cce823ffc7ae3d7e21644007c3f45808ab4835f0401d21b3516a
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel libnotify sqlite gtk-layer-shell wayland"
# dbus: the daemon is a session-bus service; xfconf: its settings (xfconfd).
RUNTIME_DEPS="dbus xfconf"
BUILDSYS=meson
# - Wayland only, as the rest of this Xfce (libxfce4windowing --disable-x11):
#   the pop-ups are wlr-layer-shell surfaces (gtk-layer-shell) on labwc;
# - no notification sounds: they need libcanberra, which is not ported;
# - no systemd user unit (QNX has no systemd): the D-Bus activation files and
#   the XDG autostart entry start the daemon.
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Dsound=disabled -Dsystemd=disabled
                   -Ddbus-start-daemon=true -Dxdg-autostart=true -Dtests=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
