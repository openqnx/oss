# gtk4 4.16.7

> GTK 4 multi-platform toolkit for graphical user interfaces

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.gnome.org/sources/gtk/4.16/gtk-4.16.7.tar.xz |
| SHA-256 | `5303c7624e15a48880591b9ddd43389afba3de4fb92a219719519bb1642ce3dc` |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`graphene`](../graphene/) [`cairo`](../cairo/) [`pango`](../pango/) [`gdk-pixbuf`](../gdk-pixbuf/) [`harfbuzz`](../harfbuzz/) [`fribidi`](../fribidi/) [`libepoxy`](../libepoxy/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`libxkbcommon`](../libxkbcommon/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libtiff`](../libtiff/) |
| Run-time dependencies | [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphene`](../graphene/) [`harfbuzz`](../harfbuzz/) [`libepoxy`](../libepoxy/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libxkbcommon`](../libxkbcommon/) [`pango`](../pango/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gtk4`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gtk4` (programs, shared libraries, run-time data) and `gtk4-dev` (headers, pkg-config/CMake files, static libraries; needs `gtk4`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gtk4`](https://openqnx.github.io/oss/package/x86_64/gtk4.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gtk4.html)) · [`gtk4-dev`](https://openqnx.github.io/oss/package/x86_64/gtk4-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gtk4-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gtk4` (dependencies come along); one version: `apk add gtk4=<version>` | `apk del gtk4` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gtk4` (dependencies first) | `oss-get remove gtk4` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gtk4-dev` the same way, by its own name (`apk add gtk4-dev`, `oss-get install gtk4-dev`, by hand with its own file and list `gtk4-dev.list`); remove it before `gtk4`, which it needs.

```sh
# on the QNX target, as root
apk add gtk4 gtk4-dev
apk add qnx-test-gtk4 && /usr/local/bin/qnx-port-tests/test_gtk4    # its on-target test, from tests-8.0-<arch>
apk del gtk4 gtk4-dev qnx-test-gtk4
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gtk4.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gtk4.list
rm var/lib/manual/gtk4.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gtk4` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (51 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff
```

<details><summary>Full build chain (51)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://download.gnome.org/sources/gtk/4.16/gtk-4.16.7.tar.xz
mkdir -p dist src/gtk4
curl -fL -o dist/$(basename $URL) $URL
echo "5303c7624e15a48880591b9ddd43389afba3de4fb92a219719519bb1642ce3dc  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gtk4 --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gtk4 < ports/gtk4/patches/0001-qnx-no-linux-headers.patch
```

**[`0001-qnx-no-linux-headers.patch`](patches/0001-qnx-no-linux-headers.patch)** — GTK 4: Wayland backend without Linux headers.

> QNX has no <sys/syscall.h> (only memfd_create's guarded fast path used it; the
> shm_open() fallback is taken) and no <linux/input.h> or <dev/evdev/input.h>;
> define the BTN_* codes the wl_pointer protocol fixes.

Touches: `gdk/wayland/gdkdisplay-wayland.c`, `gdk/wayland/gdkseat-wayland.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gtk4 src/gtk4 \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dwayland-backend=true \
      -Dx11-backend=false \
      -Dbroadway-backend=true \
      -Dwin32-backend=false \
      -Dmacos-backend=false \
      -Df16c=disabled \
      -Dvulkan=disabled \
      -Dmedia-gstreamer=disabled \
      -Dcloudproviders=disabled \
      -Dsysprof=disabled \
      -Dtracker=disabled \
      -Dcolord=disabled \
      -Dintrospection=disabled \
      -Ddocumentation=false \
      -Dscreenshots=false \
      -Dman-pages=false \
      -Dbuild-demos=false \
      -Dbuild-testsuite=false \
      -Dbuild-examples=false \
      -Dbuild-tests=false \
      -Dprint-cups=disabled
ninja -C build/$QNX_ARCH/gtk4
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gtk4 install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball, not the git tag. gtk/meson.build says it outright -- "For git checkouts, but not for tarballs..." -- and from git it compiles the CSS themes with sassc, a *build-machine* program it falls back to building as a subproject and then running, which a cross build cannot do ("An exe_wrapper is needed but was not found"). The tarball ships the compiled themes.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gtk4`.

### 5. What it installs

550 files under `/usr/local`: 3 shared-library entries, 9 programs, 382 headers.

```
/usr/local/lib/libgtk-4.so
/usr/local/lib/libgtk-4.so.1
/usr/local/lib/libgtk-4.so.1.1600.7
/usr/local/bin/gtk4-broadwayd
/usr/local/bin/gtk4-builder-tool
/usr/local/bin/gtk4-encode-symbolic-svg
/usr/local/bin/gtk4-image-tool
/usr/local/bin/gtk4-launch
/usr/local/bin/gtk4-path-tool
/usr/local/bin/gtk4-query-settings
/usr/local/bin/gtk4-rendernode-tool
/usr/local/bin/gtk4-update-icon-cache
```

The complete list is `stage/<arch>/.manifest/gtk4` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gtk4
ls dists/8.0/*/ | grep '^gtk4-'
```

This writes `gtk4-<ver>-r0.tar.gz` (run time) and `gtk4-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gtk4
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gtk4-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gtk4-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gtk4`](../../tests/src/) links gtk4 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gtk4`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gtk4
/usr/local/bin/qnx-port-tests/test_gtk4
```

```
== gtk4 ==
  ok   ...
PASS gtk4
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gtk4` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gtk4`](../../tests/src/) -- a program linked against gtk4 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - GTK 4.x
   - gtk_init_check reported no display rather than aborting
   - GtkWidget type is registered
   - GtkButton derives from GtkWidget
   - GtkBox derives from GtkWidget
   - gtk_css_provider_new
   - gsk_color_node_new
   - render node keeps its bounds
   - gsk_render_node_serialize
   - created a real button
   - button label

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gtk4 4.16.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gtk4-4.16.7-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gtk4 4.16.7-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gtk4-4.16.7-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.16.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gtk4=<version>`), sources in `ported/gtk4/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gtk4`; port one with `scripts/port-update gtk4`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.16.7
# The release tarball, not the git tag. gtk/meson.build says it outright --
# "For git checkouts, but not for tarballs..." -- and from git it compiles the
# CSS themes with sassc, a *build-machine* program it falls back to building as
# a subproject and then running, which a cross build cannot do ("An exe_wrapper
# is needed but was not found"). The tarball ships the compiled themes.
TARBALL=https://download.gnome.org/sources/gtk/4.16/gtk-4.16.7.tar.xz
SHA256=5303c7624e15a48880591b9ddd43389afba3de4fb92a219719519bb1642ce3dc
SRCROOT="$SRC_DIR/gtk4"
DEPS="glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy
      wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dwayland-backend=true -Dx11-backend=false -Dbroadway-backend=true
  -Dwin32-backend=false -Dmacos-backend=false
  -Df16c=disabled
  -Dvulkan=disabled -Dmedia-gstreamer=disabled -Dcloudproviders=disabled
  -Dsysprof=disabled -Dtracker=disabled -Dcolord=disabled
  -Dintrospection=disabled -Ddocumentation=false -Dscreenshots=false
  -Dman-pages=false -Dbuild-demos=false -Dbuild-testsuite=false
  -Dbuild-examples=false -Dbuild-tests=false -Dprint-cups=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
