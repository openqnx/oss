# cairo 1.18.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/cairo/cairo.git (tag `1.18.2`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`pixman`](../pixman/) [`freetype`](../freetype/) [`fontconfig`](../fontconfig/) [`libpng`](../libpng/) [`zlib`](../zlib/) [`glib`](../glib/) [`libx11`](../libx11/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`glib`](../glib/) [`libpng`](../libpng/) [`libx11`](../libx11/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`pixman`](../pixman/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install cairo
oss-get install cairo-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cairo` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (26 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo
```

<details><summary>Full build chain (26)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.18.2 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/cairo/cairo.git src/cairo
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cairo < ports/cairo/patches/0001-xlib-no-mitshm-without-sysv-shm.patch
```

**[`0001-xlib-no-mitshm-without-sysv-shm.patch`](patches/0001-xlib-no-mitshm-without-sysv-shm.patch)** — cairo-xlib: build without MIT-SHM where the platform has no SysV shm.

> Enabling cairo's Xlib surface on QNX 8.0 fails immediately:
>
>     src/cairo-xlib-surface-shm.c:156:10: fatal error: sys/shm.h: No such file or directory
>
> MIT-SHM is System V shared memory -- the client passes the X server the id
> that shmget() returned -- and QNX 8.0 has no SysV shared memory: libc exports
> shm_open and shm_create_handle but no shmget/shmat/shmdt/shmctl, and there is
> no <sys/shm.h>.  The extension cannot work on such a platform at all.
>
> cairo already has a complete stub implementation for servers without the
> extension.  This adds a sys/shm.h header check and takes that path when the
> header is missing, and stops cairo-xlib-surface.c including two headers it
> never uses.  On platforms that have SysV IPC nothing changes.

Touches: `meson.build`, `src/cairo-xlib-surface-shm.c`, `src/cairo-xlib-surface.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/cairo src/cairo \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Dgtk_doc=false \
      -Dfreetype=enabled \
      -Dfontconfig=enabled \
      -Dpng=enabled \
      -Dzlib=enabled \
      -Dglib=enabled \
      # \
      The \
      Xlib \
      surface: \
      originally \
      disabled \
      on \
      the \
      grounds \
      that \
      # \
      QNX \
      has \
      no \
      window \
      system. \
      The \
      X11 \
      client \
      stack \
      is \
      staged \
      # \
      now, \
      and \
      GTK3's \
      X11 \
      backend \
      -- \
      which \
      Thunar \
      4.20 \
      requires \
      -- \
      # \
      draws \
      through \
      cairo-xlib. \
      patches/0001 \
      lets \
      it \
      build \
      # \
      without \
      MIT-SHM, \
      which \
      needs \
      SysV \
      shm \
      that \
      QNX \
      lacks. \
      # \
      The \
      XCB \
      surface \
      stays \
      off: \
      nothing \
      here \
      uses \
      it, \
      and \
      its \
      # \
      SHM \
      path \
      (cairo-xcb-shm.c) \
      has \
      the \
      same \
      SysV \
      dependency. \
      -Dxlib=enabled \
      -Dxcb=disabled \
      -Dxlib-xcb=disabled \
      -Dquartz=disabled \
      -Dtee=enabled \
      -Dsymbol-lookup=disabled \
      -Dspectre=disabled
ninja -C build/$QNX_ARCH/cairo
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/cairo install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): lzo

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cairo`.

### 5. What it installs

39 files under `/usr/local`: 9 shared-library entries, 14 headers.

```
/usr/local/lib/libcairo-gobject.so
/usr/local/lib/libcairo-gobject.so.2
/usr/local/lib/libcairo-gobject.so.2.11802.2
/usr/local/lib/libcairo-script-interpreter.so
/usr/local/lib/libcairo-script-interpreter.so.2
/usr/local/lib/libcairo-script-interpreter.so.2.11802.2
/usr/local/lib/libcairo.so
/usr/local/lib/libcairo.so.2
/usr/local/lib/libcairo.so.2.11802.2
```

The complete list is `stage/<arch>/.manifest/cairo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH cairo
ls repo/8.0/*/ | grep '^cairo-'
```

This writes `cairo-<ver>-r0.tar.gz` (run time) and `cairo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install cairo
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/cairo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/cairo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.18.2; GIT=https://gitlab.freedesktop.org/cairo/cairo.git; REF=1.18.2
# Also linked at run time (found by scripts/gen-dependency-index): lzo
DEPS="pixman freetype fontconfig libpng zlib glib libx11 libxext libxrender lzo"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=disabled -Dgtk_doc=false
                   -Dfreetype=enabled -Dfontconfig=enabled -Dpng=enabled
                   -Dzlib=enabled -Dglib=enabled
                   # The Xlib surface: originally disabled on the grounds that
                   # QNX has no window system.  The X11 client stack is staged
                   # now, and GTK3's X11 backend -- which Thunar 4.20 requires --
                   # draws through cairo-xlib.  patches/0001 lets it build
                   # without MIT-SHM, which needs SysV shm that QNX lacks.
                   # The XCB surface stays off: nothing here uses it, and its
                   # SHM path (cairo-xcb-shm.c) has the same SysV dependency.
                   -Dxlib=enabled -Dxcb=disabled -Dxlib-xcb=disabled -Dquartz=disabled
                   -Dtee=enabled -Dsymbol-lookup=disabled -Dspectre=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
