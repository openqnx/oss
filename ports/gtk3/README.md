# gtk3 3.24.43

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gtk.git (tag `3.24.43`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`cairo`](../cairo/) [`pango`](../pango/) [`gdk-pixbuf`](../gdk-pixbuf/) [`at-spi2-core`](../at-spi2-core/) [`libepoxy`](../libepoxy/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`libxkbcommon`](../libxkbcommon/) [`fribidi`](../fribidi/) [`harfbuzz`](../harfbuzz/) [`fontconfig`](../fontconfig/) [`libx11`](../libx11/) [`libxext`](../libxext/) [`libxi`](../libxi/) [`libxrandr`](../libxrandr/) [`libxcursor`](../libxcursor/) [`libxfixes`](../libxfixes/) [`libxcomposite`](../libxcomposite/) [`libxdamage`](../libxdamage/) [`libxinerama`](../libxinerama/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`fontconfig`](../fontconfig/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`harfbuzz`](../harfbuzz/) [`libepoxy`](../libepoxy/) [`libx11`](../libx11/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxrandr`](../libxrandr/) [`pango`](../pango/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gtk3`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gtk3
apk add gtk3-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-gtk3 && /usr/local/bin/qnx-port-tests/test_gtk3    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gtk3`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gtk3` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama
```

<details><summary>Full build chain (51)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.24.43 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gtk.git src/gtk3
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gtk3 < ports/gtk3/patches/0001-qnx-platform-fixes.patch
```

**[`0001-qnx-platform-fixes.patch`](patches/0001-qnx-platform-fixes.patch)** — GTK 3: Wayland input button codes without Linux headers.

> QNX has neither <linux/input.h> nor <dev/evdev/input.h>. The wl_pointer
> protocol defines its button codes with the Linux evdev numbers on every
> platform, so the few BTN_* values the Wayland backend uses are defined here.

Touches: `gdk/wayland/gdkdevice-wayland.c`, `gdk/wayland/gdkdisplay-wayland.c`, `gdk/broadway/broadwayd.c`, `gdk/broadway/gdkbroadway-server.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gtk3 src/gtk3 \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      # \
      All \
      three \
      backends. \
      Wayland \
      is \
      the \
      target \
      desktop's \
      native \
      one. \
      X11 \
      was \
      # \
      off \
      at \
      first \
      ("QNX has no X11 server"), \
      but \
      the \
      X11 \
      client \
      stack \
      is \
      staged \
      # \
      now \
      and \
      Thunar \
      4.20 \
      hard-requires \
      gdk/gdkx.h \
      (XDT_CHECK_LIBX11_REQUIRE), \
      so \
      # \
      without \
      it \
      the \
      Xfce \
      file \
      manager \
      cannot \
      build \
      at \
      all. \
      Broadway \
      (GTK's \
      HTML5 \
      # \
      backend) \
      needs \
      no \
      compositor, \
      so \
      it \
      is \
      the \
      one \
      way \
      to \
      drive \
      real \
      widgets \
      on \
      # \
      a \
      target \
      that \
      has \
      none; \
      broadwayd \
      serves \
      them \
      over \
      HTTP. \
      -Dwayland_backend=true \
      -Dx11_backend=true \
      -Dbroadway_backend=true \
      -Dprint_backends=file \
      -Dintrospection=false \
      -Dgtk_doc=false \
      -Dman=false \
      -Ddemos=false \
      -Dexamples=false \
      -Dtests=false \
      -Dinstalled_tests=false \
      -Dcolord=no \
      -Dcloudproviders=false \
      -Dtracker3=false \
      -Dxinerama=yes
ninja -C build/$QNX_ARCH/gtk3
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gtk3 install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gtk3`.

### 5. What it installs

724 files under `/usr/local`: 9 shared-library entries, 7 programs, 408 headers.

```
/usr/local/lib/libgailutil-3.so
/usr/local/lib/libgailutil-3.so.0
/usr/local/lib/libgailutil-3.so.0.0.0
/usr/local/lib/libgdk-3.so
/usr/local/lib/libgdk-3.so.0
/usr/local/lib/libgdk-3.so.0.2411.32
/usr/local/lib/libgtk-3.so
/usr/local/lib/libgtk-3.so.0
/usr/local/lib/libgtk-3.so.0.2411.32
/usr/local/bin/broadwayd
/usr/local/bin/gtk-builder-tool
/usr/local/bin/gtk-encode-symbolic-svg
/usr/local/bin/gtk-launch
/usr/local/bin/gtk-query-immodules-3.0
/usr/local/bin/gtk-query-settings
/usr/local/bin/gtk-update-icon-cache
```

The complete list is `stage/<arch>/.manifest/gtk3` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gtk3
ls dists/8.0/*/ | grep '^gtk3-'
```

This writes `gtk3-<ver>-r0.tar.gz` (run time) and `gtk3-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gtk3
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gtk3-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gtk3-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gtk3`](../../tests/src/) links gtk3 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gtk3`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gtk3
/usr/local/bin/qnx-port-tests/test_gtk3
```

```
== gtk3 ==
  ok   ...
PASS gtk3
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gtk3` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gtk3`](../../tests/src/) -- a program linked against gtk3 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - gdk_pixbuf_new
   - pixbuf geometry
   - pixbuf has an alpha channel
   - gdk-pixbuf encoded a PNG in memory
   - gdk-pixbuf decoded the PNG back
   - GtkWidget type registered
   - GtkButton type registered
   - GtkButton derives from GtkWidget
   - type name readback
   - GdkDisplay type registered
   - AtkObject instantiated
   - ATK name round trip
   - widgets constructed with a live display
   - label text round trip
   - container holds both children
   - gtk_widget_get_accessible
   - button reports the ATK push-button role
   - widget tree reports a natural size
   - gtk_widget_draw onto cairo
   - rendered widget tree written as PNG
   - no default display, as expected

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gtk3 3.24.43-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gtk3-3.24.43-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gtk3 3.24.43-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gtk3-3.24.43-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.24.43**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gtk3=<version>`), sources in `ported/gtk3/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gtk3`; port one with `scripts/port-update gtk3`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.24.43; GIT=https://gitlab.gnome.org/GNOME/gtk.git; REF=3.24.43
DEPS="glib cairo pango gdk-pixbuf at-spi2-core libepoxy wayland wayland-protocols libxkbcommon fribidi harfbuzz fontconfig libx11 libxext libxi libxrandr libxcursor libxfixes libxcomposite libxdamage libxinerama"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  # All three backends.  Wayland is the target desktop's native one.  X11 was
  # off at first ("QNX has no X11 server"), but the X11 client stack is staged
  # now and Thunar 4.20 hard-requires gdk/gdkx.h (XDT_CHECK_LIBX11_REQUIRE), so
  # without it the Xfce file manager cannot build at all.  Broadway (GTK's HTML5
  # backend) needs no compositor, so it is the one way to drive real widgets on
  # a target that has none; broadwayd serves them over HTTP.
  -Dwayland_backend=true -Dx11_backend=true -Dbroadway_backend=true
  -Dprint_backends=file
  -Dintrospection=false -Dgtk_doc=false -Dman=false
  -Ddemos=false -Dexamples=false -Dtests=false -Dinstalled_tests=false
  -Dcolord=no -Dcloudproviders=false -Dtracker3=false -Dxinerama=yes )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
