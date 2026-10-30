# swaybg 1.2.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/swaywm/swaybg.git (tag `v1.2.2`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_labwc`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add swaybg
apk add swaybg-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-labwc && /usr/local/bin/qnx-port-tests/test_labwc    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install swaybg`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild swaybg` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (33 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild wayland wayland-protocols cairo gdk-pixbuf glib
```

<details><summary>Full build chain (33)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.2.2 --recurse-submodules --shallow-submodules \
    https://github.com/swaywm/swaybg.git src/swaybg
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/swaybg < ports/swaybg/patches/0001-null-output-identifier.patch
```

**[`0001-null-output-identifier.patch`](patches/0001-null-output-identifier.patch)** — Do not print a NULL output identifier

> An output's identifier comes from its wl_output description; an output that
> sends none (wlroots' headless outputs, for one) keeps it NULL, and the debug
> messages print it with "%s". glibc prints "(null)"; QNX's libc crashes in
> strlen: swaybg died with "Memory fault" on its first output under labwc.
> Print "-" for an output without one.

Touches: `main.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/swaybg src/swaybg \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgdk-pixbuf=enabled \
      -Dman-pages=disabled
ninja -C build/$QNX_ARCH/swaybg
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/swaybg install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- swaybg: the desktop background (wallpaper) on wlroots compositors such as labwc.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH swaybg`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/swaybg
```

The complete list is `stage/<arch>/.manifest/swaybg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa swaybg
ls dists/8.0/*/ | grep '^swaybg-'
```

This writes `swaybg-<ver>-r0.tar.gz` (run time) and `swaybg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add swaybg
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/swaybg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/swaybg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_labwc`](../../tests/src/) links swaybg the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-labwc`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-labwc
/usr/local/bin/qnx-port-tests/test_labwc
```

```
== swaybg ==
  ok   ...
PASS swaybg
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_labwc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_labwc`](../../tests/src/) -- a program linked against swaybg the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | labwc 0.20.2-r1 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | oss-desktop 1.0-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | swaybg 1.2.2-r1 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | labwc 0.20.2-r1 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | oss-desktop 1.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | swaybg 1.2.2-r1 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.2.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add swaybg=<version>`), sources in `ported/swaybg/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates swaybg`; port one with `scripts/port-update swaybg`.

<details><summary>recipe.sh</summary>

```bash
# swaybg: the desktop background (wallpaper) on wlroots compositors such as labwc.
VERSION=1.2.2; GIT=https://github.com/swaywm/swaybg.git; REF=v1.2.2
DEPS="wayland wayland-protocols cairo gdk-pixbuf glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dgdk-pixbuf=enabled -Dman-pages=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
