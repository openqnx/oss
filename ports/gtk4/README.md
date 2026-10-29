# gtk4 4.16.7

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

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gtk4
oss-get install gtk4-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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

**Build the dependencies first** (46 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib graphene cairo pango gdk-pixbuf harfbuzz fribidi libepoxy wayland wayland-protocols libxkbcommon libpng libjpeg-turbo libtiff
```

<details><summary>Full build chain (46)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxft`](../libxft/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
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
./scripts/qnxpkg -a $QNX_ARCH gtk4
ls repo/8.0/*/ | grep '^gtk4-'
```

This writes `gtk4-<ver>-r0.tar.gz` (run time) and `gtk4-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gtk4
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gtk4-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gtk4-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_gtk4`](../../tests/src/) links gtk4 the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gtk4 root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_gtk4'
```

```
== gtk4 ==
  ok   ...
PASS gtk4
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

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
