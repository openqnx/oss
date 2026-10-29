# gcr 3.41.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gcr.git (tag `3.41.2`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`libgcrypt`](../libgcrypt/) [`p11-kit`](../p11-kit/) [`libsecret`](../libsecret/) [`gtk3`](../gtk3/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`libgcrypt`](../libgcrypt/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gcr
oss-get install gcr-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gcr` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (57 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libgcrypt p11-kit libsecret gtk3
```

<details><summary>Full build chain (57)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libsecret`](../libsecret/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`p11-kit`](../p11-kit/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.41.2 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gcr.git src/gcr
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gcr < ports/gcr/patches/0001-qnx-xopen-source-value.patch
```

**[`0001-qnx-xopen-source-value.patch`](patches/0001-qnx-xopen-source-value.patch)** — gcr: give _XOPEN_SOURCE a value.

> QNX's <sys/platform.h> accepts _XOPEN_SOURCE only as 500, 600 or 700; a bare
> -D_XOPEN_SOURCE stops with "This version of XOPEN_SOURCE is not supported".
> 700 is the level that declares strptime(), which the flag is there for.

Touches: `gck/meson.build`, `gcr/meson.build` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gcr src/gcr \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgtk_doc=false \
      -Dintrospection=false \
      -Dgtk=true \
      -Dssh_agent=false \
      -Dsystemd=disabled
ninja -C build/$QNX_ARCH/gcr
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gcr install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GTK enabled: gcr-3 (the UI part, prompts and certificate viewer) is what GNOME Web (epiphany) needs; gcr-base-3 alone is not enough.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gcr`.

### 5. What it installs

199 files under `/usr/local`: 9 shared-library entries, 1 programs, 60 headers.

```
/usr/local/lib/libgck-1.so
/usr/local/lib/libgck-1.so.0
/usr/local/lib/libgck-1.so.0.0.0
/usr/local/lib/libgcr-base-3.so
/usr/local/lib/libgcr-base-3.so.1
/usr/local/lib/libgcr-base-3.so.1.0.0
/usr/local/lib/libgcr-ui-3.so
/usr/local/lib/libgcr-ui-3.so.1
/usr/local/lib/libgcr-ui-3.so.1.0.0
/usr/local/bin/gcr-viewer
```

The complete list is `stage/<arch>/.manifest/gcr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gcr
ls repo/8.0/*/ | grep '^gcr-'
```

This writes `gcr-<ver>-r0.tar.gz` (run time) and `gcr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gcr
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gcr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gcr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.41.2; GIT=https://gitlab.gnome.org/GNOME/gcr.git; REF=3.41.2
DEPS="glib libgcrypt p11-kit libsecret gtk3"
BUILDSYS=meson
# GTK enabled: gcr-3 (the UI part, prompts and certificate viewer) is what
# GNOME Web (epiphany) needs; gcr-base-3 alone is not enough.
MESON_ARGS_EXTRA=( -Dgtk_doc=false -Dintrospection=false
                   -Dgtk=true -Dssh_agent=false -Dsystemd=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
