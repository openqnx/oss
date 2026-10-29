# gst-plugins-base 1.24.10

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gstreamer`](../gstreamer/) [`orc`](../orc/) [`libogg`](../libogg/) [`libvorbis`](../libvorbis/) [`opus`](../opus/) [`libtheora`](../libtheora/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`gstreamer`](../gstreamer/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libvorbis`](../libvorbis/) [`opus`](../opus/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install gst-plugins-base
oss-get install gst-plugins-base-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gst-plugins-base` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (17 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gstreamer orc libogg libvorbis opus libtheora zlib
```

<details><summary>Full build chain (17)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`gstreamer`](../gstreamer/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

The project lives in the `subprojects/gst-plugins-base/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gst-plugins-base src/gst-plugins-base/subprojects/gst-plugins-base \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Dexamples=disabled \
      -Dtools=enabled \
      -Dintrospection=disabled \
      -Ddoc=disabled \
      -Dnls=enabled \
      # \
      what \
      we \
      have \
      ported \
      -Dogg=enabled \
      -Dvorbis=enabled \
      -Dopus=enabled \
      -Dtheora=enabled \
      # \
      desktop/Linux-only \
      or \
      not \
      ported \
      yet \
      -Dalsa=disabled \
      -Dcdparanoia=disabled \
      -Dlibvisual=disabled \
      -Dpango=disabled \
      -Dtremor=disabled \
      -Dx11=disabled \
      -Dxvideo=disabled \
      -Dxshm=disabled \
      -Dgl=disabled \
      -Dgl-graphene=disabled
ninja -C build/$QNX_ARCH/gst-plugins-base
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gst-plugins-base install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gst-plugins-base`.

### 5. What it installs

320 files under `/usr/local`: 33 shared-library entries, 3 programs, 136 headers.

```
/usr/local/lib/libgstallocators-1.0.so
/usr/local/lib/libgstallocators-1.0.so.0
/usr/local/lib/libgstallocators-1.0.so.0.2410.0
/usr/local/lib/libgstapp-1.0.so
/usr/local/lib/libgstapp-1.0.so.0
/usr/local/lib/libgstapp-1.0.so.0.2410.0
/usr/local/lib/libgstaudio-1.0.so
/usr/local/lib/libgstaudio-1.0.so.0
/usr/local/lib/libgstaudio-1.0.so.0.2410.0
/usr/local/lib/libgstfft-1.0.so
/usr/local/lib/libgstfft-1.0.so.0
/usr/local/lib/libgstfft-1.0.so.0.2410.0
/usr/local/lib/libgstpbutils-1.0.so
/usr/local/lib/libgstpbutils-1.0.so.0
/usr/local/lib/libgstpbutils-1.0.so.0.2410.0
/usr/local/lib/libgstriff-1.0.so
...
```

The complete list is `stage/<arch>/.manifest/gst-plugins-base` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH gst-plugins-base
ls repo/8.0/*/ | grep '^gst-plugins-base-'
```

This writes `gst-plugins-base-<ver>-r0.tar.gz` (run time) and `gst-plugins-base-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install gst-plugins-base
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/gst-plugins-base-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/gst-plugins-base-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"          # same monorepo as the core
SUBDIR=subprojects/gst-plugins-base
DEPS="gstreamer orc libogg libvorbis opus libtheora zlib"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Dtools=enabled
  -Dintrospection=disabled -Ddoc=disabled -Dnls=enabled
  # what we have ported
  -Dogg=enabled -Dvorbis=enabled -Dopus=enabled -Dtheora=enabled
  # desktop/Linux-only or not ported yet
  -Dalsa=disabled -Dcdparanoia=disabled -Dlibvisual=disabled
  -Dpango=disabled -Dtremor=disabled
  -Dx11=disabled -Dxvideo=disabled -Dxshm=disabled
  -Dgl=disabled -Dgl-graphene=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
