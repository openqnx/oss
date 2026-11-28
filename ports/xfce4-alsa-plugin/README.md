# xfce4-alsa-plugin 0.4.0

> Xfce panel plugin: ALSA volume control (scroll to change, click for a slider, mute)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/equeim/xfce4-alsa-plugin/archive/refs/tags/0.4.0.tar.gz |
| SHA-256 | `16971c3c7d6324238db98c210da17755f27bc5baa080feae22f11005164ac3e2` |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`gtk3`](../gtk3/) [`xfce4-panel`](../xfce4-panel/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`xfce4-panel`](../xfce4-panel/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfce4_alsa_plugin`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xfce4-alsa-plugin` and `xfce4-alsa-plugin-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-alsa-plugin` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild gtk3 xfce4-panel
```

<details><summary>Full build chain (66)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfce4-panel`](../xfce4-panel/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://github.com/equeim/xfce4-alsa-plugin/archive/refs/tags/0.4.0.tar.gz
mkdir -p dist src/xfce4-alsa-plugin
curl -fL -o dist/$(basename $URL) $URL
echo "16971c3c7d6324238db98c210da17755f27bc5baa080feae22f11005164ac3e2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-alsa-plugin --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xfce4-alsa-plugin < ports/xfce4-alsa-plugin/patches/0001-meson-qnx-libasound-without-alsa-pc.patch
patch -p1 -d src/xfce4-alsa-plugin < ports/xfce4-alsa-plugin/patches/0002-tooltip-no-sound-card.patch
```

**[`0001-meson-qnx-libasound-without-alsa-pc.patch`](patches/0001-meson-qnx-libasound-without-alsa-pc.patch)** — meson: QNX's libasound has no alsa.pc

> QNX SDP 8.0 ships libasound with an ALSA-compatible API (salsa-based, the
> mixer API this plugin uses included) but without alsa.pc, so
> dependency('alsa') finds nothing. On QNX, when there is no alsa.pc, take the
> C library from the compiler's default paths and the Vala binding from the
> plugin's own src/vapi/alsa.vapi. Elsewhere nothing changes.

Touches: `meson.build` · Upstream status: Pending

**[`0002-tooltip-no-sound-card.patch`](patches/0002-tooltip-no-sound-card.patch)** — volumebutton: say "No sound card found" when there is none

> Without any sound card -- no driver running, as on a QNX system without
> io-snd or a virtual machine without audio -- the button showed "Volume
> control is not configured", which points at the settings dialog, where
> nothing can be chosen. Tell that case apart (snd_card_next finds no card)
> and say so; with a card but no element chosen the old text stays.

Touches: `src/volumebutton.vala` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfce4-alsa-plugin src/xfce4-alsa-plugin \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/xfce4-alsa-plugin
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfce4-alsa-plugin install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-alsa-plugin: a volume control for the Xfce panel on ALSA -- scroll on the button to change the volume, click for a slider and mute, a settings dialog for the card and the mixer element. On QNX it runs on the SDP's libasound (an ALSA-compatible API; the sound comes from io-snd): no PulseAudio, which xfce4-pulseaudio-plugin needs. Written in Vala: valac (build machine, docker/Dockerfile) translates it to C. Its Vala bindings come with the release (src/vapi/: alsa, libxfce4panel-2.0 -- depending only on gtk+-3.0 -- and config), so neither our xfce4-panel (built without introspection/Vala) nor the build machine has to provide them. patches/0001: libasound without alsa.pc on QNX.
- libasound is part of the QNX image (not a package here); the panel loads the plugin.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-alsa-plugin`.

### 5. What it installs

3 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/xfce4-alsa-plugin` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-alsa-plugin
ls dists/8.0/*/ | grep '^xfce4-alsa-plugin-'
```

This writes `xfce4-alsa-plugin-<ver>-r0.tar.gz` (run time) and `xfce4-alsa-plugin-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-alsa-plugin
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-alsa-plugin-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-alsa-plugin-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfce4_alsa_plugin`](../../tests/src/) links xfce4-alsa-plugin the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfce4-alsa-plugin`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfce4-alsa-plugin
/usr/local/bin/qnx-port-tests/test_xfce4_alsa_plugin
```

```
== xfce4-alsa-plugin ==
  ok   ...
PASS xfce4-alsa-plugin
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfce4_alsa_plugin` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfce4_alsa_plugin`](../../tests/src/) -- a program linked against xfce4-alsa-plugin the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xfce4-alsa-plugin`.

## Versions

This recipe builds **0.4.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-alsa-plugin=<version>`), sources in `ported/xfce4-alsa-plugin/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-alsa-plugin`; port one with `scripts/port-update xfce4-alsa-plugin`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-alsa-plugin: a volume control for the Xfce panel on ALSA -- scroll on
# the button to change the volume, click for a slider and mute, a settings
# dialog for the card and the mixer element. On QNX it runs on the SDP's
# libasound (an ALSA-compatible API; the sound comes from io-snd): no
# PulseAudio, which xfce4-pulseaudio-plugin needs. Written in Vala: valac
# (build machine, docker/Dockerfile) translates it to C.
#
# Its Vala bindings come with the release (src/vapi/: alsa, libxfce4panel-2.0
# -- depending only on gtk+-3.0 -- and config), so neither our xfce4-panel
# (built without introspection/Vala) nor the build machine has to provide
# them. patches/0001: libasound without alsa.pc on QNX.
VERSION=0.4.0
TARBALL=https://github.com/equeim/xfce4-alsa-plugin/archive/refs/tags/0.4.0.tar.gz
SHA256=16971c3c7d6324238db98c210da17755f27bc5baa080feae22f11005164ac3e2
DEPS="gtk3 xfce4-panel"
# libasound is part of the QNX image (not a package here); the panel loads the plugin.
RUNTIME_DEPS="xfce4-panel"
BUILDSYS=meson
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
