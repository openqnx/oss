# xfce4-screenshooter 1.11.3

> Xfce screenshot tool: screen, window or region, to a file or the clipboard; also a panel plugin

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/apps/xfce4-screenshooter/1.11/xfce4-screenshooter-1.11.3.tar.xz |
| SHA-256 | `1f6a14f7d1b0c440f31e24a8cc4fe2996185357fa786f0c2cdfe564ef673a710` |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`xfce4-panel`](../xfce4-panel/) [`exo`](../exo/) [`gtk-layer-shell`](../gtk-layer-shell/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) |
| Run-time dependencies | [`cairo`](../cairo/) [`exo`](../exo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`wayland`](../wayland/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfce4_screenshooter`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xfce4-screenshooter` and `xfce4-screenshooter-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-screenshooter` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (66 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel exo gtk-layer-shell wayland wayland-protocols
```

<details><summary>Full build chain (66)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/apps/xfce4-screenshooter/1.11/xfce4-screenshooter-1.11.3.tar.xz
mkdir -p dist src/xfce4-screenshooter
curl -fL -o dist/$(basename $URL) $URL
echo "1f6a14f7d1b0c440f31e24a8cc4fe2996185357fa786f0c2cdfe564ef673a710  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-screenshooter --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xfce4-screenshooter < ports/xfce4-screenshooter/patches/0001-qnx-delay-name-clash.patch
patch -p1 -d src/xfce4-screenshooter < ports/xfce4-screenshooter/patches/0002-man-page-only-when-the-program-runs-here.patch
```

**[`0001-qnx-delay-name-clash.patch`](patches/0001-qnx-delay-name-clash.patch)** — xfce4-screenshooter: rename the global "delay" in src/main.c.

> QNX's <unistd.h> declares delay() (sleep in milliseconds), and main.c, which
> includes it through GLib, defines a global int with the same name for the
> --delay option: "error: 'delay' redeclared as different kind of symbol".
> The variable is local to main.c; it is renamed opt_delay. Nothing else
> changes (lib/ only has the struct member sd->delay, which does not clash).

Touches: `src/main.c` · Upstream status: QNX-specific

**[`0002-man-page-only-when-the-program-runs-here.patch`](patches/0002-man-page-only-when-the-program-runs-here.patch)** — meson: no help2man man page when the program cannot run here

> The man page is made by running the freshly built xfce4-screenshooter
> --help under help2man. In a cross build that program is for the target
> (QNX) and does not run on the build machine: "help2man: can't get --help
> info from .../src/xfce4-screenshooter" stopped the build wherever help2man
> is installed. Make the man page only when meson can run host binaries; the
> release has no prebuilt one to install instead. --help itself is unchanged.

Touches: `src/meson.build` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfce4-screenshooter src/xfce4-screenshooter \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dx11=disabled \
      -Dwayland=enabled \
      -Dxfixes=disabled
ninja -C build/$QNX_ARCH/xfce4-screenshooter
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfce4-screenshooter install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-screenshooter: the Xfce screenshot tool -- the whole screen, the active window or a selected region, after a delay, with or without the pointer; save, copy to the clipboard or open with a program. Also a panel plugin. On Wayland it captures through wlr-screencopy (labwc has it, as the QNX screen share webrtc-screencast shows) and selects regions on a layer-shell surface.
- Wayland only, as the rest of this Xfce.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-screenshooter`.

### 5. What it installs

71 files under `/usr/local`, 1 programs.

```
/usr/local/bin/xfce4-screenshooter
```

The complete list is `stage/<arch>/.manifest/xfce4-screenshooter` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-screenshooter
ls dists/8.0/*/ | grep '^xfce4-screenshooter-'
```

This writes `xfce4-screenshooter-<ver>-r0.tar.gz` (run time) and `xfce4-screenshooter-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-screenshooter
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-screenshooter-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-screenshooter-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfce4_screenshooter`](../../tests/src/) links xfce4-screenshooter the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfce4-screenshooter`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfce4-screenshooter
/usr/local/bin/qnx-port-tests/test_xfce4_screenshooter
```

```
== xfce4-screenshooter ==
  ok   ...
PASS xfce4-screenshooter
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfce4_screenshooter` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfce4_screenshooter`](../../tests/src/) -- a program linked against xfce4-screenshooter the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xfce4-screenshooter`.

## Versions

This recipe builds **1.11.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-screenshooter=<version>`), sources in `ported/xfce4-screenshooter/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-screenshooter`; port one with `scripts/port-update xfce4-screenshooter`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-screenshooter: the Xfce screenshot tool -- the whole screen, the
# active window or a selected region, after a delay, with or without the
# pointer; save, copy to the clipboard or open with a program. Also a panel
# plugin. On Wayland it captures through wlr-screencopy (labwc has it, as the
# QNX screen share webrtc-screencast shows) and selects regions on a
# layer-shell surface.
VERSION=1.11.3
TARBALL=https://archive.xfce.org/src/apps/xfce4-screenshooter/1.11/xfce4-screenshooter-1.11.3.tar.xz
SHA256=1f6a14f7d1b0c440f31e24a8cc4fe2996185357fa786f0c2cdfe564ef673a710
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel exo gtk-layer-shell wayland wayland-protocols"
RUNTIME_DEPS="libxfce4ui xfconf"
BUILDSYS=meson
# Wayland only, as the rest of this Xfce.
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Dxfixes=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
