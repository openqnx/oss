# labwc 0.20.2

> Stacking Wayland compositor (window manager) based on wlroots

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/labwc/labwc.git (tag `0.20.2`) |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`wlroots`](../wlroots/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`libxkbcommon`](../libxkbcommon/) [`glib`](../glib/) [`cairo`](../cairo/) [`pango`](../pango/) [`libxml2`](../libxml2/) [`libsfdo`](../libsfdo/) [`libpng`](../libpng/) |
| Run-time dependencies | [`cairo`](../cairo/) [`glib`](../glib/) [`libpng`](../libpng/) [`libsfdo`](../libsfdo/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`pango`](../pango/) [`pixman`](../pixman/) [`wayland`](../wayland/) [`wlroots`](../wlroots/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_labwc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add labwc
apk add labwc-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-labwc && /usr/local/bin/qnx-port-tests/test_labwc    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install labwc`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild labwc` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (43 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng
```

<details><summary>Full build chain (43)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`lcms2`](../lcms2/) [`libdrm-headers`](../libdrm-headers/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libsfdo`](../libsfdo/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`linux-headers-input-codes`](../linux-headers-input-codes/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`wlroots`](../wlroots/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.20.2 --recurse-submodules --shallow-submodules \
    https://github.com/labwc/labwc.git src/labwc
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/labwc < ports/labwc/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/labwc < ports/labwc/patches/0002-no-theme-no-theme-dirs.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — labwc on QNX, from QNX's port (github.com/qnx-ports/labwc, tag qnx-0.20.2-r0,

> commit 5f4c032a): libinput becomes optional (QNX has none: the libinput
> configuration and the seat's libinput device handling are compiled out when
> wlroots has no libinput backend), and on the QNX Screen backend labwc creates
> the qnx-redir global QNX's compositors provide.
>
>
> initial draft port
>
> ---
>  include/labwc.h        |  2 ++
>  src/config/meson.build |  7 ++++++-
>  src/config/rcxml.c     | 13 +++++++++++--
>  src/seat.c             |  2 ++
>  src/server.c           | 25 +++++++++++++++++++++++++
>  5 files changed, 46 insertions(+), 3 deletions(-)

Touches: `include/labwc.h`, `src/config/meson.build`, `src/config/rcxml.c`, `src/seat.c`, `src/server.c` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

**[`0002-no-theme-no-theme-dirs.patch`](patches/0002-no-theme-no-theme-dirs.patch)** — Do not build theme paths when no theme is configured

> Without a theme in rc.xml, rc.theme_name is NULL, and loading the window
> buttons asks for theme directories under that name: snprintf("%s", NULL).
> glibc prints "(null)", finds no such directory and goes on; QNX's libc
> crashes in strlen -- labwc died a few seconds after start on every QNX target
> with no theme configured (SIGSEGV in build_theme_path_labwc, from
> load_buttons). No theme name means no theme directories.

Touches: `src/common/dir.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/labwc; B=build/$QNX_ARCH/labwc; mkdir -p $B
```

```bash
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- labwc: a stacking Wayland compositor (window manager) on wlroots -- the one QNX's Developer Desktop runs, with QNX's changes (patches/0001, from github.com/qnx-ports/labwc). Xfce's panel and applications run on it.
- labwc will not start without a font (titles, menus) nor without keymaps.
- wlroots-0.20.pc requires libdrm (the SDP's, see scripts/lib/sdp-pc.sh).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH labwc`.

### 5. What it installs

15 files under `/usr/local`, 3 programs.

```
/usr/local/bin/lab-sensible-terminal
/usr/local/bin/labnag
/usr/local/bin/labwc
```

The complete list is `stage/<arch>/.manifest/labwc` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa labwc
ls dists/8.0/*/ | grep '^labwc-'
```

This writes `labwc-<ver>-r0.tar.gz` (run time) and `labwc-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add labwc
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/labwc-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/labwc-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_labwc`](../../tests/src/) links labwc the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-labwc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-labwc
/usr/local/bin/qnx-port-tests/test_labwc
```

```
== labwc ==
  ok   ...
PASS labwc
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_labwc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_labwc`](../../tests/src/) -- a program linked against labwc the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - XDG_RUNTIME_DIR
   - labwc started
   - labwc's Wayland socket wayland-0 appeared
   - client connected to labwc
   - swaybg running on labwc (background layer surface)
   - labwc still running after its clients
   - labwc exits on SIGTERM

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | labwc 0.20.2-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/labwc-0.20.2-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | oss-desktop 1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/oss-desktop-1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | swaybg 1.2.2-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/swaybg-1.2.2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | labwc 0.20.2-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/labwc-0.20.2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | oss-desktop 1.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/oss-desktop-1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | swaybg 1.2.2-r1 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/swaybg-1.2.2-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.20.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add labwc=<version>`), sources in `ported/labwc/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates labwc`; port one with `scripts/port-update labwc`.

<details><summary>recipe.sh</summary>

```bash
# labwc: a stacking Wayland compositor (window manager) on wlroots -- the one
# QNX's Developer Desktop runs, with QNX's changes (patches/0001, from
# github.com/qnx-ports/labwc). Xfce's panel and applications run on it.
VERSION=0.20.2; GIT=https://github.com/labwc/labwc.git; REF=0.20.2
DEPS="wlroots wayland wayland-protocols libxkbcommon glib cairo pango libxml2 libsfdo libpng"
BUILDSYS=meson
# labwc will not start without a font (titles, menus) nor without keymaps.
RUNTIME_DEPS="font-dejavu xkeyboard-config"
MESON_ARGS_EXTRA=( -Dxwayland=disabled -Dsvg=disabled -Dnls=disabled -Dman-pages=disabled
                   -Dicon=enabled -Dstatic_analyzer=disabled )
# wlroots-0.20.pc requires libdrm (the SDP's, see scripts/lib/sdp-pc.sh).
source "$WS/scripts/lib/sdp-pc.sh"
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
