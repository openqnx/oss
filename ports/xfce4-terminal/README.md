# xfce4-terminal 1.1.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/apps/xfce4-terminal/1.1/xfce4-terminal-1.1.5.tar.xz |
| SHA-256 | `3c5b1d3a01a9a113852ac0f77d1c85bf3a356b43de33ec805b21ceca7d6f0a63` |
| Build system | Meson |
| QNX patches | 2 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) [`vte`](../vte/) [`pcre2`](../pcre2/) [`gtk-layer-shell`](../gtk-layer-shell/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`vte`](../vte/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install xfce4-terminal
oss-get install xfce4-terminal-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-terminal` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (69 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell
```

<details><summary>Full build chain (69)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`giflib`](../giflib/) [`glib`](../glib/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`graphene`](../graphene/) [`graphite2`](../graphite2/) [`gtk-layer-shell`](../gtk-layer-shell/) [`gtk3`](../gtk3/) [`gtk4`](../gtk4/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libtasn1`](../libtasn1/) [`libtiff`](../libtiff/) [`libunistring`](../libunistring/) [`libwebp`](../libwebp/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lz4`](../lz4/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`vte`](../vte/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/apps/xfce4-terminal/1.1/xfce4-terminal-1.1.5.tar.xz
mkdir -p dist src/xfce4-terminal
curl -fL -o dist/$(basename $URL) $URL
echo "3c5b1d3a01a9a113852ac0f77d1c85bf3a356b43de33ec805b21ceca7d6f0a63  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-terminal --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xfce4-terminal < ports/xfce4-terminal/patches/0001-aports-0001-add-meson-option-to-build-doc.patch
patch -p1 -d src/xfce4-terminal < ports/xfce4-terminal/patches/0002-aports-0002-remove-opening-file-manager-option.patch
```

**[`0001-aports-0001-add-meson-option-to-build-doc.patch`](patches/0001-aports-0001-add-meson-option-to-build-doc.patch)** — QNX change from QNX's aports (extra/xfce4-terminal/0001-add-meson-option-to-build-doc.patch).

> Taken from qnx-ports/aports extra/xfce4-terminal -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> add meson option to build doc
>
> ---
>  meson.build       | 4 +++-
>  meson_options.txt | 7 +++++++
>  2 files changed, 10 insertions(+), 1 deletion(-)

Touches: `meson.build`, `meson_options.txt` · Upstream status: QNX-specific

**[`0002-aports-0002-remove-opening-file-manager-option.patch`](patches/0002-aports-0002-remove-opening-file-manager-option.patch)** — QNX change from QNX's aports (extra/xfce4-terminal/0002-remove-opening-file-manager-option.patch).

> Taken from qnx-ports/aports extra/xfce4-terminal -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> remove opening file manager option
>
> ---
>  terminal/terminal-window.c | 6 ++++++
>  1 file changed, 6 insertions(+)

Touches: `terminal/terminal-window.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfce4-terminal src/xfce4-terminal \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dx11=disabled \
      -Dwayland=enabled \
      -Ddocs=false
ninja -C build/$QNX_ARCH/xfce4-terminal
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfce4-terminal install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-terminal: terminal emulator (VTE). Version as in the QNX Everywhere 8.0.4r1 image; it builds against the Xfce 4.20 libraries in this tree.
- Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-terminal`.

### 5. What it installs

86 files under `/usr/local`, 1 programs.

```
/usr/local/bin/xfce4-terminal
```

The complete list is `stage/<arch>/.manifest/xfce4-terminal` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH xfce4-terminal
ls repo/8.0/*/ | grep '^xfce4-terminal-'
```

This writes `xfce4-terminal-<ver>-r0.tar.gz` (run time) and `xfce4-terminal-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install xfce4-terminal
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/xfce4-terminal-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/xfce4-terminal-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-terminal: terminal emulator (VTE). Version as in the QNX Everywhere
# 8.0.4r1 image; it builds against the Xfce 4.20 libraries in this tree.
VERSION=1.1.5
TARBALL=https://archive.xfce.org/src/apps/xfce4-terminal/1.1/xfce4-terminal-1.1.5.tar.xz
SHA256=3c5b1d3a01a9a113852ac0f77d1c85bf3a356b43de33ec805b21ceca7d6f0a63
# Also linked at run time (found by scripts/gen-dependency-index): gtk-layer-shell
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf vte pcre2 gtk-layer-shell"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwayland=enabled -Ddocs=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
