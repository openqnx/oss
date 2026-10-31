# cairo 1.18.2

> 2D vector graphics library with multiple output backends

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/cairo/cairo.git (tag `1.18.2`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`pixman`](../pixman/) [`freetype`](../freetype/) [`fontconfig`](../fontconfig/) [`libpng`](../libpng/) [`zlib`](../zlib/) [`glib`](../glib/) [`libx11`](../libx11/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`glib`](../glib/) [`libpng`](../libpng/) [`libx11`](../libx11/) [`libxext`](../libxext/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`pixman`](../pixman/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cairo`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `cairo` (programs, shared libraries, run-time data) and `cairo-dev` (headers, pkg-config/CMake files, static libraries; needs `cairo`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`cairo`](https://openqnx.github.io/oss/package/x86_64/cairo.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cairo.html)) · [`cairo-dev`](https://openqnx.github.io/oss/package/x86_64/cairo-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cairo-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add cairo` (dependencies come along); one version: `apk add cairo=<version>` | `apk del cairo` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install cairo` (dependencies first) | `oss-get remove cairo` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`cairo-dev` the same way, by its own name (`apk add cairo-dev`, `oss-get install cairo-dev`, by hand with its own file and list `cairo-dev.list`); remove it before `cairo`, which it needs.

```sh
# on the QNX target, as root
apk add cairo cairo-dev
apk add qnx-test-cairo && /usr/local/bin/qnx-port-tests/test_cairo    # its on-target test
apk del cairo cairo-dev qnx-test-cairo
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/cairo.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/cairo.list
rm var/lib/manual/cairo.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cairo
ls dists/8.0/*/ | grep '^cairo-'
```

This writes `cairo-<ver>-r0.tar.gz` (run time) and `cairo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cairo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cairo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cairo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cairo`](../../tests/src/) links cairo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cairo`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cairo
/usr/local/bin/qnx-port-tests/test_cairo
```

```
== cairo ==
  ok   ...
PASS cairo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cairo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cairo`](../../tests/src/) -- a program linked against cairo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - image surface + context
   - filled rectangle: red inside, white outside
   - 4 px stroke covers y 38..41 only
   - 50% alpha blending (OVER)
   - clip limits paint
   - linear gradient ramps black to white
   - matrix translate+scale
   - write PNG to /tmp
   - PNG read back with the same pixels
   - PDF stream starts with %PDF-
   - PDF stream has the %%EOF trailer
   - SVG stream contains <svg
   - recording surface ink extents

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cairo 1.18.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/cairo-1.18.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cairo 1.18.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/cairo-1.18.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cairo 1.18.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/cairo-1.18.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cairo 1.18.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/cairo-1.18.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.18.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cairo=<version>`), sources in `ported/cairo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cairo`; port one with `scripts/port-update cairo`.

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
