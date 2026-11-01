# libpanel 1.10.4

> Dock/panel library for GTK 4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/libpanel/1.10/libpanel-1.10.4.tar.xz |
| SHA-256 | `593888a7691f0af8aaa6e193c9e14afa86a810c0c2f27515c6d813f18733b1cd` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gtk4`](../gtk4/) [`libadwaita`](../libadwaita/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphene`](../graphene/) [`gtk4`](../gtk4/) [`libadwaita`](../libadwaita/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libpanel`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libpanel` (programs, shared libraries, run-time data) and `libpanel-dev` (headers, pkg-config/CMake files, static libraries; needs `libpanel`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libpanel`](https://openqnx.github.io/oss/package/x86_64/libpanel.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libpanel.html)) · [`libpanel-dev`](https://openqnx.github.io/oss/package/x86_64/libpanel-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libpanel-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libpanel` (dependencies come along); one version: `apk add libpanel=<version>` | `apk del libpanel` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libpanel` (dependencies first) | `oss-get remove libpanel` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libpanel-dev` the same way, by its own name (`apk add libpanel-dev`, `oss-get install libpanel-dev`, by hand with its own file and list `libpanel-dev.list`); remove it before `libpanel`, which it needs.

```sh
# on the QNX target, as root
apk add libpanel libpanel-dev
apk add qnx-test-libpanel && /usr/local/bin/qnx-port-tests/test_libpanel    # its on-target test, from tests-8.0-<arch>
apk del libpanel libpanel-dev qnx-test-libpanel
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libpanel.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libpanel.list
rm var/lib/manual/libpanel.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libpanel` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (59 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk4 libadwaita
```

<details><summary>Full build chain (59)</summary>

[`appstream`](../appstream/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`cairo`](../cairo/) [`curl`](../curl/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gtk4`](../gtk4/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libadwaita`](../libadwaita/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxmlb`](../libxmlb/) [`libxrender`](../libxrender/) [`libyaml`](../libyaml/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/libpanel/1.10/libpanel-1.10.4.tar.xz
mkdir -p dist src/libpanel
curl -fL -o dist/$(basename $URL) $URL
echo "593888a7691f0af8aaa6e193c9e14afa86a810c0c2f27515c6d813f18733b1cd  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libpanel --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libpanel src/libpanel \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dintrospection=disabled \
      -Dvapi=false \
      -Ddocs=disabled \
      -Dinstall-examples=false
ninja -C build/$QNX_ARCH/libpanel
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libpanel install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libpanel: dock/panel layout library for GTK 4 and libadwaita (resizable docks and paned layouts, frames with tabs, widgets that move between them, save/restore of the layout, the omni bar) used by GNOME Builder.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libpanel`.

### 5. What it installs

98 files under `/usr/local`: 3 shared-library entries, 36 headers.

```
/usr/local/lib/libpanel-1.so
/usr/local/lib/libpanel-1.so.1
/usr/local/lib/libpanel-1.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/libpanel` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libpanel
ls dists/8.0/*/ | grep '^libpanel-'
```

This writes `libpanel-<ver>-r0.tar.gz` (run time) and `libpanel-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libpanel
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libpanel-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libpanel-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libpanel`](../../tests/src/) links libpanel the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libpanel`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libpanel
/usr/local/bin/qnx-port-tests/test_libpanel
```

```
== libpanel ==
  ok   ...
PASS libpanel
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libpanel` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libpanel`](../../tests/src/) -- a program linked against libpanel the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - labwc (headless) Wayland socket appeared
   - gtk_init_check on labwc
   - panel types registered
   - PanelPosition round trip through a GVariant
   - a new position is indeterminate, a set one is not
   - PanelSession restored from its GVariant
   - session item: module, metadata (uri, line) and position restored
   - window with a dock, a grid and a frame mapped
   - grid holds the widgets in frames
   - widget title and modified flag
   - frame with two pages
   - frame switches to page A
   - panel_widget_raise brings page B forward
   - a closed page leaves the frame

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libpanel 1.10.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libpanel-1.10.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libpanel 1.10.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libpanel-1.10.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.10.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libpanel=<version>`), sources in `ported/libpanel/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libpanel`; port one with `scripts/port-update libpanel`.

<details><summary>recipe.sh</summary>

```bash
# libpanel: dock/panel layout library for GTK 4 and libadwaita (resizable
# docks and paned layouts, frames with tabs, widgets that move between
# them, save/restore of the layout, the omni bar) used by GNOME Builder.
VERSION=1.10.4
TARBALL=https://download.gnome.org/sources/libpanel/1.10/libpanel-1.10.4.tar.xz
SHA256=593888a7691f0af8aaa6e193c9e14afa86a810c0c2f27515c6d813f18733b1cd
DEPS="glib gtk4 libadwaita"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=disabled -Dvapi=false -Ddocs=disabled -Dinstall-examples=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
