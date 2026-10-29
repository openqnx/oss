# pqiv 2.13.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.2.tar.gz |
| SHA-256 | `154cbbe0a62be12cee23b0a46a86b2305d8128fd19924308ad5e9d22fa5ad4f7` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`gdk-pixbuf`](../gdk-pixbuf/) [`cairo`](../cairo/) |
| Run-time dependencies | [`at-spi2-core`](../at-spi2-core/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`libx11`](../libx11/) [`pango`](../pango/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install pqiv
oss-get install pqiv-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pqiv` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (52 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 gdk-pixbuf cairo
```

<details><summary>Full build chain (52)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.2.tar.gz
mkdir -p dist src/pqiv
curl -fL -o dist/$(basename $URL) $URL
echo "154cbbe0a62be12cee23b0a46a86b2305d8128fd19924308ad5e9d22fa5ad4f7  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pqiv --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/pqiv < ports/pqiv/patches/0001-aports-remove-avclose.patch
```

**[`0001-aports-remove-avclose.patch`](patches/0001-aports-remove-avclose.patch)** — QNX change from QNX's aports (extra/pqiv/remove-avclose.patch).

> Taken from qnx-ports/aports extra/pqiv -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `backends/libav.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/pqiv; B=build/$QNX_ARCH/pqiv; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" PKG_CONFIG=pkg-config \
  ./configure --prefix="$PREFIX" --backends-build=static --backends=gdkpixbuf \
     ; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pqiv, a minimalist image viewer (GTK3). Version as in the QNX Everywhere 8.0.4r1 image.
- pqiv's configure is hand-written; it takes the compiler and the backends.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pqiv`.

### 5. What it installs

3 files under `/usr/local`, 1 programs.

```
/usr/local/bin/pqiv
```

The complete list is `stage/<arch>/.manifest/pqiv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH pqiv
ls repo/8.0/*/ | grep '^pqiv-'
```

This writes `pqiv-<ver>-r0.tar.gz` (run time) and `pqiv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install pqiv
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/pqiv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/pqiv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# pqiv, a minimalist image viewer (GTK3).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.2
TARBALL=https://github.com/phillipberndt/pqiv/archive/refs/tags/2.13.2.tar.gz
SHA256=154cbbe0a62be12cee23b0a46a86b2305d8128fd19924308ad5e9d22fa5ad4f7
DEPS="glib gtk3 gdk-pixbuf cairo"
BUILDSYS=custom
# pqiv's configure is hand-written; it takes the compiler and the backends.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" PKG_CONFIG=pkg-config \
  ./configure --prefix="$PREFIX" --backends-build=static --backends=gdkpixbuf \
     ; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
