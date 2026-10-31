# wlroots 0.20.2

> Modular library for building Wayland compositors

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/wlroots/wlroots.git (tag `0.20.2`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`pixman`](../pixman/) [`libxkbcommon`](../libxkbcommon/) [`lcms2`](../lcms2/) [`libdrm-headers`](../libdrm-headers/) [`linux-headers-input-codes`](../linux-headers-input-codes/) |
| Run-time dependencies | [`lcms2`](../lcms2/) [`libxkbcommon`](../libxkbcommon/) [`pixman`](../pixman/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_wlroots`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `wlroots` (programs, shared libraries, run-time data) and `wlroots-dev` (headers, pkg-config/CMake files, static libraries; needs `wlroots`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`wlroots`](https://openqnx.github.io/oss/package/x86_64/wlroots.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/wlroots.html)) · [`wlroots-dev`](https://openqnx.github.io/oss/package/x86_64/wlroots-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/wlroots-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add wlroots` (dependencies come along); one version: `apk add wlroots=<version>` | `apk del wlroots` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install wlroots` (dependencies first) | `oss-get remove wlroots` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`wlroots-dev` the same way, by its own name (`apk add wlroots-dev`, `oss-get install wlroots-dev`, by hand with its own file and list `wlroots-dev.list`); remove it before `wlroots`, which it needs.

```sh
# on the QNX target, as root
apk add wlroots wlroots-dev
apk add qnx-test-wlroots && /usr/local/bin/qnx-port-tests/test_wlroots    # its on-target test
apk del wlroots wlroots-dev qnx-test-wlroots
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/wlroots.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/wlroots.list
rm var/lib/manual/wlroots.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild wlroots` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes
```

<details><summary>Full build chain (17)</summary>

[`expat`](../expat/) [`gettext`](../gettext/) [`lcms2`](../lcms2/) [`libdrm-headers`](../libdrm-headers/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`linux-headers-input-codes`](../linux-headers-input-codes/) [`ncurses`](../ncurses/) [`pixman`](../pixman/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.20.2 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/wlroots/wlroots.git src/wlroots
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/wlroots < ports/wlroots/patches/0001-qnx-screen-backend-from-qnx-ports.patch
```

**[`0001-qnx-screen-backend-from-qnx-ports.patch`](patches/0001-qnx-screen-backend-from-qnx-ports.patch)** — The QNX Screen backend for wlroots, from QNX's port (github.com/qnx-ports/wlroots,

> tag qnx-0.20.2-r0, commit 09fc12b1): outputs are QNX Screen windows, input
> comes from Screen events (QNX has no libinput/evdev/udev), rendering is EGL/GLES
> through QNX's libEGL/libGLESv2 on Screen buffers. Also the qnx-redir protocol
> QNX's compositors use, and the small core changes the backend needs.
>
>
> initial drafT
>
> ---
>  backend/backend.c                          |  34 +
>  backend/meson.build                        |   3 +
>  backend/qnx-screen/backend.c               | 432 +++++++++++++
>  backend/qnx-screen/event_monitor.c         | 134 ++++
>  backend/qnx-screen/input_device.c          | 427 +++++++++++++
>  backend/qnx-screen/meson.build             |  34 +
>  backend/qnx-screen/output.c                | 686 +++++++++++++++++++++
>  include/backend/qnx_screen.h               | 125 ++++
>  include/backend/qnx_screen_event_monitor.h |  37 ++
>  include/meson.build                        |   3 +
>  include/render/egl.h                       |   2 +
>  include/wlr/backend/qnx_screen.h           |  85 +++
>  include/wlr/config.h.in                    |   8 +
>  include/wlr/types/wlr_qnx_redir.h          |  39 ++
>  meson.build                                |   1 +
>  meson.options                              |   2 +-
>  protocol/meson.build                       |   3 +
>  protocol/qnx-redir.xml                     |  18 +
>  render/egl.c                               |  61 +-
>  tinywl/tinywl.c                            |   3 +
>  types/meson.build                          |   1 +
>  types/scene/surface.c                      |   2 +
>  types/wlr_compositor.c                     |   5 +
>  types/wlr_qnx_redir.c                      |  63 ++
>  util/box.c                                 |   4 +
>  25 files changed, 2202 insertions(+), 10 deletions(-)
>  create mode 100644 backend/qnx-screen/backend.c
>  create mode 100644 backend/qnx-screen/event_monitor.c
>  create mode 100644 backend/qnx-screen/input_device.c
>  create mode 100644 backend/qnx-screen/meson.build
>  create mode 100644 backend/qnx-screen/output.c
>  create mode 100644 include/backend/qnx_screen.h
>  create mode 100644 include/backend/qnx_screen_event_monitor.h
>  create mode 100644 include/wlr/backend/qnx_screen.h
>  create mode 100644 include/wlr/types/wlr_qnx_redir.h
>  create mode 100644 protocol/qnx-redir.xml
>  create mode 100644 types/wlr_qnx_redir.c

Touches: `backend/backend.c`, `backend/meson.build`, `backend/qnx-screen/backend.c`, `backend/qnx-screen/event_monitor.c`, `backend/qnx-screen/input_device.c`, `backend/qnx-screen/meson.build` … · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/wlroots; B=build/$QNX_ARCH/wlroots; mkdir -p $B
```

```bash
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- wlroots: the Wayland compositor library labwc is built on, with QNX's QNX Screen backend (patches/0001, from github.com/qnx-ports/wlroots): outputs are Screen windows, input is Screen events, rendering is GLES through the SDP's libEGL/libGLESv2. libdrm (drm_fourcc.h) and libgbm come from the SDP (8.0.5 ships both; newer DRM kernel headers: libdrm-headers). The Linux backends (DRM/KMS, libinput, X11) and Xwayland are off.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH wlroots`.

### 5. What it installs

121 files under `/usr/local`: 1 shared-library entries, 118 headers.

```
/usr/local/lib/libwlroots-0.20.so
```

The complete list is `stage/<arch>/.manifest/wlroots` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa wlroots
ls dists/8.0/*/ | grep '^wlroots-'
```

This writes `wlroots-<ver>-r0.tar.gz` (run time) and `wlroots-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add wlroots
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/wlroots-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/wlroots-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_wlroots`](../../tests/src/) links wlroots the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-wlroots`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-wlroots
/usr/local/bin/qnx-port-tests/test_wlroots
```

```
== wlroots ==
  ok   ...
PASS wlroots
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_wlroots` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_wlroots`](../../tests/src/) -- a program linked against wlroots the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - wl_display_create
   - wlr_headless_backend_create
   - wlr_renderer_autocreate (pixman)
   - wlr_allocator_autocreate
   - wlr_backend_start
   - headless output announced
   - wlr_output_init_render
   - output enabled
   - wlr_scene_rect_create
   - wlr_scene_output_create
   - scene rendered a frame
   - inside the rectangle: red
   - outside the rectangle: not red
   - frame buffer readable
   - frame committed
   - QNX Screen backend created (Screen is running)
   - QNX Screen backend: fails cleanly without Screen

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wlroots 0.20.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/wlroots-0.20.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wlroots 0.20.2-r0 | ✅ PASS | 2026-10-30 | [log](../../tests/logs/x86_64/wlroots-0.20.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wlroots 0.20.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/wlroots-0.20.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.20.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add wlroots=<version>`), sources in `ported/wlroots/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates wlroots`; port one with `scripts/port-update wlroots`.

<details><summary>recipe.sh</summary>

```bash
# wlroots: the Wayland compositor library labwc is built on, with QNX's QNX
# Screen backend (patches/0001, from github.com/qnx-ports/wlroots): outputs
# are Screen windows, input is Screen events, rendering is GLES through the
# SDP's libEGL/libGLESv2. libdrm (drm_fourcc.h) and libgbm come from the SDP
# (8.0.5 ships both; newer DRM kernel headers: libdrm-headers). The Linux backends (DRM/KMS, libinput, X11) and
# Xwayland are off.
VERSION=0.20.2; GIT=https://gitlab.freedesktop.org/wlroots/wlroots.git; REF=0.20.2
DEPS="wayland wayland-protocols pixman libxkbcommon lcms2 libdrm-headers linux-headers-input-codes"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dbackends=qnx-screen -Drenderers=gles2 -Dallocators=auto
                   -Dxwayland=disabled -Dsession=disabled -Dexamples=false
                   -Dxcb-errors=disabled -Dwerror=false -Dcolor-management=enabled -Dlibliftoff=disabled )
source "$WS/scripts/lib/sdp-pc.sh"
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
