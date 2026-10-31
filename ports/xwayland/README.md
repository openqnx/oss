# xwayland 24.1.14

> X server running as a Wayland client (Xwayland)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xorg/xserver.git (tag `xwayland-24.1.14`) |
| Build system | Meson |
| QNX patches | 5 |
| Build dependencies | [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`pixman`](../pixman/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`libxau`](../libxau/) [`libxdmcp`](../libxdmcp/) [`libxfont2`](../libxfont2/) [`libxkbfile`](../libxkbfile/) [`libxcvt`](../libxcvt/) [`libxshmfence`](../libxshmfence/) [`libdrm-headers`](../libdrm-headers/) [`linux-headers-input-codes`](../linux-headers-input-codes/) [`libmd`](../libmd/) [`xkbcomp`](../xkbcomp/) [`xkeyboard-config`](../xkeyboard-config/) [`font-util`](../font-util/) |
| Run-time dependencies | [`libmd`](../libmd/) [`libxau`](../libxau/) [`libxcvt`](../libxcvt/) [`libxfont2`](../libxfont2/) [`pixman`](../pixman/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_virtualgl`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xwayland
apk add xwayland-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-virtualgl && /usr/local/bin/qnx-port-tests/test_virtualgl    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xwayland`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xwayland` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (35 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild wayland wayland-protocols pixman xorgproto xtrans libxau libxdmcp libxfont2 libxkbfile libxcvt libxshmfence libdrm-headers linux-headers-input-codes libmd xkbcomp xkeyboard-config font-util
```

<details><summary>Full build chain (35)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`font-util`](../font-util/) [`fontenc`](../fontenc/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`libdrm-headers`](../libdrm-headers/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libmd`](../libmd/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcvt`](../libxcvt/) [`libxdmcp`](../libxdmcp/) [`libxfont2`](../libxfont2/) [`libxkbfile`](../libxkbfile/) [`libxml2`](../libxml2/) [`libxshmfence`](../libxshmfence/) [`linux-headers-input-codes`](../linux-headers-input-codes/) [`ncurses`](../ncurses/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xkbcomp`](../xkbcomp/) [`xkeyboard-config`](../xkeyboard-config/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b xwayland-24.1.14 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xorg/xserver.git src/xwayland
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xwayland < ports/xwayland/patches/0001-qnx-ospoll-poll.patch
patch -p1 -d src/xwayland < ports/xwayland/patches/0002-qnx-setvbuf.patch
patch -p1 -d src/xwayland < ports/xwayland/patches/0003-no-sa-restart.patch
patch -p1 -d src/xwayland < ports/xwayland/patches/0004-qnx-input-event-codes.patch
patch -p1 -d src/xwayland < ports/xwayland/patches/0005-test-no-sa-restart.patch
```

**[`0001-qnx-ospoll-poll.patch`](patches/0001-qnx-ospoll-poll.patch)** — Use the poll() backend on QNX

> ospoll picks epoll when epoll_create1() exists, and its epoll backend uses
> edge-triggered notification (EPOLLET). QNX's epoll is an emulation
> (libepoll) without EPOLLET, so the server did not compile. On QNX use the
> portable poll() backend ospoll already has.

Touches: `os/ospoll.c` · Upstream status: Pending

**[`0002-qnx-setvbuf.patch`](patches/0002-qnx-setvbuf.patch)** — Line-buffer the log with setvbuf on QNX

> The log file is made line buffered with setlinebuf(), a BSD function QNX's
> C library does not have, except on the systems listed for the standard
> setvbuf(..., _IOLBF, BUFSIZ) form. Add QNX to that list.

Touches: `os/osinit.c` · Upstream status: Pending

**[`0003-no-sa-restart.patch`](patches/0003-no-sa-restart.patch)** — Install the scheduler signal handler without SA_RESTART where there is none

> The smart-scheduler timer signal handler is installed with SA_RESTART,
> which QNX does not define (it has no restartable system calls). Without it
> the flags are 0: the server's read and write paths already retry on EINTR.

Touches: `os/utils.c` · Upstream status: Pending

**[`0004-qnx-input-event-codes.patch`](patches/0004-qnx-input-event-codes.patch)** — Take the button codes from linux/input-event-codes.h on QNX

> xwayland-input.c includes <linux/input.h> for the BTN_* button codes it
> maps Wayland pointer buttons to. QNX has no evdev and no <linux/input.h>;
> the codes alone are in <linux/input-event-codes.h> (as Wayland compositors
> on QNX use them): include that on QNX.

Touches: `hw/xwayland/xwayland-input.c` · Upstream status: Pending

**[`0005-test-no-sa-restart.patch`](patches/0005-test-no-sa-restart.patch)** — simple-xinit: no SA_RESTART where there is none

> simple-xinit, the launcher of the test suite, sets SA_RESTART for its
> SIGCHLD handler; QNX does not define SA_RESTART. Leave it out there (as 0003
> does for the server's scheduler timer).

Touches: `test/simple-xinit.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xwayland; B=build/$QNX_ARCH/xwayland; mkdir -p $B
```

```bash
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xwayland: the X server as a Wayland client (X.Org's xserver, Xwayland only). On the QNX desktop it is an X display in a window ("Xwayland :1 -geometry WxH", rootful): X11 programs and vglclient -- VirtualGL's client, which draws OpenGL frames rendered on a Linux machine (RViz in an Ubuntu guest) -- show there. Software rendering: no glamor/DRI3 (no desktop GL or DRM render nodes on QNX); no MIT-SHM (no System V shared memory on QNX).
- Xwayland requires libdrm's headers even without DRI3: the SDP's libdrm through scripts/lib/sdp-pc.sh, as for wlroots.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xwayland`.

### 5. What it installs

6 files under `/usr/local`, 1 programs.

```
/usr/local/bin/Xwayland
```

The complete list is `stage/<arch>/.manifest/xwayland` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xwayland
ls dists/8.0/*/ | grep '^xwayland-'
```

This writes `xwayland-<ver>-r0.tar.gz` (run time) and `xwayland-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xwayland
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xwayland-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xwayland-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_virtualgl`](../../tests/src/) links xwayland the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-virtualgl`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-virtualgl
/usr/local/bin/qnx-port-tests/test_virtualgl
```

```
== xwayland ==
  ok   ...
PASS xwayland
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_virtualgl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_virtualgl`](../../tests/src/) -- a program linked against xwayland the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - labwc up (Wayland compositor)
   - Xwayland up (an X display on labwc)
   - XOpenDisplay
   - X11: drawn colour read back exactly
   - vglclient listening (VGL transport)
   - version handshake (VGL 2.x)
   - frame sent: JPEG tile + end of frame
   - vglclient drew the frame in the window

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | virtualgl 3.1.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/virtualgl-3.1.5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xwayland 24.1.14-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xwayland-24.1.14-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | virtualgl 3.1.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/virtualgl-3.1.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xwayland 24.1.14-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xwayland-24.1.14-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **24.1.14**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xwayland=<version>`), sources in `ported/xwayland/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xwayland`; port one with `scripts/port-update xwayland`.

<details><summary>recipe.sh</summary>

```bash
# xwayland: the X server as a Wayland client (X.Org's xserver, Xwayland only).
# On the QNX desktop it is an X display in a window ("Xwayland :1
# -geometry WxH", rootful): X11 programs and vglclient -- VirtualGL's client,
# which draws OpenGL frames rendered on a Linux machine (RViz in an Ubuntu
# guest) -- show there. Software rendering: no glamor/DRI3 (no desktop GL or
# DRM render nodes on QNX); no MIT-SHM (no System V shared memory on QNX).
VERSION=24.1.14; GIT=https://gitlab.freedesktop.org/xorg/xserver.git; REF=xwayland-24.1.14
DEPS="wayland wayland-protocols pixman xorgproto xtrans libxau libxdmcp libxfont2 libxkbfile libxcvt libxshmfence libdrm-headers linux-headers-input-codes libmd xkbcomp xkeyboard-config font-util"
RUNTIME_DEPS="xkbcomp xkeyboard-config"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dglamor=false -Dglx=false -Ddri3=false -Ddrm=false -Dmitshm=false -Dxvfb=false
                   -Dxdmcp=false -Dxdm-auth-1=false -Dsecure-rpc=false -Dlibdecor=false -Dxwayland_ei=false
                   -Dxselinux=false -Dsystemd_notify=false -Ddocs=false -Ddevel-docs=false -Ddocs-pdf=false
                   -Dsha1=libmd -Dlisten_tcp=true -Dxkb_dir=/usr/local/share/X11/xkb -Dxkb_bin_dir=/usr/local/bin
                   -Dxkb_output_dir=/tmp )
# Xwayland requires libdrm's headers even without DRI3: the SDP's libdrm
# through scripts/lib/sdp-pc.sh, as for wlroots.
source "$WS/scripts/lib/sdp-pc.sh"
do_configure() { qnx_sdp_pc && qnx_meson_setup; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
