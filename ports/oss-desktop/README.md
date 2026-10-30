# oss-desktop 1.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`labwc`](../labwc/) [`swaybg`](../swaybg/) [`xfce4-panel`](../xfce4-panel/) [`xfce4-terminal`](../xfce4-terminal/) [`thunar`](../thunar/) [`xfce4-appfinder`](../xfce4-appfinder/) [`dbus`](../dbus/) [`hicolor-icon-theme`](../hicolor-icon-theme/) [`lxde-icon-theme`](../lxde-icon-theme/) [`font-dejavu`](../font-dejavu/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_labwc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add oss-desktop
apk add oss-desktop-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-labwc && /usr/local/bin/qnx-port-tests/test_labwc    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install oss-desktop`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild oss-desktop` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (88 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu
```

<details><summary>Full build chain (88)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`exo`](../exo/) [`expat`](../expat/) [`font-dejavu`](../font-dejavu/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`garcon`](../garcon/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`harfbuzz`](../harfbuzz/) [`hicolor-icon-theme`](../hicolor-icon-theme/) [`icu`](../icu/) [`labwc`](../labwc/) [`lcms2`](../lcms2/) [`libdrm-headers`](../libdrm-headers/) [`libepoxy`](../libepoxy/) [`libexif`](../libexif/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libnotify`](../libnotify/) [`libpng`](../libpng/) [`libsfdo`](../libsfdo/) [`libtasn1`](../libtasn1/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfce4windowing`](../libxfce4windowing/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`linux-headers-input-codes`](../linux-headers-input-codes/) [`lxde-icon-theme`](../lxde-icon-theme/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`swaybg`](../swaybg/) [`thunar`](../thunar/) [`vte`](../vte/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`wlroots`](../wlroots/) [`xcb-proto`](../xcb-proto/) [`xfce4-appfinder`](../xfce4-appfinder/) [`xfce4-panel`](../xfce4-panel/) [`xfce4-terminal`](../xfce4-terminal/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

```bash
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-desktop/files/." "$SRCROOT/"; }
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/oss-desktop; B=build/$QNX_ARCH/oss-desktop; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() {
  install -Dm755 "$S/bin/oss-desktop" "$STAGE$PREFIX/bin/oss-desktop"
  install -Dm755 "$S/bin/oss-desktop-menu" "$STAGE$PREFIX/bin/oss-desktop-menu"
  install -d "$STAGE$PREFIX/share/oss-desktop" && cp -a "$S/share/oss-desktop/." "$STAGE$PREFIX/share/oss-desktop/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- oss-desktop: a desktop session on QNX Screen from the ported components, in the spirit of QNX Everywhere's Developer Desktop -- labwc (window manager, on wlroots' QNX Screen backend), swaybg (background), the Xfce panel with an application menu, the Xfce terminal and file manager. The root menu (right click on the background) lists every installed application (.desktop entry) and runs the port test suite in a terminal. This component is only the session: a start script, oss-desktop, and labwc's configuration (files/); the programs are the components below.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH oss-desktop`.

### 5. What it installs

5 files under `/usr/local`, 2 programs.

```
/usr/local/bin/oss-desktop
/usr/local/bin/oss-desktop-menu
```

The complete list is `stage/<arch>/.manifest/oss-desktop` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa oss-desktop
ls dists/8.0/*/ | grep '^oss-desktop-'
```

This writes `oss-desktop-<ver>-r0.tar.gz` (run time) and `oss-desktop-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add oss-desktop
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/oss-desktop-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/oss-desktop-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_labwc`](../../tests/src/) links oss-desktop the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-labwc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-labwc
/usr/local/bin/qnx-port-tests/test_labwc
```

```
== oss-desktop ==
  ok   ...
PASS oss-desktop
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_labwc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_labwc`](../../tests/src/) -- a program linked against oss-desktop the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - XDG_RUNTIME_DIR
   - labwc started
   - labwc's Wayland socket wayland-0 appeared
   - client connected to labwc
   - swaybg running on labwc (background layer surface)
   - labwc still running after its clients
   - labwc exits on SIGTERM

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | labwc 0.20.2-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | oss-desktop 1.0-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | swaybg 1.2.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | labwc 0.20.2-r0 | ❌ FAIL | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | labwc 0.20.2-r1 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | oss-desktop 1.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | swaybg 1.2.2-r0 | ❌ FAIL | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | swaybg 1.2.2-r1 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add oss-desktop=<version>`), sources in `ported/oss-desktop/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates oss-desktop`; port one with `scripts/port-update oss-desktop`.

<details><summary>recipe.sh</summary>

```bash
# oss-desktop: a desktop session on QNX Screen from the ported components, in
# the spirit of QNX Everywhere's Developer Desktop -- labwc (window manager,
# on wlroots' QNX Screen backend), swaybg (background), the Xfce panel with
# an application menu, the Xfce terminal and file manager. The root menu
# (right click on the background) lists every installed application
# (.desktop entry) and runs the port test suite in a terminal.
# This component is only the session: a start script, oss-desktop, and
# labwc's configuration (files/); the programs are the components below.
VERSION=1.0
DEPS="labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu"
RUNTIME_DEPS="labwc swaybg xfce4-panel xfce4-terminal thunar xfce4-appfinder dbus hicolor-icon-theme lxde-icon-theme font-dejavu"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-desktop/files/." "$SRCROOT/"; }
do_build() { :; }
do_install() {
  install -Dm755 "$S/bin/oss-desktop" "$STAGE$PREFIX/bin/oss-desktop"
  install -Dm755 "$S/bin/oss-desktop-menu" "$STAGE$PREFIX/bin/oss-desktop-menu"
  install -d "$STAGE$PREFIX/share/oss-desktop" && cp -a "$S/share/oss-desktop/." "$STAGE$PREFIX/share/oss-desktop/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
